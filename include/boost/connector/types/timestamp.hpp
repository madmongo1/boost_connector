#ifndef BOOST_CONNECTOR_TYPES_TIMESTAMP_HPP
#define BOOST_CONNECTOR_TYPES_TIMESTAMP_HPP

#include <boost/functional/hash.hpp>

#include <chrono>

namespace boost::connector
{
struct timestamp_type
{
    using native_type = std::chrono::system_clock::duration;

    explicit timestamp_type(std::chrono::system_clock::time_point tp =
                                std::chrono::system_clock::now())
    : impl_(tp.time_since_epoch())
    {
    }

    std::uint64_t
    nanos() const
    {
        return std::chrono::duration_cast< std::chrono::nanoseconds >(impl_)
            .count();
    }

    std::uint64_t
    millis() const
    {
        return std::chrono::duration_cast< std::chrono::milliseconds >(impl_)
            .count();
    }

    static timestamp_type
    now()
    {
        return timestamp_type(std::chrono::system_clock::now());
    }

    inline friend std::size_t
    hash_value(timestamp_type ts);

    inline friend bool
    operator==(timestamp_type l, timestamp_type r);

    inline friend bool
    operator<(timestamp_type l, timestamp_type r);

  private:
    native_type impl_;
};

inline std::size_t
hash_value(timestamp_type ts)
{
    using rep = timestamp_type::native_type::rep;
    return hash< rep >()(ts.impl_.count());
}

bool
operator==(timestamp_type l, timestamp_type r)
{
    return l.impl_ == r.impl_;
}

bool
operator<(timestamp_type l, timestamp_type r)
{
    return l.impl_ < r.impl_;
}

inline std::string
to_string(timestamp_type ts)
{
    return std::to_string(ts.nanos());
}

inline std::ostream &
operator<<(std::ostream &os, timestamp_type ts)
{
    return os << ts.nanos();
}

}   // namespace boost::connector

#endif
