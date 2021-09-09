#ifndef BOOST_CONNECTOR__DEPENDENCY__DEPENDENCY_MAP__HPP
#define BOOST_CONNECTOR__DEPENDENCY__DEPENDENCY_MAP__HPP

#include <any>
#include <string>
#include <unordered_map>

namespace boost::connector
{
struct dependency_map_impl
{
    std::any const *
    query(std::string const &name) const;

    void
    set(std::string name, std::any value);

  private:
    using map_type = std::unordered_map< std::string, std::any >;
    map_type map_;
};

struct mutable_dependency_map;

struct dependency_map
{
    dependency_map();

    dependency_map(dependency_map const &) noexcept = default;

    dependency_map(dependency_map &&) noexcept = default;

    dependency_map &
    operator=(dependency_map const &) noexcept = default;

    dependency_map &
    operator=(dependency_map &&) noexcept = default;

    friend dependency_map
    fix(mutable_dependency_map &&source);

    friend mutable_dependency_map
    clone(dependency_map const &source);

    template < class T >
    T &
    query(std::string const &name)
    {
        return std::any_cast< T >(impl_->query(name));
    }

    template < class T >
    T &
    require(std::string const &name)
    {
        auto pany   = impl_->query(name);
        auto result = std::any_cast< T >(pany);
        if (result)
            return result;
        else if (pany)
            throw std::runtime_error("dependency_map type mismatch");
        else
            throw std::runtime_error("dependency_map: lookup failed");
    }

  private:
    dependency_map(std::shared_ptr< dependency_map_impl const > impl) noexcept;

  private:
    std::shared_ptr< dependency_map_impl const > impl_;
};

struct mutable_dependency_map
{
    /// Create an empty dependency map
    mutable_dependency_map();

    mutable_dependency_map(mutable_dependency_map const &) = delete;

    mutable_dependency_map &
    operator=(mutable_dependency_map const &) = delete;

    template < class T >
    void
    set(std::string_view name, T value)
    {
        impl_->set(std::string(name.begin(), name.end()), std::any(std::move(value)));
    }

  private:
    mutable_dependency_map(std::shared_ptr< dependency_map_impl > impl) noexcept;

    friend dependency_map
    fix(mutable_dependency_map &&source);

    friend mutable_dependency_map
    clone(dependency_map const &source);

  private:
    std::shared_ptr< dependency_map_impl > impl_;
};

}   // namespace boost::connector

#endif
