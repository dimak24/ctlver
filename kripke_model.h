#pragma once

#include "graph.h"

template <typename S, typename S0, typename R, typename L>
struct KripkeModel : Graph<S, R> {
    using States = S;
    using InitialStates = S0;
    using Relation = R;
    using Labels = L;
};
