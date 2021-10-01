//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#include <boost/connector/property_value.hpp>

namespace boost::connector
{
property_value::property_value(const char *s)
: property_value(std::string(s))
{
}

property_value::property_value(string_view s)
: property_value(std::string(s.begin(), s.end()))
{
}

property_value::property_value(property_value &&other) noexcept
: jt_(&property_value_void_jump_table)
, sbo_ {}
{
    other.jt_->move_construct(&sbo_, &other.sbo_);
    jt_ = std::exchange(other.jt_, &property_value_void_jump_table);
}

property_value &
property_value::operator=(property_value &&other) noexcept
{
    // make move-constructed copy
    auto tmp = property_value(std::move(other));

    // destroy our implementation
    jt_->destroy(&sbo_);
    jt_ = &property_value_void_jump_table;

    // move-construct our implementation from copy
    tmp.jt_->move_construct(&sbo_, &tmp);
    jt_ = std::exchange(tmp.jt_, &property_value_void_jump_table);

    return *this;
}

property_value::~property_value()
{
    jt_->destroy(&sbo_);
}

std::size_t
hash_value(property_value const &arg)
{
    if (arg.jt_)
        return arg.jt_->hash_value(&arg.sbo_);
    else
        return 0;
}

bool
property_value::operator==(property_value const &r) const
{
    if (jt_->info() != r.jt_->info())
        return false;
    return jt_->equal(&sbo_, &r.sbo_);
}

std::ostream &
operator<<(std::ostream &os, property_value const &arg)
{
    if (arg.jt_)
        arg.jt_->to_ostream(os, &arg.sbo_);
    else
        os << "empty";
    return os;
}

}   // namespace boost::connector