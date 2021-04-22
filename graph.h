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

}

template <typename Ns, typename Es>
struct Graph {
    using Nodes = Ns;
    using Edges = Es;
    using AdjList = typename impl::MakeAdjList<Es>::type;
};
