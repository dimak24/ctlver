#include "CTL.h"

template <typename Model, typename CTLFormula>
struct CTLCheck {
    using Satisfy = typename Model::States;
};

template <typename Model>
struct CTLCheck<Model, True<>> {
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

template <typename Model, typename A, typename B>
struct CTLCheck<Model, EU<A, B>> {
    using SatisfyA_ = typename CTLCheck<Model, A>::Satisfy;
    using SatisfyB_ = typename CTLCheck<Model, B>::Satisfy;
    using SatisfyAB_ = Unite<SatisfyA_, SatisfyB_>;

    template <typename Edge>
    struct Selector_
        : std::integral_constant<bool,
            Contains<SatisfyA_, typename Edge::Source>> {};

    using Graph_ = Graph<
        SatisfyAB_,
        typename ReverseAll_<
            Select<typename Model::Relation, Selector_>>::type>;

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

    using Graph_ = Graph<
        SatisfyF_,
        typename ReverseAll_<
            Select<typename Model::Relation, Selector_>>::type>;

    template <typename SCC>
    struct SCCSelector_
        : std::integral_constant<bool,
            (Len<SCC> > 1)
            || Contains<
                Get<
                    typename impl::MakeAdjList<typename Model::Relation>::type,
                    typename SCC::Head
                >, typename SCC::Head>> {};

    using SCCs_ = Select<SCCKosaraju<Graph_>, SCCSelector_>;
    using Satisfy = Reachable<Graph_, Chain<SCCs_>>;
};

