#include <boost/connector/entity/entity_cache.hpp>

#include <iostream>

int
main()
{
    auto cache = boost::connector::entity_cache();

    std::cout << "Hello, World!\n";
}