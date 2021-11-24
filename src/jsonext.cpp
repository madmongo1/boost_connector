//
// Created by hodge on 24/11/2021.
//

#include <boost/connector/jsonext.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/json/serialize.hpp>

#include <algorithm>

namespace boost::connector::jsonext
{
namespace
{
void
update_bool(SHA256_CTX *ctx, bool val)
{
    unsigned char const tmp = val ? '1' : '0';
    SHA256_Update(ctx, &tmp, 1);
}

void
update_array(SHA256_CTX *ctx, json::array const &a)
{
    for (auto &v : a)
        update_sha256(ctx, v);
}

void
update_int64(SHA256_CTX *ctx, std::int64_t v)
{
    auto buf = boost::endian::native_to_little(v);
    SHA256_Update(ctx, &buf, sizeof(buf));
}

void
update_uint64(SHA256_CTX *ctx, std::uint64_t v)
{
    auto buf = boost::endian::native_to_little(v);
    SHA256_Update(ctx, &buf, sizeof(buf));
}

void
update_double(SHA256_CTX *ctx, double v)
{
    boost::endian::native_to_little_inplace(v);
    SHA256_Update(ctx, &v, sizeof(v));
}

void
update_string(SHA256_CTX *ctx, json::string const &s)
{
    SHA256_Update(ctx, s.data(), s.size());
}

void
update_object(SHA256_CTX *ctx, json::object const &o)
{
    std::vector< std::tuple< string_view, json::value const * > > pointers;
    pointers.reserve(o.size());

    auto by_key = [](std::tuple< string_view, json::value const * > const &l,
                     std::tuple< string_view, json::value const * > const &r)
    { return std::get< 0 >(l) < std::get< 0 >(r); };

    for (auto &[k, v] : o)
        pointers.push_back(std::tuple(k, &v));
    std::sort(pointers.begin(), pointers.end(), by_key);
    for (auto &[k, pv] : pointers)
    {
        SHA256_Update(ctx, k.data(), k.size());
        update_sha256(ctx, *pv);
    }
}
}   // namespace

void
update_sha256(SHA256_CTX *ctx, json::value const &val)
{
    switch (val.kind())
    {
    case json::kind::array:
        return update_array(ctx, val.as_array());
    case json::kind::null:
        SHA256_Update(ctx, "null", 4);
        return;
    case json::kind::bool_:
        return update_bool(ctx, val.as_bool());
    case json::kind::int64:
        return update_int64(ctx, val.as_int64());
    case json::kind::uint64:
        return update_uint64(ctx, val.as_uint64());
    case json::kind::double_:
        return update_double(ctx, val.as_double());
    case json::kind::string:
        return update_string(ctx, val.as_string());
    case json::kind::object:
        return update_object(ctx, val.as_object());
    }
}

std::vector< json::string >
dissect_path(const json::string &input)
{
    auto result = std::vector< json::string >();

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

json::string
to_path(std::vector< json::string >::const_iterator first,
        std::vector< json::string >::const_iterator last)
{
    json::string result;
    for (; first != last; ++first)
    {
        result += '/';
        result += *first;
    }
    if (result.empty())
        result += '/';
    return result;
}

json::string
to_path(const std::vector< json::string > &dissected)
{
    return to_path(dissected.begin(), dissected.end());
}

json::value const &
find_path(const json::value                          &v,
          std::vector< json::string >::const_iterator original,
          std::vector< json::string >::const_iterator last)
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

namespace
{
unsigned long
assert_unsigned_integer(json::string const &s)
{
    char *end;
    auto  ret = ::strtoul(s.c_str(), &end, 10);
    if (std::distance(s.c_str(), static_cast< char const * >(end)) != s.size())
        throw std::invalid_argument("assert_unsigned_integer: " +
                                    std::string(s.begin(), s.end()));
    return ret;
}
}   // namespace

json::value const &
subvalue(const json::value &v, const json::string &name)
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

json::value const &
find_path(const json::value &v, const std::vector< json::string > &path)
{
    return find_path(v, path.begin(), path.end());
}

json::value const &
find_path(const json::value &v, const json::string &path)
{
    return find_path(v, dissect_path(path));
}
}   // namespace boost::connector::detail::jsonext