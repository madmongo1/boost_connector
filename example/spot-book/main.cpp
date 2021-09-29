#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/connector/entity/entity_cache.hpp>
#include <boost/connector/order_book.hpp>
#include <boost/connector/vendor/ftx/interface/ftx_websocket_connector_concept.hpp>
#include <boost/connector/vendor/ftx/order_book_impl.hpp>

#include <iostream>

namespace asio = boost::asio;
using namespace std::literals;
namespace connector = boost::connector;

namespace boost::connector
{
struct any_set_layer
{
    any_set_layer(std::shared_ptr< any_set_layer const > parent = nullptr)
    : parent_(std::move(parent))
    , map_()
    {
    }

    template < class T >
    T const *
    query() const
    {
        auto i = map_.find(typeid(T));
        if (i == map_.end())
            if (parent_)
                return parent->query< T >();
            else
                return nullptr;
        else
            return std::any_cast< T >(std::addressod(i->second));
    }

    template < class T >
    T const &
    require() const
    try
    {
        return std::any_cast< T >(map_.at(typeid(T)));
    }
    catch (std::exception &)
    {
        std::ostringstream ss;
        ss << "any_set: " << boost::demangle(typeid(t).name()) << " not found";
        std::throw_with_nested(std::invalid_argument(std::move(ss).str()));
    }

    std::shared_ptr< any_set_layer const >          parent_;
    std::unordered_map< std::type_index, std::any > map_;
};

struct mutable_any_set
{
    mutable_any_set()
    : impl_(std::make_shared< any_set_layer >())
    {
    }

    mutable_any_set(any_set_layer &&my_layer)
    : impl_(std::make_shared< any_set_layer >(std::move(my_layer)))
    {
    }

    template < class T >
    mutable_any_set &
    set(T &&x)
    {
        impl_->set(std::forward< T >(x));
    }

    std::shared_ptr< any_set_layer >
    impl_ptr() &&
    {
        return std::move(impl_);
    }

    template < class T >
    T const *
    query() const
    {
        return impl_->template query< T >();
    }

    template < class T >
    T const &
    require() const
    {
        return impl_->template require< T >();
    }

  private:
    std::shared_ptr< any_set_layer > impl_;
};

struct any_set
{
    any_set(mutable_any_set &&mset)
    : impl_(std::move(mset).impl_ptr())
    {
    }

    mutable_any_set
    mutate()
    {
        return mutable_any_set(any_set_layer(impl_));
    }

    std::shared_ptr< any_set_layer const > impl_ = nullptr;
};

struct entity_context
{
    using mutex_type = std::recursive_mutex;
    using lock_type  = std::unique_lock< mutex_type >;

    lock_type
    lock();

  private:
    struct cache
    {
        mutex_type mutex_;
    };
    std::shared_ptr< cache > cache_;
};

template < class Interface >
struct interface_tag
{
};

struct order_book_handle
{
    /// Construct an order_book from a context
    template < class T >
    order_book_handle(interface_tag< T > tag,
                      entity_context     context,
                      std::string        base,
                      std::string        term)
    : lifetime_(
          acquire(tag, std::move(context), std::move(base), std::move(term)))
    {
    }

  private:
    template < class T >
    std::shared_ptr< interface::order_book_concept >
    acquire(interface_tag< T > tag,
            entity_context &   context,
            std::string        base,
            std::string        term)
    {
        context.set_params(context.mutate_params().set("base", std::move(base)).set("term", std::move(base));
        auto candidate = context.construct<>
        auto lock      = context.lock();
    }

  private:
    std::shared_ptr< interface::order_book_concept > lifetime_;
};
}   // namespace boost::connector

int
main()
{
    std::cout << "Hello, World!\n";

    auto ioc = boost::asio::io_context();
    auto sslctx =
        boost::asio::ssl::context(boost::asio::ssl::context::tls_client);

    auto s         = asio::ssl::stream< asio::ip::tcp::socket >(ioc, sslctx);
    s.next_layer() = asio::ip::tcp::socket(ioc);

    auto mdeps = boost::connector::mutable_dependency_map();
    mdeps.set("io_context", std::addressof(ioc));
    mdeps.set("client_ssl_context", std::addressof(sslctx));

    auto context = boost::connector::entity_context();
    context.set_dependencies(fix(std::move(mdeps)));

    asio::co_spawn(
        ioc.get_executor(),
        [&sslctx]() -> asio::awaitable< void >
        {
            using namespace connector;
            auto key =
                ftx_websocket_key { .url = "wss://ftx.com/ws/", .auth = {} };
            auto ftxconn = vendor::ftx::websocket_connector(
                make_lifetime_ptr<
                    vendor::ftx::ftx_websocket_connector_concept >(
                    co_await asio::this_coro::executor, sslctx, key));

            auto btcs = std::vector< order_book >();
            auto eths = std::vector< order_book >();
            for (int i = 0; i < 10; ++i)
            {
                btcs.emplace_back(
                    make_lifetime_ptr< vendor::ftx::order_book_impl >(
                        ftxconn, "BTC/USD"));
                eths.emplace_back(
                    make_lifetime_ptr< vendor::ftx::order_book_impl >(
                        ftxconn, "ETH/USD"));
            }

            auto t = asio::steady_timer(co_await asio::this_coro::executor);
            for (int i = 0; i < 10; ++i)
            {
                t.expires_after(5s);
                co_await t.async_wait(asio::use_awaitable);
                btcs[i].reset();
                eths[i].reset();
            }
        },
        asio::detached);

    ioc.run();
}