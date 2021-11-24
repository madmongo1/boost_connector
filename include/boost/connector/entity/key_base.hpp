#ifndef BOOST_CONNECTOR_ENTITY_KEY_BASE_HPP
#define BOOST_CONNECTOR_ENTITY_KEY_BASE_HPP

#include <boost/json/value.hpp>

#include <set>

namespace boost::connector
{
/// @brief Base class of mutable and immutable keys
struct key_base
{
    /// @brief the type of set that contains unique paths of used elements
    using path_set = std::set< json::string >;

    /// @brief iterator type for used elements
    using const_iterator = path_set::const_iterator;

  protected:
    /// @brief Construct a key_base from an original value document and optional
    /// index of used elements
    /// @param original
    /// @param index
    key_base(std::shared_ptr< json::value const > original,
             path_set                             index = {});

  public:
    /// \brief return a reference to the underlying orignal value structure
    /// \return
    inline json::value const &
    value() const;

    /// \brief return a shared copy of the underlying value structure
    inline std::shared_ptr< json::value const > const &
    value_ptr() const;

    /// \brief return an iterator to the beginning of the index
    inline const_iterator
    begin() const;

    /// \brief return an iterator to one past the end of the index
    inline const_iterator
    end() const;

    /// \brief Return a reference to the index of used values
    inline path_set const &
    index() const;

  protected:
    inline path_set &
    index();

  private:
    friend std::ostream &
    operator<<(std::ostream &os, key_base const &key);

  private:
    std::shared_ptr< json::value const > value_;

    /// @brief Index of used values in the value_
    path_set index_;
};

}   // namespace boost::connector

#include <boost/connector/entity/impl/key_base.hpp>

#endif   // BOOST_CONNECTOR_ENTITY_KEY_BASE_HPP