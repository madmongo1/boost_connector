#include <boost/connector/entity/key.hpp>
#include <boost/connector/jsonext.hpp>
#include <boost/endian.hpp>
#include <boost/functional/hash.hpp>
#include <boost/json.hpp>
#include <openssl/sha.h>

#include <algorithm>
#include <iostream>
#include <set>
#include <span>
#include <sstream>
#include <tuple>
#include <vector>

namespace json = boost::json;


struct unwrap
{
    unwrap(std::exception &e)
    : e(e)
    {
    }

    void
    operator()(std::ostream &os, std::size_t level = 0) const
    {
        if (level)
            os << '\n';
        os << std::string(level, ' ') << "exception: " << e.what();
        try
        {
            std::rethrow_if_nested(e);
        }
        catch (std::exception &e)
        {
            auto unext = unwrap(e);
            unext(os, level + 1);
        }
        catch (...)
        {
            std::cerr << '\n'
                      << std::string(level + 1, ' ') << "exception: unknown";
        }
    }

    std::exception &e;
};

std::ostream &
operator<<(std::ostream &os, unwrap const &u)
{
    u(os);
    return os;
}

void
test(json::value const &v, json::string const &path)
{
    std::cout << "path: " << path << " ";
    try
    {
        std::cout << boost::connector::jsonext::find_path(v, path) << '\n';
    }
    catch (std::exception &e)
    {
        std::cout << unwrap(e) << '\n';
    }
}

void
do_use(boost::connector::mutable_key &k, json::string const &item)
{
    std::cout << "using item: " << item << " yields: ";
    try
    {
        std::cout << k.use(item);
    }
    catch (std::exception &e)
    {
        std::cout << "exception: " << unwrap(e);
    }
    std::cout << '\n';
}

int
main()
try
{
    static const char data[] = R"__json(
{
    "animals" : 
    [
        {
            "type": "cat",
            "name": "Felix"
        },
        { "type": "dog" }
    ]
}
    )__json";

    auto v = json::parse(data);

    test(v, "/");
    test(v, "");
    test(v, "/animals");
    test(v, "/animals/0");
    test(v, "/animals/1");
    test(v, "/animals/2");
    test(v, "/animals/felix");
    test(v, "/animals/-1");
    test(v, "/animals/0/type");

    static const char config_data[] = R"__json(
{
    "venue" : {
        "ftx-prod": {
            "type": "ftx",
            "websocket-endpoint": "wss://ftx.com",
            "rest-endpoint": "https://ftx.com",
            "api-key": "my-key",
            "api-secret": "my-secret"
        }
    }
}
    )__json";

    auto config = json::parse(config_data);
    auto root_key =
        boost::connector::immutable_key(std::make_shared< json::value const >(std::move(config)));

    auto k2 = root_key.mutate();
    do_use(k2, "venue/ftx-prod/type");
    do_use(k2, "venue/ftx-prod/foo");

    auto k3 = k2.fix();
    std::cout << "k3: " << k3.digest() << " : " << k3 << std::endl;

    auto k4 = k3.mutate();
    do_use(k4, "venue/ftx-prod/websocket-endpoint");
    auto k5 = k4.fix();
    std::cout << "k5: " << k5.digest() << " : " << k5 << std::endl;
    auto k6 = k5;
    std::cout << "k6: " << k6.digest() << " : " << k6 << std::endl;

    auto k7 = root_key.mutate();
    k7.merge(k6);
    auto k8 = k7.fix();
    std::cout << "k8: " << k8.digest() << " : " << k8 << std::endl;
    std::cout << "k6 == k8: " << std::boolalpha << (k6 == k8) << std::endl;

    return 0;
}
catch (std::exception &e)
{
    std::cout << "main: exception: " << e.what() << '\n';
    return 4;
}