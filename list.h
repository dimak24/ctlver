#pragma once

#include <functional>

template <typename...>
struct List {
    using Head = void;
    using Tail = List<>;
};

template <typename H, typename... Ts>
struct List<H, Ts...> {
    using Head = H;
    using Tail = List<Ts...>;
};

namespace impl {

template <typename, typename> struct Contains;
template <typename T, typename... Ts>
struct Contains<List<Ts...>, T>
    : std::integral_constant<bool, (false || ... || std::is_same_v<Ts, T>)> {};

template <typename> struct Len;
template <typename... Ts>
struct Len<List<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};

template <typename, typename> struct Add;
template <typename T, typename... Ts>
struct Add<List<Ts...>, T> {
    using type = std::conditional_t<
        Contains<List<Ts...>, T>::value,
        List<Ts...>,
        List<Ts..., T>
    >;
};

template <typename, typename> struct PushFront;
template <typename T, typename... Ts>
struct PushFront<List<Ts...>, T> {
    using type = List<T, Ts...>;
};

template <typename, template <typename> class>
struct Select {
    using type = List<>;
};

template <typename Head, typename... Tail, template <typename> class Selector>
struct Select<List<Head, Tail...>, Selector> {
    using T = typename Select<List<Tail...>, Selector>::type;
    using type = std::conditional_t<
        Selector<Head>::value,
        typename Add<T, Head>::type,
        T
    >;
};

template <typename, template <typename> class> struct ForEach;
template <template <typename> class Functor, typename... Ts>
struct ForEach<List<Ts...>, Functor> {
    using type = List<Functor<Ts>...>;
};

template <typename Lhs, typename Rhs>
struct SetMinus {
    template <typename T>
    struct Selector_
        : std::integral_constant<bool, !Contains<Rhs, T>::value> {};

    using type = typename Select<Lhs, Selector_>::type;
};

template <typename...> struct Unite;
template <> struct Unite<> {
    using type = List<>;
};

template <typename Head>
struct Unite<Head> {
    using type = Head;
};

template <typename Head>
struct Unite<Head, List<>> : Unite<Head> {};

template <typename Lhs, typename Head, typename... Tail>
struct Unite<Lhs, List<Head, Tail...>> {
    using U_ = typename Unite<Lhs, List<Tail...>>::type;
    using type = typename Add<U_, Head>::type;
};

template <typename L1, typename L2, typename... Tail>
struct Unite<L1, L2, Tail...>
    : Unite<typename Unite<L1, L2>::type, Tail...> {};

template <typename> struct Chain;
template <typename... Ts>
struct Chain<List<Ts...>> : Unite<Ts...> {};

template <typename> struct Reverse {
    using type = List<>;
};

template <typename Head, typename... Tail>
struct Reverse<List<Head, Tail...>> {
    using type = typename Add<
        typename Reverse<List<Tail...>>::type, Head>::type;
};

} // namespace impl

template <typename L, template <typename> class Selector>
using Select = typename impl::Select<L, Selector>::type;

template <typename L, template <typename> class Functor>
using ForEach = typename impl::ForEach<L, Functor>::type;

template <typename L, typename T>
inline constexpr bool Contains = impl::Contains<L, T>::value;

template <typename L>
inline constexpr size_t Len = impl::Len<L>::value;

template <typename L, typename T>
using PushFront = typename impl::PushFront<L, T>::type;

template <typename L, typename T>
using Add = typename impl::Add<L, T>::type;

template <typename Lhs, typename Rhs>
using SetMinus = typename impl::SetMinus<Lhs, Rhs>::type;

template <typename Lhs, typename Rhs>
using Unite = typename impl::Unite<Lhs, Rhs>::type;

template <typename L>
using Chain = typename impl::Chain<L>::type;

template <typename L>
using Reverse = typename impl::Reverse<L>::type;
