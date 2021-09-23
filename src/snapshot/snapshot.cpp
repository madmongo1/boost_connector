//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#include <boost/connector/snapshot/snapshot.hpp>

namespace boost::connector
{
bool
snapshot::is_equivalent(snapshot const &r) const
{
    if (typeid(*this) != typeid(r))
        return false;
    if (status != r.status)
        return false;
    if (source != r.source)
        return false;
    if (status == status_code::good)
        return check_equivalent(r);
    else
        return true;
}

void
snapshot::print(std::ostream &os, bool structured) const
{
    os << "status: " << status << " "
       << "timestamp: " << timestamp << "source: " << source;
    if (structured)
    {
        os << '\n';
    }
}

bool
snapshot::check_equivalent(snapshot const &r) const
{
    return true;
}

std::ostream &
operator<<(std::ostream &os, snapshot const &arg)
{
    arg.print(os, true);
    return os;
}

bool
equivalent(snapshot const &l, snapshot const &r)
{
    return l.is_equivalent(r);
}

}   // namespace boost::connector