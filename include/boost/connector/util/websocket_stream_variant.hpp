#ifndef BOOST_CONNECTOR__UTIL__WEBSOCKET_STREAM_VARIANT__HPP
#define BOOST_CONNECTOR__UTIL__WEBSOCKET_STREAM_VARIANT__HPP

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/connector/util/transport_type.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/variant2/variant.hpp>

namespace boost::connector
{
using tcp_transport_layer = boost::asio::ip::tcp::socket;
using tls_transport_layer = boost::asio::ssl::stream< tcp_transport_layer >;
using ws_transport_layer  = boost::beast::websocket::stream< tcp_transport_layer >;
using wss_transport_layer = boost::beast::websocket::stream< tls_transport_layer >;

struct websocket_message
{
    beast::flat_buffer &
    as_buffer()
    {
        return buf_;
    }

    string_view
    as_string() const
    {
        auto d = buf_.data();
        return { reinterpret_cast< char const * >(d.data()), d.size() };
    }

    bool
    is_binary() const
    {
        return is_binary_;
    }

    void
    set_binary(bool b)
    {
        is_binary_ = b;
    }

  private:
    beast::flat_buffer buf_;
    bool               is_binary_ = false;
};

struct websocket_stream_variant
{
    using executor_type = asio::any_io_executor;

    websocket_stream_variant(executor_type const &exec, asio::ssl::context &sslctx, transport_type type);

    asio::awaitable< void >
    connect(std::string const &host, std::string const &service, std::string const &target);

    executor_type
    get_executor();

    asio::awaitable< void >
    close(beast::websocket::close_reason reason = beast::websocket::close_code::normal);

    /// Test wether the underlying transport is tls
    /// @return bool which will be true if the next layer is TLS
    bool
    is_tls() const;

    asio::awaitable< std::size_t >
    write(asio::const_buffer buf);

    asio::awaitable< websocket_message >
    read();

  private:
    static variant2::variant< ws_transport_layer, wss_transport_layer >
    construct(asio::any_io_executor const &exec, asio::ssl::context &sslctx, transport_type type);

    /// Replace the tcp transport layer with the given socket.
    ///
    /// @pre The existing socket is not already connected.
    void
    replace_tcp(tcp_transport_layer &&tcp);

    /// Perform the tls handshake
    ///
    /// @pre is_tls() == true
    asio::awaitable< void >
    tls_handshake(std::string const &hostname);

  private:
    variant2::variant< ws_transport_layer, wss_transport_layer > vws_;
};

}   // namespace boost::connector

#endif
