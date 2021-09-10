#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/connector/util/url.hpp>
#include <boost/connector/util/websocket_stream_variant.hpp>
#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector.hpp>

namespace boost::connector
{
// interface

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
    asio::co_spawn(get_executor(),
                   run(),
                   [self = shared_from_this()](std::exception_ptr)
                   {
                       // @todo mark status as fatal, emit signal and stop
                   });
}

void
ftx_websocket_connector::stop()
{
    asio::dispatch(get_executor(), [self = shared_from_this()] { self->on_stop(); });
}

asio::any_io_executor const &
ftx_websocket_connector::get_executor() const
{
    return exec_;
}

// private

asio::awaitable< void >
ftx_websocket_connector::run()
{
    auto url_parts = decode_url(args_.url);

    auto ws = websocket_stream_variant(get_executor(), sslctx_, url_parts.transport);

    co_return;
}

void
ftx_websocket_connector::on_stop()
{
}

}   // namespace boost::connector