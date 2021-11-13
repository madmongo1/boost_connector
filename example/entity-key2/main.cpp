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

struct sha256_digest
{
    using mutable_span = std::span< unsigned char, SHA256_DIGEST_LENGTH >;
    using const_span   = std::span< unsigned char const, SHA256_DIGEST_LENGTH >;

    const_span
    data() const
    {
        return const_span(data_, data_ + SHA256_DIGEST_LENGTH);
    }

    mutable_span
    data()
    {
        return mutable_span(data_, data_ + SHA256_DIGEST_LENGTH);
    }

    bool
    operator==(sha256_digest const &r) const
    {
        return std::memcmp(data_, r.data_, sizeof(data_)) == 0;
    }

  private:
    friend std::size_t
    hash_value(sha256_digest const &self)
    {
        return boost::hash_range(self.data_, self.data_ + SHA256_DIGEST_LENGTH);
    }

  private:
    unsigned char data_[SHA256_DIGEST_LENGTH];
};

template <>
struct std::hash< sha256_digest > : boost::hash< sha256_digest >
{
};

std::ostream &
operator<<(std::ostream &os, sha256_digest const &digest)
{
    auto              span = digest.data();
    char              buffer[SHA256_DIGEST_LENGTH * 2];
    static const char hex[] = "01234567890abcdef";
    char             *p     = buffer;
    for (unsigned char b : span)
    {
        *p++ = hex[b >> 4];
        *p++ = hex[b & 0xf];
    }
    return os.write(buffer, sizeof(buffer));
}

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

void
update_sha256(SHA256_CTX *ctx, value const &val);

void
update_sha256(SHA256_CTX *ctx, bool const &val)
{
    unsigned char const tmp = val ? '1' : '0';
    SHA256_Update(ctx, &tmp, 1);
}

void
update_sha256(SHA256_CTX *ctx, array const &a)
{
    for (auto &v : a)
        update_sha256(ctx, v);
}

void
update_sha256(SHA256_CTX *ctx, std::int64_t const &v)
{
    auto buf = boost::endian::native_to_little(v);
    SHA256_Update(ctx, &buf, sizeof(buf));
}

void
update_sha256(SHA256_CTX *ctx, std::uint64_t const &v)
{
    auto buf = boost::endian::native_to_little(v);
    SHA256_Update(ctx, &buf, sizeof(buf));
}

void
update_sha256(SHA256_CTX *ctx, double v)
{
    boost::endian::native_to_little_inplace(v);
    SHA256_Update(ctx, &v, sizeof(v));
}

void
update_sha256(SHA256_CTX *ctx, string const &s)
{
    SHA256_Update(ctx, s.data(), s.size());
}

void
update_sha256(SHA256_CTX *ctx, object const &o)
{
    std::vector< std::tuple< string_view, value const * > > pointers;
    pointers.reserve(o.size());

    auto by_key = [](std::tuple< string_view, value const * > const &l,
                     std::tuple< string_view, value const * > const &r)
    { return std::get< 0 >(l) < std::get< 0 >(r); };

    for (auto &[k, v] : o)
        pointers.push_back(std::tuple(k, &v));
    std::sort(pointers.begin(), pointers.end());
    for (auto &[k, pv] : pointers)
    {
        SHA256_Update(ctx, k.data(), k.size());
        update_sha256(ctx, *pv);
    }
}

void
update_sha256(SHA256_CTX *ctx, value const &val)
{
    switch (val.kind())
    {
    case kind::array:
        return update_sha256(ctx, val.as_array());
    case kind::null:
        SHA256_Update(ctx, "null", 4);
        return;
    case kind::bool_:
        return update_sha256(ctx, val.as_bool());
    case kind::int64:
        return update_sha256(ctx, val.as_int64());
    case kind::uint64:
        return update_sha256(ctx, val.as_uint64());
    case kind::double_:
        return update_sha256(ctx, val.as_double());
    case kind::string:
        return update_sha256(ctx, val.as_string());
    case kind::object:
        return update_sha256(ctx, val.as_object());
    }
}

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

string
to_path(std::vector< string > const &dissected)
{
    return to_path(dissected.begin(), dissected.end());
}

value const &
find_path(value const                          &v,
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

struct key_base
{
    using path_set = std::set< json::string >;

    using const_iterator = path_set::const_iterator;

  protected:
    key_base(std::shared_ptr< json::value const > original, path_set index = {})
    : value_(std::move(original))
    , index_(std::move(index))
    {
    }

  public:
    json::value const &
    value() const
    {
        return *value_;
    }

    std::shared_ptr< json::value const > const &
    value_ptr() const
    {
        return value_;
    }

    const_iterator
    begin() const
    {
        return index_.begin();
    }

    const_iterator
    end() const
    {
        return index_.end();
    }

    path_set const &
    index() const
    {
        return index_;
    }

  protected:
    path_set &
    index()
    {
        return index_;
    }

  private:
    friend std::ostream &
    operator<<(std::ostream &os, key_base const &key)
    {
        std::string_view sep = "";
        for (auto &path : key.index())
        {
            os << sep << path << "=" << jsonext::find_path(key.value(), path);
            sep = ", ";
        }
        return os;
    }

  private:
    std::shared_ptr< json::value const > value_;

    /// @brief Index of used values in the value_
    path_set index_;
};

struct mutable_key;

struct immutable_key : key_base
{
    immutable_key(std::shared_ptr< json::value const > original,
                  path_set                             idx = {})
    : key_base(std::move(original), std::move(idx))
    {
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        for (auto &path : index())
        {
            SHA256_Update(&ctx, path.data(), path.size());
            jsonext::update_sha256(&ctx, jsonext::find_path(value(), path));
        }
        SHA256_Final(digest_.data().data(), &ctx);
    }

    mutable_key
    mutate() const;

    sha256_digest const &
    digest() const
    {
        return digest_;
    }

    bool
    operator==(immutable_key const &r) const
    {
        return digest_ == r.digest() && equal_contents(r);
    }

  private:
    friend std::size_t
    hash_value(immutable_key const &self)
    {
        return hash_value(self.digest_);
    }

    bool
    equal_contents(immutable_key const &r) const
    {
        auto &il = index();
        auto &ir = r.index();
        if (il.size() != ir.size())
            return false;

        auto lfirst = il.begin();
        auto rfirst = ir.begin();
        while (lfirst != il.end())
        {
            auto &lk = *lfirst;
            auto &rk = *rfirst;
            if (lk != rk)
                return false;
            auto &&lv = jsonext::find_path(value(), lk);
            auto &&rv = jsonext::find_path(r.value(), rk);
            if (lv != rv)
                return false;
            ++lfirst;
            ++rfirst;
        }
        return true;
    }

  private:
    sha256_digest digest_;
};

template <>
struct std::hash< immutable_key > : boost::hash< immutable_key >
{
};

struct mutable_key : key_base
{
    mutable_key(std::shared_ptr< json::value const > original,
                path_set                             index = {})
    : key_base(std::move(original), std::move(index))
    {
    }

    json::value const &
    use(json::string const &path)
    try
    {
        // diect the path to normalise it
        auto dissected = jsonext::dissect_path(path);

        // find on the dissected representation
        json::value const &ref = jsonext::find_path(value(), dissected);

        // convert the dissected path to canonical representation for indexing
        index().insert(jsonext::to_path(dissected));

        // return found value reference
        return ref;
    }
    catch (std::exception &)
    {
        std::throw_with_nested(std::invalid_argument(
            "mutable_key::use: " + std::string(path.begin(), path.end())));
    }

    void
    merge(immutable_key const &other)
    {
        for (auto &&path : other)
        {
            auto [i, b] = index().insert(path);
            boost::ignore_unused(i, b);
        }
    }

    immutable_key
    fix() const
    {
        return immutable_key(value_ptr(), index());
    }

  private:
    static json::string
    normalise(json::string const &in)
    {
        return jsonext::to_path(jsonext::dissect_path(in));
    }
};

mutable_key
immutable_key::mutate() const
{
    return mutable_key(value_ptr(), index());
}

void
do_use(mutable_key &k, json::string const &item)
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
        immutable_key(std::make_shared< json::value const >(std::move(config)));

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