#include <boost/connector/entity/entity_cache.hpp>
#include <boost/asio.hpp>
#include <iostream>

int
main()
{
    auto ioc = boost::asio::io_context();

    auto mdeps = boost::connector::mutable_dependency_map();
    mdeps.set("io_context", std::addressof(ioc));



    auto context = boost::connector::entity_context();
    context.set_dependencies(fix(std::move(mdeps)));



    std::cout << "Hello, World!\n";
}