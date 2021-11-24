#include <boost/connector/entity/immutable_key.hpp>
#include <boost/connector/entity/mutable_key.hpp>
#include <boost/connector/jsonext.hpp>
#include <openssl/sha.h>

namespace boost::connector
{
immutable_key::immutable_key(std::shared_ptr< json::value const > original,
                             path_set                             idx)
: key_base(std::move(original), std::move(idx))
{
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    for (auto &path : index())
    {
        SHA256_Update(&ctx, path.data(), path.size());
        jsonext::update_sha256(
            &ctx, jsonext::find_path(value(), path));
    }
    SHA256_Final(digest_.data().data(), &ctx);
}

mutable_key
immutable_key::mutate() const
{
    return mutable_key(value_ptr(), index());
}

std::size_t
hash_value(const immutable_key &self)
{
    return hash_value(self.digest_);
}

bool
immutable_key::equal_contents(const immutable_key &r) const
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

}   // namespace boost::connector