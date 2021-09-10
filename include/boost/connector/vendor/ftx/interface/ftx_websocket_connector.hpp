#ifndef BOOST_CONNETOR__VENDOR__FTX__FTX_WEBSOCKET_CONNECTOR__HPP
#define BOOST_CONNETOR__VENDOR__FTX__FTX_WEBSOCKET_CONNECTOR__HPP

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/connector/entity/entity_impl_concept.hpp>
#include <boost/connector/vendor/ftx/ftx_config.hpp>

#include <memory>

namespace boost::connector
{
/// Implementation of an FTX websocket connector.
///
/// Once started, the connector will continuously try to reconnect to the url indicated in the supplied key.
/// Once connected, it will optionally authenticate (if the authentication fields in the key are not empty).
/// If the connection is dropped, the object will seek to re-establish a connection after a small delay.
/// If the stop() member is called, the object will abandon its current connection as cleanly as possible
/// @note this object manages its own memory via enable_shared_from_this. It will allow itself to be destoyed once
/// all internal outstanding asynchrnous operations are completed.
///
struct ftx_websocket_connector
: entity_impl_concept
, std::enable_shared_from_this< ftx_websocket_connector >
{
    /// Construct an FTX websocket connector.
    ///
    /// @param exec is the IO executor on which the objects internal operations will make progress.
    /// It is also the executor on which subscription slots will be executed.
    /// @param sslctx is a reference to the ssl context to be used for any ssl connections. The context's lifetime must
    /// not end before the end of the lifetime of this object
    /// @param key is the set of arguments that uniquely ifdentify this connectior and parameterise the connection.
    ftx_websocket_connector(boost::asio::any_io_executor exec,
                            boost::asio::ssl::context &  sslctx,
                            ftx_websocket_key const &    key);

    virtual void
    start() override final;

    virtual void
    stop() override final;

  private:
    asio::any_io_executor   exec_;
    asio::ssl::context &    sslctx_;
    ftx_websocket_key const args_;
};

}   // namespace boost::connector

#endif