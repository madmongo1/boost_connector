//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/boost_connector
//

#include <boost/connector/property_map.hpp>

namespace boost::connector
{
mutable_property_map::mutable_property_map(property_map_layer::parent_type parent_layer)
: impl_(std::make_shared< property_map_layer >(std::move(parent_layer)))
{
}

}
