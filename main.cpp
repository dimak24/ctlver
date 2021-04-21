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

template <typename, typename>
struct ListContains {
    constexpr const static bool value = false;
};

template <typename... Elements, typename Item>
struct ListContains<List<Elements...>, Item> {
    constexpr const static bool value = (false || ... || std::is_same_v<Elements, Item>);
};

template <typename L, typename>
struct ListPushFront {
    using type = L;
};

template <typename Item, typename... Elements>
struct ListPushFront<List<Elements...>, Item> {
    using type = List<Item, Elements...>;
};

template <typename Lhs, typename>
struct ListMinus {
    using type = Lhs;
};

template <typename Head, typename... Tail, typename Rhs>
struct ListMinus<List<Head, Tail...>, Rhs> {
    using type = std::conditional_t<
        ListContains<Rhs, Head>::value,
        typename ListMinus<List<Tail...>, Rhs>::type,
        typename ListPushFront<typename ListMinus<List<Tail...>, Rhs>::type, Head>::type
    >;
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

template <typename...>
struct True {};

template <typename L, typename R>
struct Or {
    using Lhs = L;
    using Rhs = R;
};

template <typename F>
struct Not {
    using Arg = F;
};

template <typename F>
struct EG {
    using Arg = F;
};

template <typename F>
struct EU {
    using Arg = F;
};

template <typename F>
struct EX {
    using Arg = F;
};

template <typename...>
struct always_false {
    constexpr static const inline bool value = false;
};

template <char first, typename Sequence>
struct push_front {};

template <char first, char... tail>
struct push_front<first, std::integer_sequence<char, tail...>> {
    using type = std::integer_sequence<char, first, tail...>;
};

template <char first, char...>
struct cut_last {
    constexpr const static char last = first;
    using head = std::integer_sequence<char>;
};

template <char first, char second, char... tail>
struct cut_last<first, second, tail...> {
    constexpr const static char last = cut_last<second, tail...>::last;
    using head = typename push_front<first, typename cut_last<second, tail...>::head>::type;
};

template <char first, char... chars>
auto remove_braces() {
    if constexpr (!sizeof...(chars)) {
        return std::integer_sequence<char, first>{};
    }
    constexpr const char last = cut_last<chars...>::last;
    if constexpr ((first == '(' && last == ')')
               || (first == '[' && last == ']')
               || (first == '{' && last == '}')) {
        return typename cut_last<chars...>::head{};
    } else {
        static_assert(
            always_false<std::integer_sequence<char, first, chars...>>::value);
    }
}

template <typename>
struct CTL {};

template <typename CharT, CharT... chars>
struct CTLParser {
    using type = Prop<std::integer_sequence<CharT, chars...>>;
};

template <char... chars>
struct CTLParser<char, '!', chars...> {
    using type = Not<typename CTL<decltype(remove_braces<chars...>())>::type>;
};

template <char... chars>
struct CTLParser<char, 'E', 'G', chars...> {
    using type = EG<typename CTL<decltype(remove_braces<chars...>())>::type>;
};

template <char... chars>
struct CTLParser<char, 'E', 'X', chars...> {
    using type = EX<typename CTL<decltype(remove_braces<chars...>())>::type>;
};

template <char... chars>
struct CTLParser<char, 'E', 'U', chars...> {
    using type = EU<typename CTL<decltype(remove_braces<chars...>())>::type>;
};

template <typename CharT, CharT... chars>
struct CTL<std::integer_sequence<CharT, chars...>> {
    using type = typename CTLParser<CharT, chars...>::type;
};

template <typename CharT, CharT... name>
typename CTL<std::integer_sequence<CharT, name...>>::type operator""_CTL() {
    return {};
}

template <typename Model, typename CTLFormula>
struct CTLCheck {
    using Satisfy = typename Model::States;
};

template <typename L, template <typename> class Selector>
struct ListSelect {
    using type = List<>;
};

template <typename Head, typename... Tail, template <typename> class Selector>
struct ListSelect<List<Head, Tail...>, Selector> {
    using T = typename ListSelect<List<Tail...>, Selector>::type;
    using type = std::conditional_t<
        Selector<Head>::value,
        typename ListPushFront<T, Head>::type,
        T
    >;
};

template <typename Model, typename Name>
struct CTLCheck<Model, Prop<Name>> {
    template <typename Item>
    struct Selector {
        using L = typename Model::Labels::template get<Item>;
        constexpr const static bool value = ListContains<L, Prop<Name>>::value;
    };
    using Satisfy = typename ListSelect<typename Model::States, Selector>::type;
};

template <typename Lhs, typename>
struct ListUnite {
    using type = Lhs;
};

template <typename Lhs, typename Head, typename... Tail>
struct ListUnite<Lhs, List<Head, Tail...>> {
    using L = typename ListUnite<Lhs, List<Tail...>>::type;
    using type = std::conditional_t<
        ListContains<Lhs, Head>::value,
        L,
        typename ListPushFront<L, Head>::type>;
};

template <typename Model, typename Lhs, typename Rhs>
struct CTLCheck<Model, Or<Lhs, Rhs>> {
    using SatisfyLhs = typename CTLCheck<Model, Lhs>::Satisfy;
    using SatisfyRhs = typename CTLCheck<Model, Rhs>::Satisfy;
    using Satisfy = typename ListUnite<SatisfyLhs, SatisfyRhs>::type;
};

template <typename Model, typename F>
struct CTLCheck<Model, Not<F>> {
    using Satisfy = typename ListMinus<typename Model::States, typename CTLCheck<Model, F>::Satisfy>::type;
};


int main() {
    using my_list = List<Node<1>, Node<2>>;
    using my_dict = Dict<
        MapItem<Node<1>, List<>>,
        MapItem<Node<2>, my_list>
    >;

    using p = decltype("p"_prop);
    using q = decltype("q"_prop);
    using r = decltype("r"_prop);

    using formula = EX<Or<EG<Or<p, Not<q>>>, r>>;
    using model = KripkeModel<
        List<Node<1>, Node<2>, Node<3>>,
        List<Node<1>>,
        AdjList<
            MapItem<Node<1>, List<Node<2>, Node<3>>>,
            MapItem<Node<3>, List<Node<2>>>
        >,
        DefaultDict<List<>,
            MapItem<Node<1>, List<p, q>>,
            MapItem<Node<2>, List<r>>
        >
    >;

    using result = typename CTLCheck<model, Or<q, Not<r>>>::Satisfy;
    ShowType<result> _;
}
