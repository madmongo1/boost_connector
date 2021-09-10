#include <boost/asio/use_awaitable.hpp>
#include <boost/connector/util/websocket_stream_variant.hpp>

namespace boost::connector
{
websocket_stream_variant::websocket_stream_variant(asio::any_io_executor const &exec,
                                                   asio::ssl::context &         sslctx,
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
}   // namespace

asio::awaitable< void >
websocket_stream_variant::connect(std::string const &host, std::string const &service, std::string const &target)
{
    auto endpoints = co_await resolve(host, service);

    /// @todo connect to endpoints with timeout, cancel rest on first to connect
}

}   // namespace boost::connector