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

template <typename L, typename R>
struct Or {
    using Lhs = L;
    using Rhs = R;
};

template <typename...>
struct True {};

template <typename F>
struct Not {
    using Arg = F;
};
