#include <boost/json.hpp>

#include <iostream>
#include <sstream>

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

namespace jsonext
{
using namespace json;

unsigned long
assert_unsigned_integer(string const &s)
{
    char *end;
    auto  ret = ::strtoul(s.c_str(), &end, 10);
    if (std::distance(s.c_str(), static_cast< char const * >(end)) != s.size())
        throw std::invalid_argument("assert_unsigned_integer: " +
                                    std::string(s.begin(), s.end()));
    return ret;
}

value const &
subvalue(value const &v, json::string const &name)
{
    auto make_exception = [&]
    {
        return std::invalid_argument("subvalue: " +
                                     std::string(name.begin(), name.end()));
    };

    if (auto o = v.if_object())
    {
        return o->at(name);
    }
    else if (auto a = v.if_array())
    {
        try
        {
            return a->at(assert_unsigned_integer(name.c_str()));
        }
        catch (std::exception &)
        {
            std::throw_with_nested(make_exception());
        }
    }
    else
    {
        throw make_exception();
    }
}

std::vector< string >
dissect_path(string const &input)
{
    auto result = std::vector< string >();

    auto current = input.begin();
    auto end     = input.end();

    auto is_sep = [](char s) { return s == '/'; };

    for (; current != end;)
    {
        if (is_sep(*current))
        {
            ++current;
            continue;
        }
        auto sep = std::find_if(current, end, is_sep);
        result.emplace_back(current, sep);
        current = sep;
    }

    return result;
}

string
to_path(std::vector< string >::const_iterator first,
        std::vector< string >::const_iterator last)
{
    string result;
    for (; first != last; ++first)
    {
        result += '/';
        result += *first;
    }
    if (result.empty())
        result += '/';
    return result;
}

value const &
find_path(value const &                         v,
          std::vector< string >::const_iterator original,
          std::vector< string >::const_iterator last)
{
    auto pcurrent = &v;
    auto first    = original;
    try
    {
        while (first != last)
        {
            pcurrent = &(subvalue(*pcurrent, *first));
            ++first;
        }
    }
    catch (std::exception &e)
    {
        std::stringstream ss;
        ss << __func__ << ": error at " << to_path(original, first)
           << " evaluating " << *first;
        std::throw_with_nested(std::invalid_argument(std::move(ss).str()));
    }

    return *pcurrent;
}

value const &
find_path(value const &v, std::vector< string > const &path)
{
    return find_path(v, path.begin(), path.end());
}

value const &
find_path(value const &v, string const &path)
{
    return find_path(v, dissect_path(path));
}
}   // namespace jsonext

void
test(json::value const &v, json::string const &path)
{
    std::cout << "path: " << path << " ";
    try
    {
        std::cout << jsonext::find_path(v, path) << '\n';
    }
    catch (std::exception &e)
    {
        std::cout << unwrap(e) << '\n';
    }
}

int
main()
try
{
    static const char data[] = R"__json(
{
    "animals" : 
    [
        { "type": "cat" },
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
    return 0;
}
catch (std::exception &e)
{
    std::cout << "main: exception: " << e.what() << '\n';
    return 4;
}