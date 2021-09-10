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

}   // namespace boost::connector