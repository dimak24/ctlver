template <typename K, typename V>
struct KV {
    using Key = K;
    using Value = V;
};

template <typename Default, typename...>
struct DefaultDict {
    template <typename> using get = Default;
};

template <typename Default, typename Key, typename Value, typename... Items>
struct DefaultDict<Default, KV<Key, Value>, Items...> {
    template <typename K>
    using get = std::conditional_t<
        std::is_same_v<Key, K>,
        Value,
        typename DefaultDict<Default, Items...>::template get<K>>;

    template <typename K>
    constexpr const static inline bool contains = (
        std::is_same_v<Key, K> || ... || std::is_same_v<typename Items::Key, K>);
};

template <typename... Items>
using Dict = DefaultDict<void, Items...>;
