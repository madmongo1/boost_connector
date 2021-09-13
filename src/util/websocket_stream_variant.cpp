#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/connector/config/error.hpp>
#include <boost/connector/util/websocket_stream_variant.hpp>
#include <boost/variant2/variant.hpp>

namespace boost::connector
{
websocket_stream_variant::websocket_stream_variant(asio::any_io_executor const &exec,
                                                   asio::ssl::context          &sslctx,
                                                   transport_type               type)
: vws_(construct(exec, sslctx, type))
{
}

variant2::variant< ws_transport_layer, wss_transport_layer >
websocket_stream_variant::construct(asio::any_io_executor const &exec, asio::ssl::context &sslctx, transport_type type)
{
    if (type == transport_type::tcp)
        return ws_transport_layer(exec);
    else
        return wss_transport_layer(exec, sslctx);
}

namespace
{
asio::awaitable< asio::ip::tcp::resolver::results_type >
resolve(std::string const &host, std::string const &service)
{
    auto resolver = asio::ip::tcp::resolver(co_await asio::this_coro::executor);
    co_return co_await resolver.async_resolve(host, service, asio::use_awaitable);
}

tcp_transport_layer
selected(std::variant< tcp_transport_layer, tcp_transport_layer > v)
{
    switch (v.index())
    {
    case 0:
        return std::move(std::get< 0 >(v));
    case 1:
        return std::move(std::get< 1 >(v));
    default:
        throw std::logic_error(__func__);
    }
}

asio::awaitable< tcp_transport_layer >
connect(asio::ip::tcp::endpoint ep)
{
    auto sock = tcp_transport_layer(co_await asio::this_coro::executor);
    co_await sock.async_connect(ep, asio::use_awaitable);
    co_return std::move(sock);
}

asio::awaitable< tcp_transport_layer >
connect_range(asio::ip::tcp::resolver::results_type::const_iterator first,
              asio::ip::tcp::resolver::results_type::const_iterator last)
{
    using namespace asio::experimental::awaitable_operators;

    assert(first != last);

    auto next = std::next(first);
    if (next == last)
        co_return co_await connect(first->endpoint());
    else
        co_return selected(co_await(connect(first->endpoint()) || connect_range(next, last)));
}

asio::awaitable< tcp_transport_layer >
connect_one(asio::ip::tcp::resolver::results_type results)
{
    return connect_range(results.begin(), results.end());
}
}   // namespace

asio::awaitable< void >
websocket_stream_variant::connect(std::string const &host, std::string const &service, std::string const &target)
{
    replace_tcp(co_await connect_one(co_await resolve(host, service)));
    if (is_tls())
        co_await tls_handshake(host);

    co_await visit([&host, &target](auto &ws) { return ws.async_handshake(host, target, asio::use_awaitable); }, vws_);
}

bool
websocket_stream_variant::is_tls() const
{
    // clang-format off
    return visit([]< class Ws >(Ws const &) 
    { 
        return std::is_same_v< std::remove_cvref_t< Ws >, 
            wss_transport_layer >; 
    }, vws_);
    // clang-format on
}

asio::awaitable< std::size_t >
websocket_stream_variant::write(asio::const_buffer buf)
{
    // clang-format off
    return visit([buf](auto &ws)
    {
        return ws.async_write(buf, asio::use_awaitable);
    }, vws_);
    // clang-format on
}

asio::awaitable< websocket_message >
websocket_stream_variant::read()
{
    auto msg = websocket_message();
    co_await visit([&msg](auto &ws) { return ws.async_read(msg.as_buffer(), asio::use_awaitable); }, vws_);
    visit([&msg](auto &ws) { msg.set_binary(ws.got_binary()); }, vws_);
    co_return msg;
}

asio::awaitable< void >
websocket_stream_variant::tls_handshake(std::string const &hostname)
{
    assert(is_tls());
    auto &tls = get< wss_transport_layer >(vws_).next_layer();
    if (!SSL_set_tlsext_host_name(tls.native_handle(), hostname.c_str()))
        throw system_error(error_code { static_cast< int >(::ERR_get_error()), asio::error::get_ssl_category() });
    return tls.async_handshake(asio::ssl::stream_base::client, asio::use_awaitable);
}

void
websocket_stream_variant::replace_tcp(tcp_transport_layer &&tcp)
{
    auto &layer = visit([](auto &ws) -> tcp_transport_layer & { return beast::get_lowest_layer(ws); }, vws_);
    layer       = std::move(tcp);
}

}   // namespace boost::connector