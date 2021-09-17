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
#include <boost/connector/vendor/ftx/ftx_config.hpp>
#include <boost/describe.hpp>
#include <boost/json/value.hpp>
#include <boost/signals2/signal.hpp>

#include <memory>
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
struct ftx_websocket_connector_concept
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
                                    boost::asio::ssl::context &  sslctx,
                                    ftx_websocket_key const &    key);

    virtual void
    start() override final;

    virtual void
    stop() override final;

    /// return a reference to the internal executor.
    ///
    /// Dependent entities may use this executor to co-ordinate work with this
    /// connector.
    /// @note Safe to call from any thread or executor
    inline executor_type const &
    get_executor() const;

    using channel_slot = std::function< void(json::value) >;

    /// @brief Set the callback for a subscription event arriving from upstream
    /// @note the callback will be called on the executor of this object
    /// @param index a channel_market_pair indicatinf which channel and market
    /// are of interest
    /// @param cb a function object to be called when a message arrives
    /// @pre This function must be called from within the executor returned by
    /// get_executor()
    void
    on(channel_market_pair const &index, channel_slot cb);

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

    struct channel_state
    {
        using timer_t    = asio::steady_timer;
        using time_point = timer_t::time_point;

        channel_state(executor_type exec)
        : cv(std::move(exec), time_point::max())
        {
        }

        std::size_t        interest = 1;
        bool               acquired = false;
        asio::steady_timer cv;
    };

    using channel_state_map =
        std::unordered_map< channel_market_pair,
                            channel_state,
                            boost::hash< channel_market_pair >,
                            std::equal_to<> >;
    using channel_iterator = channel_state_map::iterator;

    /// @brief represents unique ownership of a channel on the connector
    struct channel_token
    {
        channel_token(ftx_websocket_connector_concept *host = nullptr,
                      channel_iterator                 iter = {})
        : host_(host)
        , iter_(iter)
        {
        }

        channel_token(channel_token const &) = delete;
        channel_token(channel_token &&other) noexcept
        : host_(std::exchange(other.host_, nullptr))
        , iter_(std::exchange(other.iter_, {}))
        {
        }

        channel_token &
        operator=(channel_token const &) = delete;
        channel_token &
        operator=(channel_token &&) = delete;

        ~channel_token()
        {
            if (host_)
                host_->release_channel(iter_);
        }

        ftx_websocket_connector_concept *host_;
        channel_iterator                 iter_;
    };

    asio::awaitable< channel_token >
    acquire_channel(channel_market_pair const &index);

    void
    release_channel(channel_iterator iter);

  private:
    asio::awaitable< void >
    run();

    asio::awaitable< void >
    reconnect();

    asio::awaitable< void >
    read_state(websocket_stream_variant &               ws,
               async_circular_buffer< json::value, 1 > &pong_buffer);

    void
    notify_connection_state(connection_state s);

  private:
    asio::any_io_executor   exec_;
    asio::ssl::context &    sslctx_;
    ftx_websocket_key const args_;

    asio::cancellation_signal  stop_signal_;
    async_queue< std::string > write_queue_;

    // Set this latch to instruct the current connection to immediately drop
    std::function< void() > on_drop_ = nullptr;

    // connection state

    connection_state   connection_state_ = connection_state::down;
    asio::steady_timer connection_state_cv_ {
        get_executor(),
        asio::steady_timer::time_point::max()
    };

    channel_state_map channel_states_;

    // subscription management
    using subscribe_map = std::unordered_map< channel_market_pair,
                                              channel_slot,
                                              util::hash_any,
                                              std::equal_to<> >;
    subscribe_map signals_;
};

auto
ftx_websocket_connector_concept::get_executor() const -> executor_type const &
{
    return exec_;
}
}   // namespace vendor::ftx
}   // namespace boost::connector

#endif
