#include <boost/algorithm/string.hpp>
#include <boost/asio/error.hpp>
#include <boost/connector/config/error.hpp>
#include <boost/connector/util/url.hpp>

#include <regex>

namespace boost::connector
{
namespace
{
std::string
deduce_port(std::string const &scheme, std::string port)
{
    using boost::algorithm::iequals;

    if (port.empty())
    {
        if (iequals(scheme, "ws") or iequals(scheme, "http"))
            port = "http";
        else if (iequals(scheme, "wss") or iequals(scheme, "https"))
            port = "https";
        else
            throw system_error(asio::error::invalid_argument, "can't deduce port");
    }

    return port;
}

transport_type
deduce_transport(std::string const &scheme, std::string const &port)
{
    using boost::algorithm::iequals;
    if (scheme.empty())
    {
        if (port.empty())
            return transport_type::tcp;

        if (iequals(port, "http") or iequals(port, "ws") or iequals(port, "80"))
            return transport_type::tcp;

        if (iequals(port, "https") or iequals(port, "wss") or iequals(port, "443"))
            return transport_type::tls;

        throw system_error(asio::error::invalid_argument, "cannot deduce transport");
    }
    else
    {
        if (iequals(scheme, "http") or iequals(scheme, "ws"))
            return transport_type::tcp;

        if (iequals(scheme, "https") or iequals(scheme, "wss"))
            return transport_type::tls;

        throw system_error(asio::error::invalid_argument, "invalid scheme");
    }
}

std::string
build_target(std::string const &path, std::string const &query, std::string const &fragment)
{
    std::string result;

    if (path.empty())
        result = "/";
    else
        result = path;

    if (!query.empty())
        result += "?" + query;

    if (!fragment.empty())
        result += "#" + fragment;

    return result;
}

}   // namespace

url_parts
decode_url(std::string const &url)
{
    //
    // This algorithm uses a regex to crack a fully formed URL.
    // It is by no means perfect, but is "good enough" for the common cases that will be found in this library.
    // For example, it does not handle the username/password prefex on the authority (which you should not be using
    // anyway)
    //
    static auto url_regex =
        std::regex(R"regex((ws|wss|http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\x3f?([^ #]*)#?([^ ]*))regex",
                   std::regex_constants::icase);
    auto match = std::smatch();
    if (not std::regex_match(url, match, url_regex))
        throw system_error(asio::error::invalid_argument, "invalid url");

    auto &protocol = match[1];
    auto &host     = match[2];
    auto &port_ind = match[3];
    auto &path     = match[4];
    auto &query    = match[5];
    auto &fragment = match[6];

    return url_parts { .hostname  = host,
                       .service   = deduce_port(protocol, port_ind),
                       .path_etc  = build_target(path, query, fragment),
                       .transport = deduce_transport(protocol, port_ind) };
}

}   // namespace boost::connector