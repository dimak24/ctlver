template <typename...>
struct List;

namespace detail {

template <typename L, template <typename> class Selector>
struct ListSelect {
    using type = List<>;
};

template <typename Head, typename... Tail, template <typename> class Selector>
struct ListSelect<List<Head, Tail...>, Selector> {
    using T = typename ListSelect<List<Tail...>, Selector>::type;
    using type = std::conditional_t<
        Selector<Head>::value,
        typename T::template add<Head>,
        T
    >;
};

template <typename Lhs, typename Rhs>
struct ListMinus {
    template <typename T>
    struct Selector_ : std::integral_constant<bool, !Rhs::template contains<T>> {};

    using type = typename Lhs::template select<Selector_>;
};

template <typename Lhs, typename>
struct ListUnite {
    using type = Lhs;
};

template <typename Lhs, typename Head, typename... Tail>
struct ListUnite<Lhs, List<Head, Tail...>> {
    using U = typename ListUnite<Lhs, List<Tail...>>::type;
    using type = typename U::template add<Head>;
};

}

template <typename... Ts>
struct List {
    template <typename T>
    constexpr const static bool contains = (false || ... || std::is_same_v<Ts, T>);

    template <typename T>
    using add = std::conditional_t<contains<T>, List<Ts...>, List<T, Ts...>>;

    template <typename Rhs>
    using unite = typename detail::ListUnite<List<Ts...>, Rhs>::type;

    template <template <typename> class Selector>
    using select = typename detail::ListSelect<List<Ts...>, Selector>::type;

    template <typename Rhs>
    using setminus = typename detail::ListMinus<List<Ts...>, Rhs>::type;
};

