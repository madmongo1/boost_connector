//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/router
//

#include <boost/connector/util/websocket_stream_variant.hpp>

#include <ostream>

namespace boost::connector
{
std::ostream &
operator<<(std::ostream &os, websocket_message const &msg)
{
    if (msg.is_binary())
    {
        os << "[binary length=" << msg.as_string().size() << ']';
    }
    else
    {
        os << "[text " << msg.as_string() << ']';
    }
    return os;
}

}   // namespace boost::connector