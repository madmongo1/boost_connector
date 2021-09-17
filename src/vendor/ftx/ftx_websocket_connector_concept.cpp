#include <boost/algorithm/hex.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/connector/util/check_executor.hpp>
#include <boost/connector/util/url.hpp>
#include <boost/connector/util/websocket_stream_variant.hpp>
#include <boost/connector/vendor/ftx/detail/ping_pong.hpp>
#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector_concept.hpp>
#include <boost/functional/hash.hpp>
#include <boost/json.hpp>

#include <iostream>

namespace boost::connector::vendor::ftx
{
// interface

ftx_websocket_connector_concept::ftx_websocket_connector_concept(
    boost::asio::any_io_executor exec,
    boost::asio::ssl::context &  sslctx,
    ftx_websocket_key const &    key)
: exec_(std::move(exec))
, sslctx_(sslctx)
, args_(key)
, write_queue_(exec_)
{
}

void
ftx_websocket_connector_concept::start()
{
    asio::co_spawn(
        get_executor(),
        run(),
        asio::bind_cancellation_slot(
            stop_signal_.slot(),
            [self = shared_from_this()](std::exception_ptr ep)
            {
                try
                {
                    if (ep)
                        std::rethrow_exception(ep);
                    std::cout << "ftx_websocket_connector::start : success\n";
                }
                catch (const std::exception &e)
                {
                    std::cout << "ftx_websocket_connector::start : " << e.what()
                              << '\n';
                }

                /// @todo mark status as fatal, emit signal and stop
            }));
}

void
ftx_websocket_connector_concept::stop()
{
    asio::dispatch(get_executor(),
                   [self = shared_from_this()]
                   {
                       std::cout << "emitting stop signal\n";
                       self->stop_signal_.emit(asio::cancellation_type::all);
                   });
}

void
ftx_websocket_connector_concept::send(std::string payload)
{
    assert(util::check_executor(get_executor()));
    write_queue_.push(std::move(payload));
}

// private
namespace
{
json::string
ftx_signature(std::string_view secret,
              unsigned long    time,
              std::string_view path)
{
    std::string plain = std::to_string(time);
    plain.append(path.begin(), path.end());

    unsigned char bytes[EVP_MAX_MD_SIZE];
    unsigned int  result_len = EVP_MAX_MD_SIZE;
    HMAC(EVP_sha256(),
         reinterpret_cast< const unsigned char * >(secret.data()),
         secret.size(),
         reinterpret_cast< const unsigned char * >(plain.data()),
         plain.size(),
         bytes,
         &result_len);

    json::string result;
    result.resize(result_len * 2);
    boost::algorithm::hex_lower(bytes, bytes + result_len, result.begin());
    return result;
}

std::string
ftx_auth_frame(ftx_credentials const &auth)
{
    auto now_ms = std::chrono::duration_cast< std::chrono::milliseconds >(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
    auto frame = json::object();
    frame.reserve(4);
    frame.emplace("key", auth.api_key);
    frame.emplace("time", now_ms);
    frame.emplace("sign",
                  ftx_signature(auth.api_secret, now_ms, "websocket_login"));
    if (auth.has_subaccount())
        frame.emplace("subaccount", auth.subaccount);
    return json::serialize(frame);
}

asio::awaitable< void >
ftx_write_state(websocket_stream_variant &  ws,
                async_queue< std::string > &write_queue)
{
    for (;;)
    {
        auto frame = co_await write_queue.pop();
        std::cout << "sending: " << frame << '\n';
        co_await ws.write(asio::buffer(frame));
    }
}

std::string
copy(string_view sv)
{
    return std::string(sv.begin(), sv.end());
}

}   // namespace

asio::awaitable< void >
ftx_websocket_connector_concept::reconnect()
try
{
    using namespace asio::experimental::awaitable_operators;
    std::cout << "ftx_websocket_connector::reconnect()\n";

    auto url_parts = decode_url(args_.url);

    std::cout << "ftx_websocket_connector::reconnect() - create websocket\n";
    auto ws =
        websocket_stream_variant(get_executor(), sslctx_, url_parts.transport);
    on_drop_ = [&ws] { ws.tcp_layer().close(); };

    std::cout << "ftx_websocket_connector::reconnect() - connect\n";
    co_await ws.connect(
        url_parts.hostname, url_parts.service, url_parts.path_etc);

    async_circular_buffer< json::value, 1 > pong_buffer(
        co_await asio::this_coro::executor);

    write_queue_.clear();
    signals_.clear();

    if (args_.auth.has_auth())
        write_queue_.push(ftx_auth_frame(args_.auth));

    std::cout
        << "ftx_websocket_connector::reconnect() - notify connection state\n";
    notify_connection_state(connection_state::up);

    std::cout << "ftx_websocket_connector::reconnect() - fork read, write, "
                 "pingpong\n";
    co_await(read_state(ws, pong_buffer) &&
             ftx_write_state(ws, this->write_queue_) &&
             detail::run_ping_pong(write_queue_, pong_buffer));

    std::cout << "ftx_websocket_connector::reconnect() - notify down\n";
    notify_connection_state(connection_state::down);
    on_drop_ = nullptr;
    std::cout << "ftx_websocket_connector::reconnect() - exit success\n";
}
catch (std::exception &e)
{
    on_drop_ = nullptr;
    std::cout << "ftx_websocket_connector::reconnect() - exit exception: "
              << e.what() << "\n";
}

asio::awaitable< void >
ftx_websocket_connector_concept::run()
try
{
    using namespace std::literals;
    using namespace asio::experimental::awaitable_operators;
    std::cout << "ftx_websocket_connector::run()\n";
    for (;;)
    {
        co_await reconnect();
        auto timer = asio::steady_timer(co_await asio::this_coro::executor);
        timer.expires_after(5s);
        co_await timer.async_wait(asio::use_awaitable);
    }
    std::cout << "ftx_websocket_connector::exit success()\n";
}
catch (std::exception &e)
{
    std::cout << "ftx_websocket_connector::run() exit exception: " << e.what()
              << "\n";
    throw;
}

asio::awaitable< void >
ftx_websocket_connector_concept::read_state(
    websocket_stream_variant &               ws,
    async_circular_buffer< json::value, 1 > &pong_buffer)
try
{
    std::cout << __func__ << "() - enter\n";

    auto read_cancel_handler = [this, &ws](asio::cancellation_type type)
    {
        if ((type & asio::cancellation_type::terminal) ==
            asio::cancellation_type::none)
            return;

        std::cout << "read_cancel_handler\n";
        asio::co_spawn(ws.get_executor(),
                       ws.close(),
                       [self = this->shared_from_this()](std::exception_ptr)
                       { std::cout << "close complete\n"; });
    };

    for (;;)
    {
        auto state = co_await asio::this_coro::cancellation_state;
        state.slot().assign(read_cancel_handler);
        auto msg = co_await ws.read();
        if (msg.is_binary())
            throw std::runtime_error(
                "ftx_websocket_connector::read_state - binary frame received");
        auto  jframe = json::parse(msg.as_string());
        auto &o      = jframe.as_object();
        auto &type   = o.at("type").as_string();
        if (type == "pong")
            pong_buffer.push(std::move(jframe));
        else
        {
            auto const index = channel_market_pair {
                .channel = json::value_to< std::string >(o.at("channel")),
                .market  = json::value_to< std::string >(o.at("market"))
            };
            auto i = signals_.find(index);
            if (i == signals_.end())
                std::cout << "ftx_websocket_connector_concept::read_state - "
                             "unexpected response: "
                          << msg << '\n';
            else if (i->second)
            {
                i->second(std::move(jframe));
            }
        }
    }
    std::cout << __func__ << "() - exit\n";
}
catch (std::exception &e)
{
    std::cout << __func__ << "() - exception: " << e.what() << "\n";
    throw;
}

void
ftx_websocket_connector_concept::on(channel_market_pair const &channel,
                                    channel_slot               cb)
{
    util::check_executor(get_executor());

    if (auto i = signals_.find(channel); cb)
    {
        if (i == signals_.end())
            i = signals_.emplace(channel, std::move(cb)).first;
        else
            i->second = std::move(cb);
    }
    else if (i != signals_.end())
        signals_.erase(i);
}

void
ftx_websocket_connector_concept::notify_connection_state(connection_state s)
{
    if (s != connection_state_)
    {
        connection_state_ = s;
        connection_state_cv_.cancel();
    }
}

asio::awaitable< void >
ftx_websocket_connector_concept::wait_ready()
{
    while (connection_state_ != connection_state::up)
        co_await(connection_state_cv_.async_wait(
            asio::experimental::as_tuple(asio::use_awaitable)));
}

asio::awaitable< void >
ftx_websocket_connector_concept::wait_down()
{
    while (connection_state_ == connection_state::up)
        co_await(connection_state_cv_.async_wait(
            asio::experimental::as_tuple(asio::use_awaitable)));
}

auto
ftx_websocket_connector_concept::acquire_channel(
    channel_market_pair const &index) -> asio::awaitable< channel_token >
try
{
    assert(util::check_executor(get_executor()));

    std::cout << "ftx_websocket_connector_concept::acquire_channel(" << index
              << ") - entry\n";

    // find or create the channel state and express interest

    auto iter = channel_states_.find(index);
    if (iter == channel_states_.end())
        iter = channel_states_.emplace(index, get_executor()).first;
    else
        ++iter->second.interest;

    auto &state = iter->second;

    while (state.acquired)
    {
        co_await state.cv.async_wait(
            asio::experimental::as_tuple(asio::use_awaitable));
    }

    state.acquired = true;

    std::cout << "ftx_websocket_connector_concept::acquire_channel(" << index
              << ") - exit ok\n";

    co_return channel_token { this, iter };
}
catch (std::exception &e)
{
    std::cout << "ftx_websocket_connector_concept::acquire_channel(" << index
              << ") - exception: " << e.what() << "\n";
    assert(false);
    throw;
}

void
ftx_websocket_connector_concept::release_channel(channel_iterator iter)
{
    std::cout << "ftx_websocket_connector_concept::release_channel("
              << iter->first << ") - entry\n";
    assert(util::check_executor(get_executor()));
    auto &state = iter->second;
    assert(state.acquired);
    state.acquired = false;
    state.cv.cancel_one();   // release one waiter
    if (--state.interest == 0)
    {
        std::cout << "ftx_websocket_connector_concept::release_channel("
                  << iter->first << ") - erase state\n";
        channel_states_.erase(iter);
    }
}

}   // namespace boost::connector::vendor::ftx