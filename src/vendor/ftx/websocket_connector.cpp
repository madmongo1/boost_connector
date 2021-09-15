#include <boost/connector/vendor/ftx/websocket_connector.hpp>

namespace boost::connector::vendor::ftx
{
websocket_connector::websocket_connector(lifetime_ptr< ftx_websocket_connector > lifetime)
: lifetime_(std::move(lifetime))
{
}

}   // namespace boost::connector::vendor::ftx