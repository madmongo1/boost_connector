#include <boost/algorithm/hex.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/connector/util/url.hpp>
#include <boost/connector/util/websocket_stream_variant.hpp>
#include <boost/connector/vendor/ftx/detail/ping_pong.hpp>
#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector.hpp>
#include <boost/json.hpp>
#include <boost/scope_exit.hpp>

#include <iostream>

namespace boost::connector::vendor::ftx
{
// interface

websocket_connector::websocket_connector(boost::asio::any_io_executor exec,
                                         boost::asio::ssl::context &  sslctx,
                                         ftx_websocket_key const &    key)
: exec_(std::move(exec))
, sslctx_(sslctx)
, args_(key)
, write_queue_(exec_)
{
}

void
websocket_connector::start()
{
    asio::co_spawn(get_executor(),
                   run(),
                   asio::bind_cancellation_slot(stop_signal_.slot(),
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
websocket_connector::stop()
{
    asio::dispatch(get_executor(),
                   [self = shared_from_this()]
                   {
                       std::cout << "emitting stop signal\n";
                       self->stop_signal_.emit(asio::cancellation_type::all);
                   });
}

asio::any_io_executor const &
websocket_connector::get_executor() const
{
    return exec_;
}

// private
namespace
{
json::string
ftx_signature(std::string_view secret, unsigned long time, std::string_view path)
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
    auto now_ms =
        std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch())
            .count();
    auto frame = json::object();
    frame.reserve(4);
    frame.emplace("key", auth.api_key);
    frame.emplace("time", now_ms);
    frame.emplace("sign", ftx_signature(auth.api_secret, now_ms, "websocket_login"));
    if (auth.has_subaccount())
        frame.emplace("subaccount", auth.subaccount);
    return json::serialize(frame);
}

asio::awaitable< void >
ftx_write_state(websocket_stream_variant &ws, async_queue< std::string > &write_queue)
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
websocket_connector::run()
{
    using namespace asio::experimental::awaitable_operators;
    std::cout << "ftx_websocket_connector::run()\n";
    auto url_parts = decode_url(args_.url);

    auto ws = websocket_stream_variant(get_executor(), sslctx_, url_parts.transport);
    co_await ws.connect(url_parts.hostname, url_parts.service, url_parts.path_etc);

    write_queue_.clear();
    async_circular_buffer< json::value, 1 > pong_buffer(co_await asio::this_coro::executor);

    if (args_.auth.has_auth())
        write_queue_.push(ftx_auth_frame(args_.auth));

    co_await(read_state(ws, pong_buffer) && ftx_write_state(ws, this->write_queue_) &&
             detail::run_ping_pong(write_queue_, pong_buffer));

    co_return;
}

asio::awaitable< void >
websocket_connector::read_state(websocket_stream_variant &ws, async_circular_buffer< json::value, 1 > &pong_buffer)
{
    BOOST_SCOPE_EXIT_ALL()
    {
        auto ep = std::current_exception();
        try
        {
            if (ep)
                std::throw_with_nested(ep);
            std::cout << "ftx_websocket_connector::read_state() - exit ok\n";
        }
        catch (std::exception &e)
        {
            std::cout << "ftx_websocket_connector::read_state() - exception: " << e.what() << '\'';
        }
    };

    auto read_cancel_handler = [this, &ws](asio::cancellation_type type)
    {
        if ((type & asio::cancellation_type::terminal) == asio::cancellation_type::none)
            return;

        std::cout << "read_cancel_handler\n";
        asio::co_spawn(ws.get_executor(),
                       ws.close(),
                       [self = this->shared_from_this()](std::exception_ptr) { std::cout << "close complete\n"; });
    };

    for (;;)
    {
        auto state = co_await asio::this_coro::cancellation_state;
        state.slot().assign(read_cancel_handler);
        auto msg = co_await ws.read();
        if (msg.is_binary())
            throw std::runtime_error("ftx_websocket_connector::read_state - binary frame received");
        std::cout << "frame received: " << msg.as_string() << '\n';
        auto  jframe = json::parse(msg.as_string());
        auto &o      = jframe.as_object();
        auto &type   = o.at("type").as_string();
        if (type == "pong")
            pong_buffer.push(std::move(jframe));
        else if (type == "subscribed")
            /*notify_subscribed()*/;
        else if (type == "unsubscribed")
            /*notify_unsubscribed()*/;
        else
            throw std::runtime_error("ftx_websocket_connector::read_state : unrecognised frame " +
                                     copy(msg.as_string()));
    }
}

}   // namespace boost::connector::vendor::ftx