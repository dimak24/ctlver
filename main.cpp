#include <functional>
#include <iostream>

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

    {
        using Init1 = decltype("init1"_prop);
        using Init2 = decltype("init2"_prop);
        using Try1 = decltype("try1"_prop);
        using Try2 = decltype("try2"_prop);
        using Crit1 = decltype("crit1"_prop);
        using Crit2 = decltype("crit2"_prop);

        using model = KripkeModel<
            List<
                Node<11>, Node<12>,
                Node<21>, Node<22>, Node<23>, Node<24>,
                Node<31>, Node<32>, Node<33>, Node<34>,
                Node<41>, Node<42>, Node<43>, Node<44>,
                Node<51>, Node<52>, Node<53>, Node<54>, Node<55>, Node<56>,
                Node<61>, Node<62>, Node<63>, Node<64>,
                Node<71>, Node<72>
            >,
            List<Node<11>, Node<12>>,
            List<
                Edge<Node<11>, Node<21>>,
                Edge<Node<11>, Node<22>>,
                Edge<Node<12>, Node<23>>,
                Edge<Node<12>, Node<24>>,

                Edge<Node<21>, Node<31>>,
                Edge<Node<21>, Node<32>>,
                Edge<Node<22>, Node<31>>,
                Edge<Node<22>, Node<33>>,
                Edge<Node<23>, Node<32>>,
                Edge<Node<23>, Node<34>>,
                Edge<Node<24>, Node<33>>,
                Edge<Node<24>, Node<34>>,

                Edge<Node<31>, Node<42>>,
                Edge<Node<31>, Node<43>>,
                Edge<Node<32>, Node<41>>,
                Edge<Node<32>, Node<42>>,
                Edge<Node<33>, Node<43>>,
                Edge<Node<33>, Node<44>>,
                Edge<Node<34>, Node<42>>,
                Edge<Node<34>, Node<43>>,

                Edge<Node<41>, Node<51>>,
                Edge<Node<41>, Node<52>>,
                Edge<Node<42>, Node<53>>,
                Edge<Node<43>, Node<54>>,
                Edge<Node<44>, Node<55>>,
                Edge<Node<44>, Node<56>>,

                Edge<Node<51>, Node<61>>,
                Edge<Node<51>, Node<12>>,
                Edge<Node<52>, Node<61>>,
                Edge<Node<52>, Node<62>>,
                Edge<Node<53>, Node<62>>,
                Edge<Node<54>, Node<63>>,
                Edge<Node<55>, Node<64>>,
                Edge<Node<55>, Node<63>>,
                Edge<Node<56>, Node<64>>,
                Edge<Node<56>, Node<11>>,

                Edge<Node<61>, Node<71>>,
                Edge<Node<61>, Node<24>>,
                Edge<Node<62>, Node<71>>,
                Edge<Node<63>, Node<72>>,
                Edge<Node<64>, Node<72>>,
                Edge<Node<64>, Node<21>>,

                Edge<Node<71>, Node<33>>,
                Edge<Node<72>, Node<32>>
            >,
            DefaultDict<List<>,
                KV<Node<11>, List<Init1, Init2>>,
                KV<Node<12>, List<Init1, Init2>>,
                KV<Node<21>, List<Init2>>,
                KV<Node<22>, List<Init1>>,
                KV<Node<23>, List<Init2>>,
                KV<Node<24>, List<Init1>>,
                KV<Node<32>, List<Init2, Try1>>,
                KV<Node<33>, List<Init1, Try2>>,
                KV<Node<41>, List<Crit1, Init2>>,
                KV<Node<44>, List<Crit2, Init1>>,
                KV<Node<42>, List<Try1>>,
                KV<Node<43>, List<Try2>>,
                KV<Node<51>, List<Init2>>,
                KV<Node<56>, List<Init1>>,
                KV<Node<52>, List<Crit1>>,
                KV<Node<53>, List<Try1, Try2>>,
                KV<Node<54>, List<Try1, Try2>>,
                KV<Node<55>, List<Crit2>>,
                KV<Node<62>, List<Try2, Crit1>>,
                KV<Node<63>, List<Try1, Crit2>>,
                KV<Node<71>, List<Try2>>,
                KV<Node<72>, List<Try1>>
            >
        >;

        using mutual_exclusion = decltype("AG(!(crit1 && crit2))"_CTL);
        using delay_absence = decltype("AG((try1 && init2) -> AX((!try2 -> crit1) || AX(!try2 -> crit1)))"_CTL);
        using eventual_entry = decltype("try1 -> AF(crit1)"_CTL);

        static_assert(
            std::is_same_v<List<>,
                SetMinus<
                    typename model::Nodes,
                    typename CTLCheck<model, mutual_exclusion>::Satisfy>>);

        static_assert(
            std::is_same_v<List<>,
                SetMinus<
                    typename model::Nodes,
                    typename CTLCheck<model, delay_absence>::Satisfy>>);

        static_assert(
            std::is_same_v<List<>,
                SetMinus<
                    typename model::Nodes,
                    typename CTLCheck<model, eventual_entry>::Satisfy>>);
    }
}
