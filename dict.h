template <typename K, typename V>
struct KV {
    using Key = K;
    using Value = V;
};

template <typename, typename...>
struct DefaultDict {};

template <typename... Items>
using Dict = DefaultDict<void, Items...>;

namespace impl {

template <typename, typename>
struct Get;

template <typename Default, typename K>
struct Get<DefaultDict<Default>, K> {
    using type = Default;
};

template <
    typename K,
    typename Default,
    typename Key, typename Value, typename... Items>
struct Get<DefaultDict<Default, KV<Key, Value>, Items...>, K> {
    using type = std::conditional_t<
        std::is_same_v<Key, K>,
        Value,
        typename Get<DefaultDict<Default, Items...>, K>::type
    >;
};

template <typename, typename> struct Contains;

template <typename Default, typename K, typename... Items>
struct Contains<DefaultDict<Default, Items...>, K>
    : std::integral_constant<bool, (
        false || ... || std::is_same_v<typename Items::Key, K>)> {};

template <typename, typename>
struct Add_;

template <typename Default, typename Item, typename...Items>
struct Add_<DefaultDict<Default, Items...>, Item> {
    using type = DefaultDict<Default, Item, Items...>;
};

template <typename, typename, typename>
struct Update;

template <typename Default, typename Key, typename Value>
struct Update<DefaultDict<Default>, Key, Value> {
    using type = DefaultDict<Default, KV<Key, Value>>;
};

template <
    typename K,
    typename V,
    typename Default,
    typename Key,
    typename Value,
    typename... Items>
struct Update<DefaultDict<Default, KV<Key, Value>, Items...>, K, V> {
    using Tail_ = typename Update<
        DefaultDict<Default, Items...>, K, V>::type;
    using type = std::conditional_t<
        std::is_same_v<Key, K>,
        DefaultDict<Default, KV<K, V>, Items...>,
        typename Add_<Tail_, KV<Key, Value>>::type
    >;
};

}

template <typename D, typename K>
using Get = typename impl::Get<D, K>::type;

template <typename D, typename K>
constexpr const static inline bool Contains = impl::Contains<D, K>::value;

template <typename D, typename Key, typename Value>
using Update = typename impl::Update<D, Key, Value>::type;
