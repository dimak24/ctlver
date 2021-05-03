#include <functional>

#include "CTL.h"
#include "list.h"

template <typename...>
using always_false = std::false_type;

template <char... chars>
using char_sequence = std::integer_sequence<char, chars...>;

namespace detail {

namespace tokens {

struct LeftParen {};
struct RightParen {};

template <typename Name>
using Prop = Prop<Name>;

struct Not;
struct And;
struct Or;
struct Implies;

// TODO: inherit from LTL {E, A} x {X, F, G}; give better names
struct E;
struct A;
struct U;

struct AX;
struct EX;
struct AF;
struct EF;
struct AG;
struct EG;

} // namespace tokens

constexpr bool is_space(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r';
}

constexpr bool is_alpha(char ch)
{
    return 'a' <= ch && ch <= 'z';
}

constexpr bool is_num(char ch)
{
    return '0' <= ch && ch <= '9';
}

template <typename, typename = void> struct CTLLexer {};

template <>
struct CTLLexer<char_sequence<>> {
    using type = List<>;
};

template <char first, char... tail>
struct CTLLexer<char_sequence<first, tail...>,
                 std::enable_if_t<is_space(first)>> {
    using type = typename CTLLexer<char_sequence<tail...>>::type;
};

template <char... tail>
struct CTLLexer<char_sequence<'(', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::LeftParen>;
};

template <char... tail>
struct CTLLexer<char_sequence<')', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::RightParen>;
};

template <char... tail>
struct CTLLexer<char_sequence<'&', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::And>;
};

template <char... tail>
struct CTLLexer<char_sequence<'|', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::Or>;
};

template <char... tail>
struct CTLLexer<char_sequence<'-', '>', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::Implies>;
};

template <char... tail>
struct CTLLexer<char_sequence<'!', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::Not>;
};

template <char... tail>
struct CTLLexer<char_sequence<'A', 'X', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::AX>;
};

template <char... tail>
struct CTLLexer<char_sequence<'E', 'X', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::EX>;
};

template <char... tail>
struct CTLLexer<char_sequence<'A', 'F', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::AF>;
};

template <char... tail>
struct CTLLexer<char_sequence<'E', 'F', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::EF>;
};

template <char... tail>
struct CTLLexer<char_sequence<'A', 'G', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::AG>;
};

template <char... tail>
struct CTLLexer<char_sequence<'E', 'G', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::EG>;
};

template <char... tail>
struct CTLLexer<char_sequence<'E', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::E>;
};

template <char... tail>
struct CTLLexer<char_sequence<'A', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::A>;
};

template <char... tail>
struct CTLLexer<char_sequence<'U', tail...>> {
    using type = PushFront<
        typename CTLLexer<char_sequence<tail...>>::type,
        tokens::U>;
};

template <char... tail>
struct CTLLexer<char_sequence<'&', '&', tail...>>
    : CTLLexer<char_sequence<'&', tail...>> {};

template <char... tail>
struct CTLLexer<char_sequence<'A', 'N', 'D', tail...>>
    : CTLLexer<char_sequence<'&', tail...>> {};

template <char... tail>
struct CTLLexer<char_sequence<'|', '|', tail...>>
    : CTLLexer<char_sequence<'|', tail...>> {};

template <char... tail>
struct CTLLexer<char_sequence<'O', 'R', tail...>>
    : CTLLexer<char_sequence<'|', tail...>> {};

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
        is_alpha(first) || is_num(first),
        typename push_front<typename ParseID<tail...>::ID, first>::type,
        char_sequence<>>;

    using Tail = std::conditional_t<
        is_alpha(first) || is_num(first),
        typename ParseID<tail...>::Tail,
        char_sequence<first, tail...>>;
};

template <char first, char... tail>
struct CTLLexer<std::integer_sequence<char, first, tail...>,
                 std::enable_if_t<is_alpha(first) || is_num(first)>> {
    using type = PushFront<
        typename CTLLexer<typename ParseID<first, tail...>::Tail>::type,
        tokens::Prop<typename ParseID<first, tail...>::ID>>;
};

template <typename, typename = void> struct ParseClause;
template <typename, typename = void> struct ParseExpression;
template <typename, typename = void> struct ParseConjunction;
template <typename, typename = void> struct ParseDisjunction;
template <typename, typename = void> struct ParseImplication;
template <typename, typename = void> struct ParseUntil;

template <typename... Tokens>
struct ParseConjunction<
    List<Tokens...>,
    std::enable_if_t<
        std::is_same_v<
            tokens::And,
            typename ParseClause<List<Tokens...>>::remain::Head
        >>> {
    using remain_ = typename ParseClause<List<Tokens...>>::remain;
    using result = And<
        typename ParseClause<List<Tokens...>>::result,
        typename ParseClause<typename remain_::Tail>::result>;
    using remain = typename ParseClause<typename remain_::Tail>::remain;
};

template <typename... Tokens>
struct ParseDisjunction<
    List<Tokens...>,
    std::enable_if_t<
        std::is_same_v<
            tokens::Or,
            typename ParseClause<List<Tokens...>>::remain::Head
        >>> {
    using remain_ = typename ParseClause<List<Tokens...>>::remain;
    using result = Or<
        typename ParseClause<List<Tokens...>>::result,
        typename ParseClause<typename remain_::Tail>::result>;
    using remain = typename ParseClause<typename remain_::Tail>::remain;
};

template <typename... Tokens>
struct ParseImplication<
    List<Tokens...>,
    std::enable_if_t<
        std::is_same_v<
            tokens::Implies,
            typename ParseClause<List<Tokens...>>::remain::Head
        >>> {
    using remain_ = typename ParseClause<List<Tokens...>>::remain;
    using result = Implies<
        typename ParseClause<List<Tokens...>>::result,
        typename ParseClause<typename remain_::Tail>::result>;
    using remain = typename ParseClause<typename remain_::Tail>::remain;
};

template <typename... Tokens>
struct ParseUntil<
    List<Tokens...>,
    std::enable_if_t<
        std::is_same_v<
            tokens::U,
            typename ParseClause<List<Tokens...>>::remain::Head
        >>> {
    using remain_ = typename ParseClause<List<Tokens...>>::remain;
    using lhs = typename ParseClause<List<Tokens...>>::result;
    using rhs = typename ParseClause<typename remain_::Tail>::result;
    using remain = typename ParseClause<typename remain_::Tail>::remain;
};

template <typename Tokens>
struct ParseExpression<
    Tokens, std::void_t<typename ParseDisjunction<Tokens>::result>> {
    using result = typename ParseDisjunction<Tokens>::result;
    using remain = typename ParseDisjunction<Tokens>::remain;
};

template <typename Tokens>
struct ParseExpression<
    Tokens, std::void_t<typename ParseConjunction<Tokens>::result>> {
    using result = typename ParseConjunction<Tokens>::result;
    using remain = typename ParseConjunction<Tokens>::remain;
};

template <typename Tokens>
struct ParseExpression<
    Tokens, std::void_t<typename ParseImplication<Tokens>::result>> {
    using result = typename ParseImplication<Tokens>::result;
    using remain = typename ParseImplication<Tokens>::remain;
};

template <typename Name, typename... Tokens>
struct ParseClause<List<tokens::Prop<Name>, Tokens...>> {
    using result = Prop<Name>;
    using remain = List<Tokens...>;
};

template <typename... Tokens>
struct ParseClause<List<tokens::Not, Tokens...>> {
    using result = Not<typename ParseClause<List<Tokens...>>::result>;
    using remain = typename ParseClause<List<Tokens...>>::remain;
};

template <typename... Tokens>
struct ParseClause<List<tokens::AX, Tokens...>> {
    using result = AX<typename ParseClause<List<Tokens...>>::result>;
    using remain = typename ParseClause<List<Tokens...>>::remain;
};

template <typename... Tokens>
struct ParseClause<List<tokens::EX, Tokens...>> {
    using result = EX<typename ParseClause<List<Tokens...>>::result>;
    using remain = typename ParseClause<List<Tokens...>>::remain;
};

template <typename... Tokens>
struct ParseClause<List<tokens::AF, Tokens...>> {
    using result = AF<typename ParseClause<List<Tokens...>>::result>;
    using remain = typename ParseClause<List<Tokens...>>::remain;
};

template <typename... Tokens>
struct ParseClause<List<tokens::EF, Tokens...>> {
    using result = EF<typename ParseClause<List<Tokens...>>::result>;
    using remain = typename ParseClause<List<Tokens...>>::remain;
};

template <typename... Tokens>
struct ParseClause<List<tokens::AG, Tokens...>> {
    using result = AG<typename ParseClause<List<Tokens...>>::result>;
    using remain = typename ParseClause<List<Tokens...>>::remain;
};

template <typename... Tokens>
struct ParseClause<List<tokens::EG, Tokens...>> {
    using result = EG<typename ParseClause<List<Tokens...>>::result>;
    using remain = typename ParseClause<List<Tokens...>>::remain;
};

template <typename... Tokens>
struct ParseClause<
    List<tokens::E, tokens::LeftParen, Tokens...>,
    std::enable_if_t<
        std::is_same_v<
            tokens::RightParen,
            typename ParseUntil<List<Tokens...>>::remain::Head
        >>> {
    using lhs_ = typename ParseUntil<List<Tokens...>>::lhs;
    using rhs_ = typename ParseUntil<List<Tokens...>>::rhs;
    using result = EU<lhs_, rhs_>;
    using remain = typename ParseUntil<List<Tokens...>>::remain;
};

template <typename... Tokens>
struct ParseClause<
    List<tokens::A, tokens::LeftParen, Tokens...>,
    std::enable_if_t<
        std::is_same_v<
            tokens::RightParen,
            typename ParseUntil<List<Tokens...>>::remain::Head
        >>> {
    using lhs_ = typename ParseUntil<List<Tokens...>>::lhs;
    using rhs_ = typename ParseUntil<List<Tokens...>>::rhs;
    using result = AU<lhs_, rhs_>;
    using remain = typename ParseUntil<List<Tokens...>>::remain;
};

template <typename... Tokens>
struct ParseClause<
    List<tokens::LeftParen, Tokens...>,
    std::void_t<typename ParseExpression<List<Tokens...>>::result>> {
    using result = typename ParseExpression<List<Tokens...>>::result;
    using remain = typename ParseExpression<List<Tokens...>>::remain::Tail;
};

template <typename... Tokens>
struct ParseClause<
    List<tokens::LeftParen, Tokens...>,
    std::enable_if_t<
        std::is_same_v<
            tokens::RightParen,
            typename ParseClause<List<Tokens...>>::remain::Head>>> {
    using result = typename ParseClause<List<Tokens...>>::result;
    using remain = typename ParseClause<List<Tokens...>>::remain::Tail;
};


template <typename> struct CTLParser;
template <typename... Tokens>
struct CTLParser<List<Tokens...>> {
    using type = typename ParseClause<
        List<tokens::LeftParen, Tokens..., tokens::RightParen>>::result;
};

} // namespace detail

template <typename CharT, CharT... chars>
typename detail::CTLParser<
    typename detail::CTLLexer<char_sequence<chars...>>::type>::type
operator""_CTL()
{
    return {};
}
