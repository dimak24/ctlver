template <int ID_>
struct Node {
    constexpr const static int ID = ID_;
};

template <typename S, typename T>
struct Edge {
    using Source = S;
    using Target = T;
};

template <typename... Items>
using AdjList = DefaultDict<List<>, Items...>;

namespace impl {

template <typename Edges>
struct MakeAdjList {
    using type = AdjList<>;
};

template <typename E, typename... Es>
struct MakeAdjList<List<E, Es...>> {
    using SubList_ = typename MakeAdjList<List<Es...>>::type;
    using Neighbors_ = ::Get<SubList_, typename E::Source>;
    using Updated_ = ::Add<Neighbors_, typename E::Target>;
    using type = ::Update<SubList_, typename E::Source, Updated_>;
};

template <typename>
struct MakeEdgeList {
    using type = List<>;
};

template <typename Source, typename... Targets>
struct MakeEdgeList<AdjList<KV<Source, List<Targets...>>>> {
    using type = List<Edge<Source, Targets>...>;
};

template <typename Head, typename... Tail>
struct MakeEdgeList<AdjList<Head, Tail...>> {
    using type = typename Unite<
        typename MakeEdgeList<AdjList<Head>>::type,
        typename MakeEdgeList<AdjList<Tail...>>::type>::type;
};

} // namespace impl

template <typename, typename>
struct Graph;

template <typename Ns, typename... Es>
struct Graph<Ns, List<Es...>>  {
    using Nodes = Ns;
    using Edges = List<Es...>;
    using Adj = typename impl::MakeAdjList<Edges>::type;
};

template <typename Ns, typename... Items>
struct Graph<Ns, AdjList<Items...>> {
    using Nodes = Ns;
    using Adj = AdjList<Items...>;
    using Edges = typename impl::MakeEdgeList<Adj>::type;
};

template <typename>
struct ReverseAll_;

template <typename... Edges>
struct ReverseAll_<List<Edges...>> {
    using type = List<Edge<typename Edges::Target, typename Edges::Source>...>;
};

template <typename G>
using Inverse = Graph<typename G::Nodes, typename ReverseAll_<typename G::Edges>::type>;

namespace impl {

template <typename, typename, typename = List<>, typename = void>
struct DFS;

template <typename G, typename Visited>
struct DFS<G, List<>, Visited> {
    using VisitOrder = List<>;
};

template <typename G, typename Start, typename Visited>
struct DFS<G, List<Start>, Visited, std::enable_if_t<Contains<Visited, Start>::value>> {
    using VisitOrder = List<>;
};

template <typename G, typename Visited, typename Start>
struct DFS<G, List<Start>, Visited, std::enable_if_t<!Contains<Visited, Start>::value>> {
    using Next_ = typename Get<typename G::Adj, Start>::type;
    using Visited_ = typename Add<Visited, Start>::type;
    using VisitOrder =
        typename Add<typename DFS<G, Next_, Visited_>::VisitOrder, Start>::type;
};

template <typename G, typename Visited, typename Start, typename... Tail>
struct DFS<G, List<Start, Tail...>, Visited, std::enable_if_t<(sizeof...(Tail) > 0)>> {
    using Head_ = typename DFS<G, List<Start>, Visited>::VisitOrder;
    using Visited_ = typename Unite<Visited, Head_>::type;
    using VisitOrder = typename Unite<Head_, typename DFS<G, List<Tail...>, Visited_>::VisitOrder>::type;
};

template <typename, typename, typename = List<>, typename = void>
struct SCCKosarajuCollect;

template <typename G, typename Visited, typename Head>
struct SCCKosarajuCollect<G, List<Head>, Visited, std::enable_if_t<Contains<Visited, Head>::value>> {
    using type = List<>;
};

template <typename G, typename Visited, typename Head>
struct SCCKosarajuCollect<G, List<Head>, Visited, std::enable_if_t<!Contains<Visited, Head>::value>> {
    using Next_ = typename Get<typename G::Adj, Head>::type;
    using Visited_ = typename Add<Visited, Head>::type;
    using SCC_ =
        typename Add<typename DFS<G, Next_, Visited_>::VisitOrder, Head>::type;
    using type = List<SCC_>;
};

template <typename G, typename Visited, typename Head, typename... Tail>
struct SCCKosarajuCollect<G, List<Head, Tail...>, Visited, std::enable_if_t<(sizeof...(Tail) > 0)>> {
    using Head_ = typename SCCKosarajuCollect<G, List<Head>, Visited>::type;
    using Visited_ = typename Unite<Visited, typename Chain<Head_>::type>::type;
    using type = typename Unite<Head_, typename SCCKosarajuCollect<G, List<Tail...>, Visited_>::type>::type;
};

template <typename G, typename, typename = typename G::Nodes, typename = List<>, typename = void>
struct SCCKosaraju;

template <typename G, typename GInv, typename Visited, typename Head>
struct SCCKosaraju<G, GInv, List<Head>, Visited, std::enable_if_t<Contains<Visited, Head>::value>> {
    using type = List<>;
};

template <typename G, typename GInv, typename Visited, typename Head>
struct SCCKosaraju<G, GInv, List<Head>, Visited, std::enable_if_t<!Contains<Visited, Head>::value>> {
    using Sorted_ = typename DFS<GInv, List<Head>, Visited>::VisitOrder;
    using Visited_ = typename Unite<Visited, typename SetMinus<typename G::Nodes, Sorted_>::type>::type;
    using type = typename SCCKosarajuCollect<G, typename Reverse<Sorted_>::type, Visited_>::type;
};

template <typename G, typename GInv, typename Visited, typename Head, typename... Tail>
struct SCCKosaraju<G, GInv, List<Head, Tail...>, Visited, std::enable_if_t<(sizeof...(Tail) > 0)>> {
    using SCCsHead_ = typename SCCKosaraju<G, GInv, List<Head>, Visited>::type;
    using Visited_ = typename Unite<Visited, typename Chain<SCCsHead_>::type>::type;
    using type = typename Unite<SCCsHead_, typename SCCKosaraju<G, GInv, List<Tail...>, Visited_>::type>::type;
};



} // namespace impl

template <typename G, typename Start>
using DFS = typename impl::DFS<G, List<Start>>::VisitOrder;

template <typename G, typename Nodes>
using Reachable = typename impl::DFS<G, Nodes>::VisitOrder;

template <typename G>
using SCCKosaraju = typename impl::SCCKosaraju<G, Inverse<G>>::type;
