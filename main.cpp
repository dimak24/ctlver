#include <functional>

#include "kripke_model.h"
#include "CTL_parse.h"
#include "CTL_check.h"

template <typename>
struct ShowType {
    explicit ShowType() = delete;
};

int main() {
    {
        using Start = decltype("start"_prop);
        using Error = decltype("error"_prop);
        using Close = decltype("close"_prop);
        using Heat = decltype("heat"_prop);

        using model = KripkeModel<
            List<Node<0>, Node<1>, Node<2>, Node<3>, Node<4>, Node<5>, Node<6>>,
            List<>,
            List<
                Edge<Node<0>, Node<1>>,
                Edge<Node<0>, Node<2>>,
                Edge<Node<1>, Node<4>>,
                Edge<Node<4>, Node<1>>,
                Edge<Node<4>, Node<2>>,
                Edge<Node<2>, Node<0>>,
                Edge<Node<3>, Node<2>>,
                Edge<Node<3>, Node<0>>,
                Edge<Node<3>, Node<3>>,
                Edge<Node<6>, Node<3>>,
                Edge<Node<2>, Node<5>>,
                Edge<Node<5>, Node<6>>>,
            DefaultDict<List<>,
                KV<Node<1>, List<Start, Error>>,
                KV<Node<2>, List<Close>>,
                KV<Node<3>, List<Close, Heat>>,
                KV<Node<4>, List<Start, Close, Error>>,
                KV<Node<5>, List<Start, Close>>,
                KV<Node<6>, List<Start, Close, Heat>>>>;

        using formula = decltype("AG(start -> AF(heat))"_CTL);
        static_assert(std::is_same_v<List<>, typename CTLCheck<model, formula>::Satisfy>);
    }

    {
        using p = decltype("p"_prop);
        using q = decltype("q"_prop);

        using model = KripkeModel<
            List<Node<0>, Node<1>, Node<2>, Node<3>>,
            List<>,
            List<
                Edge<Node<0>, Node<1>>,
                Edge<Node<1>, Node<2>>,
                Edge<Node<2>, Node<2>>,
                Edge<Node<1>, Node<3>>,
                Edge<Node<3>, Node<3>>>,
            DefaultDict<List<>,
                KV<Node<0>, List<p, q>>,
                KV<Node<1>, List<q>>,
                KV<Node<2>, List<p>>,
                KV<Node<3>, List<q>>>>;

        using formula = decltype("EG((p -> q) && EF(q && !p))"_CTL);
        static_assert(
            std::is_same_v<
                List<Node<0>, Node<1>, Node<3>>,
                typename CTLCheck<model, formula>::Satisfy>);
    }
}
