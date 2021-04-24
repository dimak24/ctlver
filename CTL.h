#include "logic.h"

template <typename F>
struct EG {
    using Arg = F;
};

// E (L U R)
template <typename L, typename R>
struct EU {
    using Lhs = L;
    using Rhs = R;
};

template <typename F>
struct EX {
    using Arg = F;
};
