#ifndef BOOST_CONNECTOR__UTIL__WEBSOCKET_STREAM_VARIANT__HPP
#define BOOST_CONNECTOR__UTIL__WEBSOCKET_STREAM_VARIANT__HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/connector/util/transport_type.hpp>
#include <boost/variant2/variant.hpp>

namespace boost::connector
{
using tcp_transport_layer = boost::asio::ip::tcp::socket;
using tls_transport_layer = boost::asio::ssl::stream< tcp_transport_layer >;
using ws_transport_layer  = boost::beast::websocket::stream< tcp_transport_layer >;
using wss_transport_layer = boost::beast::websocket::stream< tls_transport_layer >;

struct websocket_stream_variant
{
    websocket_stream_variant(asio::any_io_executor const &exec, asio::ssl::context &sslctx, transport_type type);

  private:
    static variant2::variant< ws_transport_layer, wss_transport_layer >
    construct(asio::any_io_executor const &exec, asio::ssl::context &sslctx, transport_type type);

  private:
    variant2::variant< ws_transport_layer, wss_transport_layer > vws_;
};

}   // namespace boost::connector

#endif
