#ifndef BOOST_CONNECTOR_SNAPSHOT_CONDITION_HPP
#define BOOST_CONNECTOR_SNAPSHOT_CONDITION_HPP

#include <boost/connector/util/describe_operators.hpp>

#include <chrono>
#include <string>
#include <vector>

namespace boost::connector
{
BOOST_DEFINE_ENUM_CLASS(status_code, good, not_ready, error)

struct condition
{
    status_code                           status_;
    std::chrono::system_clock::time_point timestamp_;
    std::vector< std::string >            infomation_;
};

}   // namespace boost::connector

#endif
