#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector.hpp>

namespace boost::connector
{
ftx_websocket_connector::ftx_websocket_connector(boost::asio::any_io_executor exec,
                                                 boost::asio::ssl::context &  sslctx,
                                                 ftx_websocket_key const &    key)
: exec_(std::move(exec))
, sslctx_(sslctx)
, args_(key)
{
}

void
ftx_websocket_connector::start()
{
}

void
ftx_websocket_connector::stop()
{
}

}   // namespace boost::connector