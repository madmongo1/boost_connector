#ifndef BOOST_CONNECTOR_INTERFACE_SNAPSHOT_HPP
#define BOOST_CONNECTOR_INTERFACE_SNAPSHOT_HPP

#include <boost/connector/condition.hpp>
#include <boost/connector/types/timestamp.hpp>

namespace boost::connector
{
struct snapshot
{
    status_code    status = status_code::error;
    std::string    source;
    timestamp_type timestamp = timestamp_type::now();

    /// @brief Check for equivalence between two snapshots.
    /// @details Two snapshots are equivalent if:
    /// - They are of the same type
    /// - They have the same status
    /// - If the status is good, the data encapsulated by the snapshot is
    /// equivalent, i.e. represents the same algorithm state
    bool
    is_equivalent(snapshot const &r) const;

    virtual void
    print(std::ostream &os, bool structured = true) const;

    virtual ~snapshot() = default;

  protected:
    virtual bool
    check_equivalent(snapshot const &r) const;
};

std::ostream &
operator<<(std::ostream &os, snapshot const &arg);

bool
equivalent(snapshot const &l, snapshot const &r);

}   // namespace boost::connector

#endif
