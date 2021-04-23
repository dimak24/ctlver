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

namespace impl {

template <typename, typename, typename Visited = List<>, typename = void>
struct Reachable;

template <typename G, typename Visited>
struct Reachable<G, List<>, Visited> {
    using type = List<>;
};

template <typename G, typename Node, typename Visited>
struct Reachable<G, List<Node>, Visited, std::enable_if_t<Contains<Visited, Node>::value>> {
    using type = List<>;
};

template <typename G, typename Node, typename Visited>
struct Reachable<G, List<Node>, Visited, std::enable_if_t<!Contains<Visited, Node>::value>> {
    using Next_ = typename Get<typename G::Adj, Node>::type;
    using VisitedWithNode_ = typename Add<Visited, Node>::type;
    using type =
        typename Add<typename Reachable<G, Next_, VisitedWithNode_>::type, Node>::type;
};

template <typename G, typename Visited, typename Node, typename... Tail>
struct Reachable<G, List<Node, Tail...>, Visited, std::enable_if_t<(sizeof...(Tail) > 0)>> {
    using Reachable_ = typename Reachable<G, List<Node>, Visited>::type;
    using Visited_ = typename Unite<Visited, Reachable_>::type;
    using type = typename Unite<Reachable_, typename Reachable<G, List<Tail...>, Visited_>::type>::type;
};

template <typename> struct SCCKosaraju;

} // namespace impl

template <typename G, typename Nodes>
using Reachable = typename impl::Reachable<G, Nodes>::type;

template <typename G>
using SCCKosaraju = List<>;
