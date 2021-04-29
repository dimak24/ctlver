#include <functional>

#include "logic.h"
#include "list.h"

template <typename...>
using always_false = std::false_type;

namespace tokens {

struct LeftParen {};
struct RightParen {};

template <typename Name>
using Prop = Prop<Name>;

struct Not {};
struct And {};
struct Or {};
struct Implies {};

}

constexpr bool is_space(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r';
}

constexpr bool is_alpha(char ch)
{
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

template <char... chars>
using char_sequence = std::integer_sequence<char, chars...>;

template <typename, typename = void> struct STMTLexer {};

template <>
struct STMTLexer<char_sequence<>> {
    using type = List<>;
};

template <char first, char... tail>
struct STMTLexer<char_sequence<first, tail...>,
                 std::enable_if_t<is_space(first)>> {
    using type = typename STMTLexer<char_sequence<tail...>>::type;
};

template <char... tail>
struct STMTLexer<char_sequence<'(', tail...>> {
    using type = PushFront<
        typename STMTLexer<char_sequence<tail...>>::type,
        tokens::LeftParen>;
};

template <char... tail>
struct STMTLexer<char_sequence<')', tail...>> {
    using type = PushFront<
        typename STMTLexer<char_sequence<tail...>>::type,
        tokens::RightParen>;
};

template <char... tail>
struct STMTLexer<char_sequence<'&', tail...>> {
    using type = PushFront<
        typename STMTLexer<char_sequence<tail...>>::type,
        tokens::And>;
};

template <char... tail>
struct STMTLexer<char_sequence<'|', tail...>> {
    using type = PushFront<
        typename STMTLexer<char_sequence<tail...>>::type,
        tokens::Or>;
};

template <char... tail>
struct STMTLexer<char_sequence<'-', '>', tail...>> {
    using type = PushFront<
        typename STMTLexer<char_sequence<tail...>>::type,
        tokens::Implies>;
};

template <char... tail>
struct STMTLexer<char_sequence<'!', tail...>> {
    using type = PushFront<
        typename STMTLexer<char_sequence<tail...>>::type,
        tokens::Not>;
};

template <char... tail> 
struct STMTLexer<char_sequence<'&', '&', tail...>>
    : STMTLexer<char_sequence<'&', tail...>> {};

template <char... tail>
struct STMTLexer<char_sequence<'A', 'N', 'D', tail...>>
    : STMTLexer<char_sequence<'&', tail...>> {};

template <char... tail>
struct STMTLexer<char_sequence<'|', '|', tail...>>
    : STMTLexer<char_sequence<'|', tail...>> {};

template <char... tail>
struct STMTLexer<char_sequence<'O', 'R', tail...>>
    : STMTLexer<char_sequence<'|', tail...>> {};

template <char...> struct ParseID {
    using ID = std::integer_sequence<char>;
    using Tail = std::integer_sequence<char>;
};

template <typename, char> struct push_front;
template <char ch, char... chars>
struct push_front<char_sequence<chars...>, ch> {
    using type = char_sequence<ch, chars...>;
};

template <char first, char... tail>
struct ParseID<first, tail...> {
    using ID = std::conditional_t<
        is_alpha(first),
        typename push_front<typename ParseID<tail...>::ID, first>::type,
        char_sequence<>>;
    
    using Tail = std::conditional_t<
        is_alpha(first),
        typename ParseID<tail...>::Tail,
        char_sequence<first, tail...>>;
};

template <char first, char... tail>
struct STMTLexer<std::integer_sequence<char, first, tail...>,
                 std::enable_if_t<is_alpha(first)>> {
    using type = PushFront<
        typename STMTLexer<typename ParseID<first, tail...>::Tail>::type,
        tokens::Prop<typename ParseID<first, tail...>::ID>>;
};

template <typename> struct Statement;

template <typename CharT, CharT... chars>
struct Statement<std::integer_sequence<CharT, chars...>> {
    using type = typename STMTLexer<char_sequence<chars...>>::type;
};

template <typename CharT, CharT... name>
typename Statement<std::integer_sequence<CharT, name...>>::type
operator""_stmt()
{
    return {};
}
