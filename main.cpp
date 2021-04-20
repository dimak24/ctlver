#include <iostream>
#include <functional>

template <int ID_>
struct Node {
    constexpr const static int ID = ID_;
};

template <typename S, typename T>
struct Edge {
    using Source = S;
    using Target = T;
};

template <typename...>
struct List {};

template <typename H, typename... Ts>
struct List<H, Ts...> {
    using Head = H;
    using Tail = List<Ts...>;
};

template <typename K, typename V>
struct MapItem {
    using Key = K;
    using Value = V;
};

template <typename Default, typename...>
struct DefaultDict {
    template <typename>
    using get = Default;
};

template <typename Default, typename Key, typename Value, typename... Items>
struct DefaultDict<Default, MapItem<Key, Value>, Items...> {
    template <typename Key_>
    using get = std::conditional_t<
        std::is_same_v<Key, Key_>,
        Value,
        typename DefaultDict<Default, Items...>::template get<Key_>
    >;
};

template <typename... Items>
using Dict = DefaultDict<void, Items...>;

template <typename... Items>
using AdjList = DefaultDict<List<>, Items...>;

template <typename CharT, CharT... chars>
struct get_name {
    constexpr static inline const char value[sizeof...(chars) + 1] = {
        chars..., '\0'};

    explicit constexpr get_name(
        const std::integer_sequence<CharT, chars...>&) noexcept
    {}
};

template <typename Name_>
struct Prop {
    using Name = Name_;
    constexpr static inline const auto name =
        static_cast<const char*>(decltype(get_name(Name{}))::value);
};

template <typename CharT, CharT... name>
Prop<std::integer_sequence<CharT, name...>> operator""_prop() {
    return {};
}


template <typename S, typename S0, typename R, typename L>
struct KripkeModel {
    using States = S;
    using InitialStates = S0;
    using Relation = R;
    using Labels = L;
};

template <typename>
struct ShowType {
    explicit ShowType() = delete;
};
int main() {
    using my_list = List<Node<1>, Node<2>>;
    using my_dict = Dict<
        MapItem<Node<1>, List<>>,
        MapItem<Node<2>, my_list>
    >;

    std::cout << decltype("kek"_prop)::name << std::endl;

    /*
    ShowType<
        KripkeModel<
            List<Node<1>, Node<2>, Node<3>>,
            List<Node<1>>,
            AdjList<
                MapItem<Node<1>, List<Node<2>, Node<3>>>,
                MapItem<Node<3>, List<Node<2>>>
            >,
            Dict<
                MapItem<Node<1>, List<decltype("p"_prop), decltype("q"_prop)>>,
                MapItem<Node<2>, List<decltype("p"_prop)>>
            >
        >
    > _;
    */
}
