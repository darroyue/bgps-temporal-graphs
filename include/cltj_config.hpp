#ifndef CLTJ_CONFIG_H
#define CLTJ_CONFIG_H

#include <iostream>
#include<vector>

namespace cltj{
    using namespace std;

    typedef std::array<uint32_t, 3> spo_triple;
    typedef uint8_t spo_order_type[3];
    typedef spo_order_type spo_orders_type[6];
    const static spo_orders_type spo_orders = {{0, 1, 2}, //SPO
                                              {0, 2, 1}, //SOP
                                              {1, 2, 0}, //POS
                                              {1, 0, 2}, //PSO
                                              {2, 0, 1}, //OSP
                                              {2, 1, 0}}; //OPS

    typedef uint32_t tuple_value_type;
    typedef std::array<uint32_t, 5> spot_quad;    // temporal intervals treated as two integers
    typedef uint8_t spot_order_type[5];
    typedef spot_order_type spot_orders_type[18];

    typedef uint32_t temporal_endpoint_type;
    typedef std::pair<temporal_endpoint_type, temporal_endpoint_type> temporal_interval; 

    #define SPOT  0
    #define SPT   1
    #define SO    2
    #define ST    3
    #define POS   4
    #define POT   5
    #define PS    6
    #define PT    7
    #define OSP   8
    #define OST   9
    #define OP   10
    #define OT   11
    #define TSPO 12    
    #define TSO  13
    #define TPOS 14
    #define TPS  15
    #define TOSP 16
    #define TOP  17
    
    // The following array is indexed according to the above indexes for each possible order of S, P, O, T.
    // For each order, the corresponding array entry indicates the order in which to consider the components to achieve the desired order

    const static spot_orders_type spot_orders = {   {0, 1, 2, 3, 4}, //SPOT
                                                    {0, 1, 3, /**/2, 4/**/}, //SPT
                                                    {0, 2, /**/ 1, 3, 4/**/}, //SO
                                                    {0, 3, /**/1, 2, 4/**/}, //ST
                                                    {1, 2, 0, /**/3, 4/**/}, //POS
                                                    {1, 2, 3, /**/0, 4/**/}, //POT
                                                    {1, 0, /**/2, 3, 4/**/}, //PS
                                                    {1, 3, /**/0, 2, 4/**/}, //PT
                                                    {2, 0, 1, /**/3, 4/**/}, //OSP
                                                    {2, 0, 3, /**/1, 4/**/}, //OST
                                                    {2, 1, /**/0, 3, 4/**/}, //OP
                                                    {2, 3, /**/0, 1, 4/**/}, //OT
                                                    {3, 0, 1, 2, 4},  //TSPO 
                                                    {3, 0, 2, /**/1, 4/**/}, //TSO
                                                    {3, 1, 2, 0, 4}, //TPOS
                                                    {3, 1, 0, /**/2, 4/**/}, //TPS
                                                    {3, 2, 0, 1, 4}, //TOSP
                                                    {3, 2, 1, /**/0, 4/**/} //TOP
                                                };


    struct comparator_order {
            uint64_t i;
            comparator_order(uint64_t pi) {
                i = pi;
            };
            inline bool operator()(const spot_quad& t1, const spot_quad& t2) {
                if(t1[spot_orders[i][0]] == t2[spot_orders[i][0]]) {
                    if(t1[spot_orders[i][1]] == t2[spot_orders[i][1]]) {
                        if (t1[spot_orders[i][2]] == t2[spot_orders[i][2]])
                        return t1[spot_orders[i][3]] < t2[spot_orders[i][3]]; // Intervals are compared only by their initial endpoint
                        else
                return t1[spot_orders[i][2]] < t2[spot_orders[i][2]];
                    }
                    return t1[spot_orders[i][1]] < t2[spot_orders[i][1]];
                }
                return t1[spot_orders[i][0]] < t2[spot_orders[i][0]];
            }
        };

    // comparator for sorting arrays of "pointers" to tuples
    struct comparator_order_idx {
        const std::vector<spot_quad>& data;
        uint64_t i;                          

        comparator_order_idx(const std::vector<spot_quad>& d, uint64_t pi)
            : data(d), i(pi) {}

        inline bool operator()(size_t a, size_t b) const {
            const spot_quad& t1 = data[a];
            const spot_quad& t2 = data[b];

            if (t1[spot_orders[i][0]] == t2[spot_orders[i][0]]) {
                if (t1[spot_orders[i][1]] == t2[spot_orders[i][1]]) {
                    if (t1[spot_orders[i][2]] == t2[spot_orders[i][2]])
                        return t1[spot_orders[i][3]] < t2[spot_orders[i][3]];
                    else
                        return t1[spot_orders[i][2]] < t2[spot_orders[i][2]];
                }
                return t1[spot_orders[i][1]] < t2[spot_orders[i][1]];
            }
            return t1[spot_orders[i][0]] < t2[spot_orders[i][0]];
        }
    };


    struct trie_status_pair {
        int8_t m_trie;
        bool m_switch;
        uint8_t m_status;
        trie_status_pair() = default;
        trie_status_pair(int8_t trie, bool swt, uint8_t status) : m_trie(trie), m_switch(swt), m_status(status) {}
    };

    typedef trie_status_pair switch_type[4];
    typedef switch_type switches_type[18];

    // s=0, p=1, o=2, t=3 
    // tables for trie switching
    const static switches_type trie_map_leap_2 = {
                                            {{-1, false, 0}, {-1, false, 0}, {SPOT, false, 0}, {SPT, false, 1}}, //SPOT
                                            {{-1, false, 0}, {-1, false, 0}, {SPOT, false, 0}, {SPT, false, 1}}, //SPT
                                            {{-1, false, 0}, {OSP, true, 0}, {-1, false, 0}, {OST, true, 0}}, //SO
                                            {{-1, false, 0}, {TSPO, true, 0}, {TSO, true, 0}, {-1, false, 0}}, //ST
                                            {{POS, false, 0}, {-1, false, 0}, {-1, false, 0}, {POT, false, 1}}, //POS
                                            {{POS, true, 0}, {-1, false, 0}, {-1, false, 0}, {POT, false, 0}}, //POT
                                            {{-1, false, 0}, {-1, false, 0}, {SPOT, true, 0}, {SPT, true, 0}}, //PS
                                            {{TPS, true, 0}, {-1, false, 0}, {TPOS, true, 0}, {-1, false, 0}}, //PT
                                            {{-1, false, 0}, {OSP, false, 0}, {-1, false, 0}, {OST, false, 1}}, //OSP
                                            {{-1, false, 0}, {OSP, false, 0}, {-1, false, 0}, {OST, false, 1}}, //OST
                                            {{POS, true, 0}, {-1, false, 0}, {-1, false, 0}, {POT, true, 0}}, //OP
                                            {{TOSP, true, 0}, {TOP, true, 0}, {-1, false, 0}, {-1, false, 0}}, //OT
                                            {{-1, false, 0}, {TSPO, false, 0}, {TSO, false, 1}, {-1, false, 0}},  //TSPO
                                            {{-1, false, 0}, {TSPO, false, 0}, {TSO, false, 1}, {-1, false, 0}}, //TSO
                                            {{TPS, false, 1}, {-1, false, 0}, {TPOS, false, 0}, {-1, false, 0}}, //TPOS
                                            {{TPS, false, 1}, {-1, false, 0}, {TPOS, false, 0}, {-1, false, 0}}, //TPS
                                            {{TOSP, false, 0}, {TOP, false, 1}, {-1, false, 0}, {-1, false, 0}}, //TOSP
                                            {{TOSP, false, 0}, {TOP, false, 1}, {-1, false, 0}, {-1, false, 0}} //TOP
                                            };


    const static switches_type trie_map_2 = {{{-1, false, 0}, {-1, false, 0}, {SPOT, false, 0}, {TSPO, true, 0}}, //SPOT
                                            {{-1, false, 0}, {-1, false, 0}, {SPOT, false, 0}, {TSPO, true, 0}}, //SPT, invalid configuration
                                            {{-1, false, 0}, {/*OSP*/SPOT, true, 0}, {-1, false, 0}, {/*OST*/TOSP, true, 0}}, //SO
                                            {{-1, false, 0}, {TSPO, true, 0}, {TSO, true, 0}, {-1, false, 0}}, //ST
                                            {{SPOT, true, 0}, {-1, false, 0}, {-1, false, 0}, {TPOS, true, 0}}, //POS
                                            {{POS, true, 0}, {-1, false, 0}, {-1, false, 0}, {TPOS, true, 0}}, //POT, invalid configuration
                                            {{-1, false, 0}, {-1, false, 0}, {SPOT, false, 0}, {TSPO, true, 0}}, //PS
                                            {{TSPO, true, 0}, {-1, false, 0}, {TPOS, false, 0}, {-1, false, 0}}, //PT
                                            {{-1, false, 0}, {SPOT, true, 0}, {-1, false, 0}, {TOSP, true, 0}}, //OSP
                                            {{-1, false, 0}, {SPOT, true, 0}, {-1, false, 0}, {TOSP, true, 0}}, //OST
                                            {{SPOT, true, 0}, {-1, false, 0}, {-1, false, 0}, {TPOS, true, 0}}, //OP
                                            {{TOSP, false, 0}, {TPOS, true, 0}, {-1, false, 0}, {-1, false, 0}}, //OT
                                            {{-1, false, 0}, {TSPO, false, 0}, {TOSP, true, 0}, {-1, false, 0}},  //TSPO
                                            {{-1, false, 0}, {TSPO, false, 0}, {TOSP, true, 0}, {-1, false, 0}}, //TSO, invalid configuration
                                            {{TSPO, true, 0}, {-1, false, 0}, {TPOS, false, 0}, {-1, false, 0}}, //TPOS
                                            {{TSPO, true, 0}, {-1, false, 0}, {TPOS, false, 0}, {-1, false, 0}}, //TPS, invalid configuration
                                            {{TOSP, false, 0}, {TPOS, true, 0}, {-1, false, 0}, {-1, false, 0}}, //TOSP
                                            {{TOSP, false, 0}, {TPOS, true, 0}, {-1, false, 0}, {-1, false, 0}} //TOP, invalid configuration
                                            };


    const static switches_type trie_map_leap_3 = {{{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {SPOT, false, 0}}, //SPOT
                                            {{-1, false, 0}, {-1, false, 0}, {TSPO, true, 0}, {-1, false, 0}}, //SPT
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //SO
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //ST
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {SPOT, true, 0}}, //POS
                                            {{TPOS, true, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //POT
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //PS
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //PT
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {SPOT, true, 0}}, //OSP
                                            {{-1, false, 0}, {TOSP, true, 0}, {-1, false, 0}, {-1, false, 0}}, //OST
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //OP
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //OT
                                            {{-1, false, 0}, {-1, false, 0}, {TSPO, true, 0}, {-1, false, 0}},  //TSPO
                                            {{-1, false, 0}, {TOSP, true, 0}, {-1, false, 0}, {-1, false, 0}}, //TSO
                                            {{TPOS, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //TPOS
                                            {{-1, false, 0}, {-1, false, 0}, {TSPO, true, 0}, {-1, false, 0}}, //TPS
                                            {{-1, false, 0}, {TOSP, false, 0}, {-1, false, 0}, {-1, false, 0}}, //TOSP
                                            {{TPOS, true, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}} //TOP
                                            };


    const static switches_type trie_map_3 = {{{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {SPOT, false, 0}}, //SPOT
                                            {{-1, false, 0}, {-1, false, 0}, {TSPO, true, 0}, {-1, false, 0}}, //SPT
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //SO
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //ST
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {SPOT, true, 0}}, //POS
                                            {{TPOS, true, 2}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //POT
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //PS
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //PT
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {SPOT, true, 0}}, //OSP
                                            {{-1, false, 0}, {TOSP, true, 4}, {-1, false, 0}, {-1, false, 0}}, //OST
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //OP
                                            {{-1, false, 0}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //OT
                                            {{-1, false, 0}, {-1, false, 0}, {TSPO, true, 0}, {-1, false, 0}},  //TSPO
                                            {{-1, false, 0}, {TOSP, true, 4}, {-1, false, 0}, {-1, false, 0}}, //TSO
                                            {{TPOS, false, 2}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}}, //TPOS
                                            {{-1, false, 0}, {-1, false, 0}, {TSPO, true, 0}, {-1, false, 0}}, //TPS
                                            {{-1, false, 0}, {TOSP, false, 4}, {-1, false, 0}, {-1, false, 0}}, //TOSP
                                            {{TPOS, true, 2}, {-1, false, 0}, {-1, false, 0}, {-1, false, 0}} //TOP
                                            };

}
#endif
