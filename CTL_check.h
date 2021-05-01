#include "CTL.h"

namespace impl_ {

template <typename, typename = void>
struct CTLNormalize;

template <typename Name>
struct CTLNormalize<Prop<Name>> {
    using F = Prop<Name>;
};

template <> struct CTLNormalize<True> {
    using F = True;
};

template <template <typename, typename> class Allowed, typename L, typename R>
struct CTLNormalize<
        Allowed<L, R>,
        std::enable_if_t<
            std::is_same_v<Allowed<L, R>, Or<L, R>>
            || std::is_same_v<Allowed<L, R>, EU<L, R>>>> {
    using L_ = typename CTLNormalize<L>::F;
    using R_ = typename CTLNormalize<R>::F;
    using F = Allowed<L_, R_>;
};

template <template <typename> class Allowed, typename F_>
struct CTLNormalize<
        Allowed<F_>,
        std::enable_if_t<
            std::is_same_v<Allowed<F_>, Not<F_>>
            || std::is_same_v<Allowed<F_>, EX<F_>>
            || std::is_same_v<Allowed<F_>, EG<F_>>>> {
    using F = Allowed<typename CTLNormalize<F_>::F>;
};

template <> struct CTLNormalize<False> {
    using F = Not<True>;
};

template <typename L, typename R>
struct CTLNormalize<And<L, R>> {
    using F = Not<Or<
        Not<typename CTLNormalize<L>::F>,
        Not<typename CTLNormalize<R>::F>>>;
};

template <typename L, typename R>
struct CTLNormalize<Implies<L, R>> {
    using F = Or<
        Not<typename CTLNormalize<L>::F>,
        typename CTLNormalize<R>::F>;
};

template <typename F_>
struct CTLNormalize<AX<F_>> {
    using F = Not<EX<Not<typename CTLNormalize<F_>::F>>>;
};

template <typename F_>
struct CTLNormalize<EF<F_>> {
    using F = EU<True, typename CTLNormalize<F_>::F>;
};

template <typename F_>
struct CTLNormalize<AG<F_>> {
    using F = Not<EU<True, Not<typename CTLNormalize<F_>::F>>>;
};

template <typename F_>
struct CTLNormalize<AF<F_>> {
    using F = Not<EG<Not<typename CTLNormalize<F_>::F>>>;
};

template <typename L, typename R>
struct CTLNormalize<AU<L, R>> {
    using L_ = typename CTLNormalize<L>::F;
    using R_ = typename CTLNormalize<R>::F;
    using F = Not<Or<EU<Not<R_>, Not<Or<L_, R_>>>, EG<Not<R_>>>>;
};

template <typename Model, typename CTLFormula>
struct CTLCheck {
    using Satisfy = typename Model::States;
};

template <typename Model>
struct CTLCheck<Model, True> {
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

template <typename> struct Sources_;
template <typename... Edges>
struct Sources_<List<Edges...>> {
    using type = List<typename Edges::Source...>;
};

template <typename Model, typename F>
struct CTLCheck<Model, EX<F>> {
    using FSatisfy_ = typename CTLCheck<Model, F>::Satisfy;

    template <typename Edge>
    struct Selector_ : impl::Contains<FSatisfy_, typename Edge::Target> {};

    using Satisfy = typename Sources_<
        Select<typename Model::Relation, Selector_>>::type;
};

template <typename Model, typename A, typename B>
struct CTLCheck<Model, EU<A, B>> {
    using SatisfyA_ = typename CTLCheck<Model, A>::Satisfy;
    using SatisfyB_ = typename CTLCheck<Model, B>::Satisfy;
    using SatisfyAB_ = Unite<SatisfyA_, SatisfyB_>;

    template <typename Edge>
    struct Selector_ : impl::Contains<SatisfyA_, typename Edge::Source> {};

    using Graph_ = Inverse<Graph<
        SatisfyAB_, Select<typename Model::Relation, Selector_>>>;

    using Satisfy = Reachable<Graph_, SatisfyB_>;
};

template <typename Model, typename F>
struct CTLCheck<Model, EG<F>> {
    using SatisfyF_ = typename CTLCheck<Model, F>::Satisfy;

    template <typename Edge>
    struct Selector_
        : std::integral_constant<bool,
            Contains<SatisfyF_, typename Edge::Source>
            && Contains<SatisfyF_, typename Edge::Target>> {};

    using Graph_ = Inverse<Graph<
        SatisfyF_, Select<typename Model::Relation, Selector_>>>;

    template <typename SCC>
    struct SCCSelector_
        : std::integral_constant<bool,
            (Len<SCC> > 1)
            || Contains<Get<typename Model::Adj, typename SCC::Head>,
                        typename SCC::Head>> {};

    using SCCs_ = Select<SCCKosaraju<Graph_>, SCCSelector_>;
    using Satisfy = Reachable<Graph_, Chain<SCCs_>>;
};

} // namespace impl_

template <typename Model, typename Formula>
using CTLCheck = impl_::CTLCheck<
    Model, typename impl_::CTLNormalize<Formula>::F>;
