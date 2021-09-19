#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/connector/entity/entity_cache.hpp>
#include <boost/connector/price_ladder.hpp>
#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector_concept.hpp>
#include <boost/connector/vendor/ftx/price_ladder_impl.hpp>

#include <iostream>

namespace asio = boost::asio;
using namespace std::literals;
namespace connector = boost::connector;

int
main()
{
    std::cout << "Hello, World!\n";

    auto ioc = boost::asio::io_context();
    auto sslctx =
        boost::asio::ssl::context(boost::asio::ssl::context::tls_client);

    auto s         = asio::ssl::stream< asio::ip::tcp::socket >(ioc, sslctx);
    s.next_layer() = asio::ip::tcp::socket(ioc);

    /*
        auto mdeps = boost::connector::mutable_dependency_map();
        mdeps.set("io_context", std::addressof(ioc));
        mdeps.set("client_ssl_context", std::addressof(sslctx));

        auto context = boost::connector::entity_context();
        context.set_dependencies(fix(std::move(mdeps)));

    */

    asio::co_spawn(
        ioc.get_executor(),
        [&sslctx]() -> asio::awaitable< void >
        {
            auto key =
                connector::ftx_websocket_key { .url  = "wss://ftx.com/ws/",
                                               .auth = {} };
            auto ftxconn = boost::connector::vendor::ftx::websocket_connector(
                connector::make_lifetime_ptr<
                    connector::vendor::ftx::ftx_websocket_connector_concept >(
                    co_await asio::this_coro::executor, sslctx, key));

            auto ladder1 = boost::connector::price_ladder(
                connector::make_lifetime_ptr<
                    boost::connector::vendor::ftx::price_ladder_impl >(
                    ftxconn));
            auto ladder2 = boost::connector::price_ladder(
                connector::make_lifetime_ptr<
                    boost::connector::vendor::ftx::price_ladder_impl >(
                    ftxconn));

            auto t = asio::steady_timer(co_await asio::this_coro::executor, 5s);
            co_await t.async_wait(asio::use_awaitable);
            ladder1.reset();
            t.expires_after(5s);
            co_await t.async_wait(asio::use_awaitable);
        },
        asio::detached);

    ioc.run();
}