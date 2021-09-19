#include <boost/connector/vendor/ftx/websocket_connector.hpp>

#include <cassert>

namespace boost::connector::vendor::ftx
{
websocket_connector::websocket_connector(
    lifetime_ptr< ftx_websocket_connector_concept > lifetime)
: lifetime_(std::move(lifetime))
{
}

void
websocket_connector::drop()
{
    assert(lifetime_);
    lifetime_->drop();
}

void
websocket_connector::send(std::string payload)
{
    assert(lifetime_);
    lifetime_->send(std::move(payload));
}

auto
websocket_connector::get_executor() const -> executor_type const &
{
    assert(lifetime_);
    return lifetime_->get_executor();
}

asio::awaitable< void >
websocket_connector::wait_ready()
{
    assert(lifetime_);
    return lifetime_->wait_ready();
}

asio::awaitable< void >
websocket_connector::wait_down()
{
    assert(lifetime_);
    return lifetime_->wait_down();
}

ftx_websocket_connector_concept &
websocket_connector::get_impl()
{
    assert(lifetime_);
    return *lifetime_;
}

}   // namespace boost::connector::vendor::ftx