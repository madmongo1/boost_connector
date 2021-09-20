#ifndef BOOST_CONNETOR__VENDOR__FTX__FTX_WEBSOCKET_CONNECTOR__HPP
#define BOOST_CONNETOR__VENDOR__FTX__FTX_WEBSOCKET_CONNECTOR__HPP

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/connector/entity/entity_impl_concept.hpp>
#include <boost/connector/util/async_circular_buffer.hpp>
#include <boost/connector/util/async_latch.hpp>
#include <boost/connector/util/async_queue.hpp>
#include <boost/connector/util/equals.hpp>
#include <boost/connector/util/hash_any.hpp>
#include <boost/connector/util/hash_value.hpp>
#include <boost/connector/vendor/ftx/channel_market_pair.hpp>
#include <boost/connector/vendor/ftx/channel_state.hpp>
#include <boost/connector/vendor/ftx/ftx_config.hpp>
#include <boost/json/value.hpp>
#include <boost/variant2/variant.hpp>

#include <deque>
#include <memory>
#include <queue>
#include <unordered_map>

namespace boost::connector
{
// forward declaration
struct websocket_stream_variant;

namespace vendor::ftx
{
BOOST_DEFINE_ENUM_CLASS(connection_state, down, up)

/// Implementation of an FTX websocket connector.
///
/// Once started, the connector will continuously try to reconnect to the url
/// indicated in the supplied key. Once connected, it will optionally
/// authenticate (if the authentication fields in the key are not empty). If the
/// connection is dropped, the object will seek to re-establish a connection
/// after a small delay. If the stop() member is called, the object will abandon
/// its current connection as cleanly as possible
/// @note this object manages its own memory via enable_shared_from_this. It
/// will allow itself to be destoyed once all internal outstanding asynchrnous
/// operations are completed.
///
struct ftx_websocket_connector_concept final
: entity_impl_concept
, std::enable_shared_from_this< ftx_websocket_connector_concept >
{
    /// @brief The executor type used for internal operations.
    using executor_type = asio::any_io_executor;

    /// Construct an FTX websocket connector.
    ///
    /// @param exec is the IO executor on which the objects internal operations
    /// will make progress. It is also the executor on which subscription slots
    /// will be executed.
    /// @param sslctx is a reference to the ssl context to be used for any ssl
    /// connections. The context's lifetime must not end before the end of the
    /// lifetime of this object
    /// @param key is the set of arguments that uniquely ifdentify this
    /// connectior and parameterise the connection.
    ftx_websocket_connector_concept(boost::asio::any_io_executor exec,
                                    boost::asio::ssl::context   &sslctx,
                                    ftx_websocket_key const     &key);

    void
    start();

    void
    stop();

    /// return a reference to the internal executor.
    ///
    /// Dependent entities may use this executor to co-ordinate work with this
    /// connector.
    /// @note Safe to call from any thread or executor
    inline executor_type const &
    get_executor() const;

    using channel_slot = std::function< void(json::value) >;

    /// @brief Queue a message for send.
    /// @param payload  The message to send
    /// @pre must be called while being invoked by the executor retured by
    /// get_executor()
    /// @note If the connection drops before the message can be sent, the
    /// message will be silently dropped.
    void
    send(std::string payload);

    /// @brief Wait for the connection to be fully established.
    /// @details If the connection is already fully established, return
    /// immediately.
    /// @pre must be called from the connection's executor. No marshalling takes
    /// place.
    /// @return
    asio::awaitable< void >
    wait_ready();

    /// @brief Instantaneous check whether the connection is up.
    /// @pre must be called on this objects' executor
    /// @return
    bool
    is_up() const;

    /// @brief Wait for the connection to be no longer established.
    /// @details If the connection is already not fully established, return
    /// immediately.
    /// @pre must be called from the connection's executor. No marshalling takes
    /// place.
    /// @return
    asio::awaitable< void >
    wait_down();

    void
    drop()
    {
        if (on_drop_)
        {
            auto d   = std::move(on_drop_);
            on_drop_ = nullptr;
            d();
        }
    }

    channel_state_map::iterator
    locate_channel(channel_market_pair const &index);

    void
    release_channel(channel_state_map::iterator iter);

  private:
    static std::string
    make_subscribe_message(channel_market_pair const &index,
                           boost::string_view         op);

  private:
    asio::awaitable< void >
    run();

    asio::awaitable< void >
    reconnect();

    asio::awaitable< void >
    read_state(websocket_stream_variant                &ws,
               async_circular_buffer< json::value, 1 > &pong_buffer);

    void
    notify_connection_state(connection_state s);

  private:
    asio::any_io_executor   exec_;
    asio::ssl::context     &sslctx_;
    ftx_websocket_key const args_;

    asio::cancellation_signal  stop_signal_;
    async_queue< std::string > write_queue_;

    // Call this function to instruct the current connection to immediately drop
    std::function< void() > on_drop_ = nullptr;

    // connection state

    connection_state   connection_state_ = connection_state::down;
    asio::steady_timer connection_state_cv_ {
        get_executor(),
        asio::steady_timer::time_point::max()
    };

    channel_state_map channel_states_;
};

auto
ftx_websocket_connector_concept::get_executor() const -> executor_type const &
{
    return exec_;
}
}   // namespace vendor::ftx
}   // namespace boost::connector

#endif
