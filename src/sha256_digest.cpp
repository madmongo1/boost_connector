#include <boost/connector/sha256_digest.hpp>

#include <ostream>

namespace boost::connector
{
std::size_t
hash_value(sha256_digest const &self)
{
    return boost::hash_range(self.data_,
                             self.data_ + sha256_digest::digest_length);
}

std::ostream &
operator<<(std::ostream &os, sha256_digest const &digest)
{
    auto              span = digest.data();
    char              buffer[sha256_digest::digest_length * 2];
    static const char hex[] = "01234567890abcdef";
    char             *p     = buffer;
    for (unsigned char b : span)
    {
        *p++ = hex[b >> 4];
        *p++ = hex[b & 0xf];
    }
    return os.write(buffer, sizeof(buffer));
}

}   // namespace boost::connector