//
// Created by hodge on 29/09/2021.
//

#ifndef BOOST_CONNECTOR_PROPERTY_MAP_HPP
#define BOOST_CONNECTOR_PROPERTY_MAP_HPP

namespace boost::connector
{

struct property_value_jump_table
{
    void (*print)(std::ostream& os, void const* target);
    void (*hash_value)(void const* target);
    void (*equals)(void const* l, void const* r);
};

template<class T>
struct print_on_stream
{
    void operator()(std::ostream& os, T const& x) const
    {
        os << x;
    }
};

/// @todo rewrite this class to support SBO
struct property_value
{
    template<class T, std::enable_if_t<!std::is_base_of_v<property_value, std::decay_t<T>>>* = nullptr>
    property_value(T&& x)
    : impl_(construct(std::forward<T>(x)))
    {

    }

    struct pv_concept
    {
        virtual void stream_out(std::ostream& os) const = 0;

    };

    template<class T>
    struct pv_impl : pv_concept
    {
        pv_impl(T&& x)
            : x_(std::move(x))
        {}

        void
        stream_out(std::ostream &os) const override
        {
            auto op = print_on_stream<T>();
            op(os, x_);
        }

        T x_;
    };

    template<class T>
    std::unique_ptr<pv_concept>
    construct(T&& x)
    {
        using impl_type = pv_impl<std::decay_t<T>>;
        return std::make_shared<impl_type>(std::forward<T>(x));
    }

    std::unique_ptr<pv_concept> impl_;
};

struct property_map_layer
{

};

struct property_map
{
};

struct mutable_property_map
{
    template<class T>
    mutable_property_map&& set(std::string_view name, T value)
    {

    }


};

mutable_property_map mutate(property_map const& source_map);

property_map fix(mutable_property_map&& m);

}
#endif   // BOOST_CONNECTOR_PROPERTY_MAP_HPP
