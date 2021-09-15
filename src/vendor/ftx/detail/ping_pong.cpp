//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/connector
//

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/connector/vendor/ftx/detail/ping_pong.hpp>

#include <iostream>

namespace boost::connector::vendor::ftx::detail
{
asio::awaitable< void >
run_ping_pong(async_queue< std::string > &write_queue, async_circular_buffer< json::value, 1 > &pong_event)
try
{
    std::cout << __func__ << "() - enter\n";

    using namespace std::literals;
    using namespace asio::experimental::awaitable_operators;

    // timer to track time to next ping plus timeout of corresponding pong
    auto timer     = asio::steady_timer(co_await asio::this_coro::executor);
    auto next_ping = asio::steady_timer::clock_type::now();

    // wait for the timer to time out. Do not throw on cancellation
    auto timeout = [&timer, &next_ping]() -> asio::awaitable< void >
    {
        timer.expires_at(next_ping);
        co_await timer.async_wait(asio::experimental::as_tuple(asio::use_awaitable));
    };

    // infinite loop until cancelled
    for (;;)
    {
        // we are at a timer threshold. Compute the next threshold time.
        next_ping += 15s;

        // queue the ping to be sent
        write_queue.push(R"json({"op": "ping"})json"s);

        // wait for either a pong or a timeout
        switch (auto which = co_await(pong_event.pop() || timeout()); which.index())
        {
        case 0:
            // pong received, wait for timeout before sending next ping
            co_await timeout();
            break;

        case 1:
            // timed out, throw to cause teardown of connection
            throw std::runtime_error("ftx_websocket_connector: ping pong timeout");
        }
    }
    std::cout << __func__ << "() - exit\n";
}
catch (std::exception &e)
{
    std::cout << __func__ << "() - exception: " << e.what() << "\n";
}

}   // namespace boost::connector::vendor::ftx::detail