#include <iostream>
#include <functional>

#include "list.h"
#include "dict.h"
#include "graph.h"

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

// E (L U R)
template <typename L, typename R>
struct EU {
    using Lhs = L;
    using Rhs = R;
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

// template <char... chars>
// struct CTLParser<char, 'E', 'U', chars...> {
//     using type = EU<typename CTL<decltype(remove_braces<chars...>())>::type>;
// };

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


template <typename Model, typename Name>
struct CTLCheck<Model, Prop<Name>> {
    template <typename Item>
    struct Selector_ {
        using L = Get<typename Model::Labels, Item>;
        constexpr const static bool value = Contains<L, Prop<Name>>;
    };
    using Satisfy = Select<typename Model::States, Selector_>;
};

template <typename Model, typename Lhs, typename Rhs>
struct CTLCheck<Model, Or<Lhs, Rhs>> {
    using Satisfy = Unite<
        typename CTLCheck<Model, Lhs>::Satisfy,
        typename CTLCheck<Model, Rhs>::Satisfy
    >;
};

template <typename Model, typename F>
struct CTLCheck<Model, Not<F>> {
    using Satisfy = SetMinus<
        typename Model::States,
        typename CTLCheck<Model, F>::Satisfy
    >;
};

template <typename>
struct Sources_;

template <typename... Edges>
struct Sources_<List<Edges...>> {
    using type = List<typename Edges::Source...>;
};


template <typename Model, typename F>
struct CTLCheck<Model, EX<F>> {
    using FSatisfy_ = typename CTLCheck<Model, F>::Satisfy;

    template <typename Edge>
    struct Selector_ : std::integral_constant<
        bool, Contains<FSatisfy_, typename Edge::Target>> {};

    using Satisfy = typename Sources_<Select<
        typename impl::MakeEdgeList<
            typename Model::Relation>::type, Selector_>>::type;
};


int main() {
    using my_list = List<Node<1>, Node<2>>;
    using my_dict = Dict<
        KV<Node<1>, List<>>,
        KV<Node<2>, my_list>
    >;

    using p = decltype("p"_prop);
    using q = decltype("q"_prop);
    using r = decltype("r"_prop);

    using formula = EX<r>;
    using model = KripkeModel<
        List<Node<1>, Node<2>, Node<3>>,
        List<Node<1>>,
        AdjList<
            KV<Node<1>, List<Node<2>, Node<3>>>,
            KV<Node<3>, List<Node<2>>>
        >,
        DefaultDict<List<>,
            KV<Node<1>, List<p, q>>,
            KV<Node<2>, List<r>>
        >
    >;

    /**
     *      1 ---> 2
     *      |  ----^
     *      v /
     *      3 <--> 4
     */


    using graph = Graph<
        List<Node<1>, Node<2>, Node<3>, Node<4>>,
        List<
            Edge<Node<1>, Node<2>>,
            Edge<Node<1>, Node<3>>,
            Edge<Node<3>, Node<2>>,
            Edge<Node<3>, Node<4>>,
            Edge<Node<4>, Node<3>>
        >
    >;

    using reachable = typename impl::Reachable<graph, List<Node<4>>>::type;
    ShowType<reachable> _;

    //ShowType<Update<Dict<KV<float, float>>, int, int>> _;
    // ShowType<typename Graph<List<Node<1>, Node<2>, Node<3>>, typename graph::Adj>::Edges> _;

    std::cout << Contains<my_dict, Node<3>> << std::endl;

    using result = typename CTLCheck<model, formula>::Satisfy;
    ShowType<result> _;
}
