#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/connector/entity/entity_cache.hpp>
#include <boost/connector/order_book.hpp>
#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector_concept.hpp>
#include <boost/connector/vendor/ftx/order_book_impl.hpp>

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
            using namespace connector;
            auto key =
                ftx_websocket_key { .url = "wss://ftx.com/ws/", .auth = {} };
            auto ftxconn = vendor::ftx::websocket_connector(
                make_lifetime_ptr<
                    vendor::ftx::ftx_websocket_connector_concept >(
                    co_await asio::this_coro::executor, sslctx, key));

            auto btcs = std::vector< order_book >();
            auto eths = std::vector< order_book >();
            for (int i = 0; i < 10; ++i)
            {
                btcs.emplace_back(
                    make_lifetime_ptr< vendor::ftx::order_book_impl >(
                        ftxconn, "BTC/USD"));
                eths.emplace_back(
                    make_lifetime_ptr< vendor::ftx::order_book_impl >(
                        ftxconn, "ETH/USD"));
            }

            auto t = asio::steady_timer(co_await asio::this_coro::executor);
            for (int i = 0; i < 10; ++i)
            {
                t.expires_after(5s);
                co_await t.async_wait(asio::use_awaitable);
                btcs[i].reset();
                eths[i].reset();
            }
        },
        asio::detached);

    ioc.run();
}