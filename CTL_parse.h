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

template <typename CharT, CharT... chars>
struct CTL<std::integer_sequence<CharT, chars...>> {
    using type = typename CTLParser<CharT, chars...>::type;
};

template <typename CharT, CharT... name>
typename CTL<std::integer_sequence<CharT, name...>>::type operator""_CTL() {
    return {};
}
