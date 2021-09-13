#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/connector/entity/entity_cache.hpp>
#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector.hpp>

#include <iostream>

namespace asio = boost::asio;

int
main()
{
    std::cout << "Hello, World!\n";

    auto ioc    = boost::asio::io_context();
    auto sslctx = boost::asio::ssl::context(boost::asio::ssl::context::tls_client);

    auto s         = asio::ssl::stream< asio::ip::tcp::socket >(ioc, sslctx);
    s.next_layer() = asio::ip::tcp::socket(ioc);

    /*
        auto mdeps = boost::connector::mutable_dependency_map();
        mdeps.set("io_context", std::addressof(ioc));
        mdeps.set("client_ssl_context", std::addressof(sslctx));

        auto context = boost::connector::entity_context();
        context.set_dependencies(fix(std::move(mdeps)));

    */

    auto args = boost::connector::ftx_websocket_key { .url  = "wss://ftx.com/ws/",
                                                      .auth = {

                                                      } };

    auto pconn = boost::connector::make_lifetime_ptr< boost::connector::ftx_websocket_connector >(
        ioc.get_executor(), sslctx, args);

    ioc.run();
}