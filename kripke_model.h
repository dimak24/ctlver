template <typename S, typename S0, typename R, typename L>
struct KripkeModel {
    using States = S;
    using InitialStates = S0;
    using Relation = R;
    using Labels = L;
};
