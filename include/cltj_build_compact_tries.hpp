//
// Created by Diego Arroyuelo on 5/09/24.
//

#ifndef CLTJ_CLTJ_BUILD_COMPACT_TRIES_HPP
#define CLTJ_CLTJ_BUILD_COMPACT_TRIES_HPP

#include <queue>
#include <cltj_config.hpp>
#include <cltj_temporal_wm.hpp>


namespace cltj {

        template<class trie_type>
        trie_type create_full_trie_SPOT(vector<spot_quad> &D) {

            std::sort(D.begin(), D.end(), comparator_order(SPOT));

            uint64_t c0 = 1, cur_value = D[0][spot_orders[SPOT][0]];
            std::vector<uint64_t> v0;
            std::vector<uint64_t> seq;

            for (uint64_t i = 1; i < D.size(); i++) {
                if (D[i][spot_orders[SPOT][0]] != D[i-1][spot_orders[SPOT][0]]) {
                    seq.push_back(cur_value);
                    cur_value = D[i][spot_orders[SPOT][0]];
                    v0.push_back(c0);
                    c0 = 1;
                } else c0++;
            }
            seq.push_back(cur_value);
            v0.push_back(c0);

            uint64_t c1, c2;
            std::vector<uint64_t> v1, v2;

            for (uint64_t i = 0, k = 0; i < v0.size(); i++) {
                c1 = 1;
                c2 = 1;
                for (uint64_t j = 1; j < v0[i]; j++) {
                    k++;
                    if (D[k][spot_orders[SPOT][1]] != D[k-1][spot_orders[SPOT][1]]) {
                        v2.push_back(c2);
                        seq.push_back(D[k-1][spot_orders[SPOT][1]]);
                        c2 = 1;
                        c1++;
                    } else c2++;
                }
                seq.push_back(D[k][spot_orders[SPOT][1]]);
                v2.push_back(c2);
                v1.push_back(c1);
                k++;
            }
            /***********/
            uint64_t c3, c4;
            std::vector<uint64_t> v3, v4;
            std::vector<temporal_interval> seq_interval;

            for (uint64_t i = 0, k = 0; i < v2.size(); i++) {
                c3 = 1;
                c4 = 1;
                for (uint64_t j = 1; j < v2[i]; j++) {
                    k++;
                    if (D[k][spot_orders[SPOT][2]] != D[k-1][spot_orders[SPOT][2]]) {
                        v4.push_back(c4);
                        seq.push_back(D[k-1][spot_orders[SPOT][2]]);
                        seq_interval.push_back(temporal_interval(D[k-1][spot_orders[SPOT][3]],D[k-1][spot_orders[SPOT][4]]));
                        c4 = 1;
                        c3++;
                    } else {
                          c4++;
                          seq_interval.push_back(temporal_interval(D[k-1][spot_orders[SPOT][3]],D[k-1][spot_orders[SPOT][4]]));
                    }
                }
                seq.push_back(D[k][spot_orders[SPOT][2]]);
                seq_interval.push_back(temporal_interval(D[k][spot_orders[SPOT][3]],D[k][spot_orders[SPOT][4]]));
                v4.push_back(c4);
                v3.push_back(c3);
                k++;
            }
            /***********/

            uint64_t c = seq.size() + seq_interval.size();//v0.size()+1;

            //for (uint64_t i = 0; i < v1.size(); i++)
            //    c += v1[i];

            //for (uint64_t i = 0; i < v2.size(); i++)
            //    c += v2[i];
            
            sdsl::bit_vector bv = sdsl::bit_vector(c, 1);
            sdsl::int_vector<> seq_compact = sdsl::int_vector<>(seq.size()/*+D.size()*/);

            uint64_t j = 0;

            bv[0] = 0;
            bv[v0.size()] = 0;
            j = v0.size();

            for (uint64_t i = 0; i < v1.size(); i++) {
                j += v1[i];
                bv[j] = 0;
            }

            for (uint64_t i = 0; i < v3.size(); i++) {
                j += v3[i];
                bv[j] = 0;
            }

            for (uint64_t i = 0; i < v4.size(); i++) {
                j += v4[i];
//                cout << j << "/" << bv.size() << " i = " << i << " v4.size() = " << v4.size() << endl;
                if (j < bv.size())
                    bv[j] = 0;
//                else
//                    cout << "cuidao" << endl;
            }

            for (j = 0; j < seq.size(); j++)
                seq_compact[j] = seq[j];

            /* CUIDADO CON LO QUE SIGUE QUE ME PARECE QUE NO VA MAS*/
            /*for (uint64_t i = 0; i < D.size(); i++)
                seq_compact[j++] = D[i][spo_orders[SPOT][2]];
            */

            /*
            cout << "v0:";
            for (uint64_t i=0; i < v0.size(); i++) cout << " " << v0[i];
            cout << endl;

            cout << "v1:";
            for (uint64_t i=0; i < v1.size(); i++) cout << " " << v1[i];
            cout << endl;

            cout << "v2:";
            for (uint64_t i=0; i < v2.size(); i++) cout << " " << v2[i];
            cout << endl;

            cout << "v3:";
            for (uint64_t i=0; i < v3.size(); i++) cout << " " << v3[i];
            cout << endl;

            cout << "v4:";
            for (uint64_t i=0; i < v4.size(); i++) cout << " " << v4[i];
            cout << endl;
            */


            /***/
            /*cout << "seq_interval:";
            for (uint64_t i=0; i < seq_interval.size(); i++) cout << "[" << seq_interval[i].first << ", " << seq_interval[i].second << "],  ";
            cout << endl;
            
            cout << "*** Trie para el orden " << (uint64_t)SPOT << " ***" << endl;
            for (uint64_t i = 0; i < bv.size(); i++) {
                cout << bv[i];
            }
            cout << endl;
            cout << "v2.size() = " << v2.size() << endl;
            for (uint64_t i = 0; i < seq_compact.size(); i++) {
                cout << seq_compact[i] << " ";
            }
            cout << endl;
            */
            /***/
            sdsl::util::bit_compress(seq_compact);

            return trie_type(bv, seq_compact, seq_interval, S_FIRST_FULL);
        }

        template<class trie_type>
        trie_type create_full_trie_no_t(vector<spot_quad> &D, uint8_t order) {

            std::sort(D.begin(), D.end(), comparator_order(order));

            /***/
            /*
            for (uint64_t i = 0; i < D.size(); i++)
                cout <<  D[i][spot_orders[order][0]] << " " <<  D[i][spot_orders[order][1]] << " " <<  D[i][spot_orders[order][2]] << " " <<  D[i][spot_orders[order][3]] << " " <<  D[i][spot_orders[order][4]] << endl;
            */
            /***/

            uint64_t c0 = 1, cur_value = D[0][spot_orders[order][0]];
            std::vector<uint64_t> v0;
            std::vector<uint64_t> seq;

            for (uint64_t i = 1; i < D.size(); i++) {
                if (D[i][spot_orders[order][0]] != D[i-1][spot_orders[order][0]]) {
                    seq.push_back(cur_value);
                    cur_value = D[i][spot_orders[order][0]];
                    v0.push_back(c0);
                    c0 = 1;
                } else c0++;
            }
            seq.push_back(cur_value);
            v0.push_back(c0);

            uint64_t c1, c2;
            std::vector<uint64_t> v1, v2;

            for (uint64_t i = 0, k = 0; i < v0.size(); i++) {
                c1 = 1;
                c2 = 1;
                for (uint64_t j = 1; j < v0[i]; j++) {
                    k++;
                    if (D[k][spot_orders[order][1]] != D[k-1][spot_orders[order][1]]) {
                                v2.push_back(c2);
                        seq.push_back(D[k-1][spot_orders[order][1]]);
                        c2 = 1;
                        c1++;
                    } else c2++;
                }
                seq.push_back(D[k][spot_orders[order][1]]);
                v2.push_back(c2);
                v1.push_back(c1);
                k++;
            }

            /***********/
            uint64_t c3, c4;
            std::vector<uint64_t> v3, v4;
            std::vector<temporal_interval> seq_interval;

            for (uint64_t i = 0, k = 0; i < v2.size(); i++) {
                c3 = 1;
                c4 = 1;
                for (uint64_t j = 1; j < v2[i]; j++) {
                    k++;
                    if (D[k][spot_orders[order][2]] != D[k-1][spot_orders[order][2]]) {
                        v4.push_back(c4);
                        seq.push_back(D[k-1][spot_orders[order][2]]);
                        seq_interval.push_back(temporal_interval(D[k-1][spot_orders[order][3]],D[k-1][spot_orders[order][4]]));
                        c4 = 1;
                        c3++;
                    } else {
                          c4++;
                          seq_interval.push_back(temporal_interval(D[k-1][spot_orders[order][3]],D[k-1][spot_orders[order][4]]));
                    }
                }
                seq.push_back(D[k][spot_orders[order][2]]);
                seq_interval.push_back(temporal_interval(D[k][spot_orders[order][3]],D[k][spot_orders[order][4]]));
                v4.push_back(c4);
                v3.push_back(c3);
                k++;
            }
            /***********/


            uint64_t c = seq.size() + seq_interval.size(); //v0.size()+1;

            /*for (uint64_t i = 0; i < v1.size(); i++)
                c += v1[i];

            for (uint64_t i = 0; i < v2.size(); i++)
                c += v2[i];
            */
            sdsl::bit_vector bv = sdsl::bit_vector(c, 1);
            sdsl::int_vector<> seq_compact = sdsl::int_vector<>(seq.size()/*+D.size()*/);

            uint64_t j = 0;

            bv[0] = 0;
            bv[v0.size()] = 0;
            j = v0.size();

            for (uint64_t i = 0; i < v1.size(); i++) {
                j += v1[i];
                bv[j] = 0;
            }

            for (uint64_t i = 0; i < v2.size(); i++) {
                j += v3[i];
                bv[j] = 0;
            }

             for (uint64_t i = 0; i < v4.size(); i++) {
                j += v4[i];
                if (j < bv.size())
                    bv[j] = 0;
            }

            for (j = 0; j < seq.size(); j++)
                seq_compact[j] = seq[j];

            /*for (uint64_t i = 0; i < D.size(); i++)
                seq_compact[j++] = D[i][spo_orders[order][2]];*/

            sdsl::util::bit_compress(seq_compact);
            /***/
            /*
            cout << "*** Trie para el orden " << (uint64_t)order << " ***" << endl;
            for (uint64_t i = 0; i < bv.size(); i++) {
                cout << bv[i];
            }
            cout << endl;
            for (uint64_t i = 0; i < seq_compact.size(); i++) {
                cout << seq_compact[i] << " ";
            }
            cout << endl;
            */
            /***/

            /*
            cout << "v0:";
            for (uint64_t i=0; i < v0.size(); i++) cout << " " << v0[i];
            cout << endl;

            cout << "v1:";
            for (uint64_t i=0; i < v1.size(); i++) cout << " " << v1[i];
            cout << endl;

            cout << "v2:";
            for (uint64_t i=0; i < v2.size(); i++) cout << " " << v2[i];
            cout << endl;

            cout << "v3:";
            for (uint64_t i=0; i < v3.size(); i++) cout << " " << v3[i];
            cout << endl;

            cout << "v4:";
            for (uint64_t i=0; i < v4.size(); i++) cout << " " << v4[i];
            cout << endl;
            */
            return trie_type(bv, seq_compact, TRIE_3_LEVELS_NO_T);
        }

        template<class trie_type> 
        trie_type create_partial_trie_2(vector<spot_quad> &D, uint8_t order){

            std::sort(D.begin(), D.end(), comparator_order(order));

            /*cout << "El orden en create_partial_trie_2 es " << (uint32_t)order << endl;

            for (uint64_t i = 0; i < D.size(); i++)
                cout <<  D[i][spot_orders[order][0]] << " " <<  D[i][spot_orders[order][1]] << " " <<  D[i][spot_orders[order][2]] << " " <<  D[i][spot_orders[order][3]] << " " <<  D[i][spot_orders[order][4]] << endl;*/

            uint64_t c0 = 1;
            std::vector<uint64_t> v0;
            std::vector<uint64_t> seq;

            for (uint64_t i = 1; i < D.size(); i++) {
                if (D[i][spot_orders[order][0]] != D[i-1][spot_orders[order][0]]) {
                    v0.push_back(c0);
                    c0 = 1;
                } else c0++;
            }
            v0.push_back(c0);

            uint64_t c1, c2;
            std::vector<uint64_t> v1, v2;

            for (uint64_t i = 0, k = 0; i < v0.size(); i++) {
                c1 = 1;
                c2 = 1;
                for (uint64_t j = 1; j < v0[i]; j++) {
                    k++;
                    if (D[k][spot_orders[order][1]] != D[k-1][spot_orders[order][1]]) {
                        v2.push_back(c2);
                        seq.push_back(D[k-1][spot_orders[order][1]]);
                        c2 = 1;
                        c1++;
                    } else c2++;
                }
                seq.push_back(D[k][spot_orders[order][1]]);
                v2.push_back(c2);
                v1.push_back(c1);
                k++;
            }

            uint64_t c = 0;
            for (uint64_t i = 0; i < v1.size(); i++)
                c += v1[i];

            sdsl::bit_vector bv = sdsl::bit_vector(c, 1);
            sdsl::int_vector<> seq_compact = sdsl::int_vector<>(seq.size());

            bv[0] = 0;
            uint64_t j = 1;
            for (uint64_t i = 0; i < v1.size(); i++) {
                j += v1[i];
                if (j-1 < bv.size())
                    bv[j-1] = 0;
            }

            for (j = 0; j < seq.size(); j++)
                seq_compact[j] = seq[j];

            sdsl::util::bit_compress(seq_compact);

            /***/
            /*
            cout << "*** Trie para el orden " << (uint64_t)order << " ***" << endl;
            for (uint64_t i = 0; i < bv.size(); i++) {
                cout << bv[i];
            }
            cout << endl;
            for (uint64_t i = 0; i < seq_compact.size(); i++) {
                cout << seq_compact[i] << " ";
            }
            cout << endl;
            */
            /***/

            /*
            cout << "v0:";
            for (uint64_t i=0; i < v0.size(); i++) cout << " " << v0[i];
            cout << endl;

            cout << "v1:";
            for (uint64_t i=0; i < v1.size(); i++) cout << " " << v1[i];
            cout << endl;

            cout << "v2:";
            for (uint64_t i=0; i < v2.size(); i++) cout << " " << v2[i];
            cout << endl;
            */

            return trie_type(bv, seq_compact, TRIE_2_LEVELS_NO_T);
        }

        struct interval_endpoint_info {
            temporal_endpoint_type ep; // the endpoint
            bool is_right_endpoint;
            uint64_t quad;
            interval_endpoint_info(temporal_endpoint_type _endpoint, bool _is_right_endpoint, uint64_t _quad) {
                ep = _endpoint;
                is_right_endpoint = _is_right_endpoint;
                quad = _quad;
            }
        };

        struct Compare {
            bool operator()(const interval_endpoint_info& a, const interval_endpoint_info& b) const {
                //if (a.ep != b.ep)
                return a.ep < b.ep /*|| (a.ep == b.ep && a.is_right_endpoint)*/;
                //else
                //    return a.is_right_endpoint;  // ante igualdad, busca dejar antes los extremos derechos de los intervalos
            }
        };

        std::vector<spot_quad> partition_graph_for_time_first(std::vector<spot_quad> &D) {

            std::vector<interval_endpoint_info> endpoints;
            for (uint64_t i = 0; i < D.size(); i++) {
                endpoints.push_back(interval_endpoint_info(D[i][3], false, i));
                endpoints.push_back(interval_endpoint_info(D[i][4]/*+1*/, true, i));  // Eliminate the +1 when assuming that intervals in the input graph will be in the form [ti, tf)
            }

            std::sort(endpoints.begin(), endpoints.end(), Compare());

            cout << "partition_graph_for_time_first() imprimiendo los endpoints ordenados:" << endl;
            for (uint64_t i = 0; i < endpoints.size(); i++)
                cout << endpoints[i].ep << " " << ((endpoints[i].is_right_endpoint)?"right endpoint": "left endpoint") << endl;

            std::set<uint64_t> active_intervals;

            temporal_endpoint_type cur_ep = endpoints[0].ep;
            active_intervals.insert(endpoints[0].quad);

            std::vector<std::pair<temporal_interval, std::vector<uint64_t>>> ground_intervals;

            for (uint64_t i = 1; i < endpoints.size(); /*i++*/) {
                //std::pair<interval_endpoint_info, interval_endpoint_info>
                temporal_interval cur_interval(endpoints[i-1].ep, endpoints[i].ep);

                ground_intervals.push_back(std::pair<temporal_interval, std::vector<uint64_t>>(cur_interval, std::vector<uint64_t>()));

                if (!endpoints[i].is_right_endpoint) {
                    for (uint64_t act_int : active_intervals)
                        ground_intervals[ground_intervals.size()-1].second.push_back(act_int);
                    
                    active_intervals.insert(endpoints[i].quad);
                    i++;
                    while (i < endpoints.size() && endpoints[i].ep == endpoints[i-1].ep/*cur_ep*/) {
                        cout << "* l " << i << " *" << endl;
                        /*if (endpoints[i].is_right_endpoint) {
                            cout << "Cuidado en partition_graph_for_time_first, esto no deberia pasar, esta mala la funcion de sort" << endl;
                            exit(1);
                        }*/
                        active_intervals.insert(endpoints[i].quad);
                        i++;
                    }

                    ground_intervals[ground_intervals.size()-1].first.second = endpoints[i-1].ep;

                } else {
                    for (uint64_t act_int : active_intervals)
                        ground_intervals[ground_intervals.size()-1].second.push_back(act_int);

                    active_intervals.erase(endpoints[i].quad);
                    i++;

                    while (i < endpoints.size() && endpoints[i].ep == endpoints[i-1].ep /*cur_ep*/ && endpoints[i].is_right_endpoint) {
                        cout << "* r " << i << " *" << endl;
                        active_intervals.erase(endpoints[i].quad);
                        i++;
                    }

                    while (i < endpoints.size() && endpoints[i].ep == endpoints[i-1].ep /*cur_ep*/) {
                        cout << "* ll " << i << " *" << endl;
                        /*if (endpoints[i].is_right_endpoint) {
                            cout << "Cuidado en partition_graph_for_time_first, esto no deberia pasar, esta mala la funcion de sort" << endl;
                            exit(1);
                        }*/
                        active_intervals.insert(endpoints[i].quad);
                        i++;
                    }
                    ground_intervals[ground_intervals.size()-1].first.second = endpoints[i-1].ep;
                }
                cur_ep = endpoints[i].ep;
            }

            cout << "En partition_graph_for_time_first mostrando los intervalos generados:" << endl;
            for (uint64_t i = 0; i < ground_intervals.size(); i++)
                cout << "[" << ground_intervals[i].first.first << ", " << ground_intervals[i].first.second << ")" << endl;

            std::vector<spot_quad> new_D;
            for (uint64_t i = 0; i < ground_intervals.size(); i++) {
                for (uint64_t j = 0; j < ground_intervals[i].second.size(); j++) {
                    uint64_t cur_tuple = ground_intervals[i].second[j];
                    new_D.push_back({D[cur_tuple][0], D[cur_tuple][1], D[cur_tuple][2],
                                     ground_intervals[i].first.first, ground_intervals[i].first.second}
                                   );
                }
            }

            cout <<  "En partition_graph_for_time_first() mostrando las nuevas tuplas generadas:" << endl;
            for (uint64_t i = 0; i < new_D.size(); i++) {
                cout << new_D[i][0] << " " << new_D[i][1] << " " << new_D[i][2] << " " << new_D[i][3] << " " << new_D[i][4] << " " << endl;
            }
            cout << "Saliendo de partition_graph_for_time_first()" << endl; 
            return new_D;
        }



        //std::vector<spot_quad>
        struct update_type {
            uint64_t tuple_index;
            bool     is_delete;
            update_type() {};
            update_type(uint64_t _tuple_index, bool _is_delete) {
                tuple_index = _tuple_index;
                is_delete   = _is_delete;
            }
        };
 
        std::vector<std::pair<temporal_interval, std::vector<update_type>>> 
        generate_list_of_updates(std::vector<spot_quad> &D) {
            /*OJO aca, es para que ante igualdad, los extremos derechos queden antes que los izquierdos*/
            //std::sort(D.begin(), D.end(), comparator_order(TSPO));

            std::vector<interval_endpoint_info> endpoints;
            for (uint64_t i = 0; i < D.size(); i++) {
                endpoints.push_back(interval_endpoint_info(D[i][3], false, i));
                endpoints.push_back(interval_endpoint_info(D[i][4]/*+1*/, true, i));  // Eliminate the +1 when assuming that intervals in the input graph will be in the form [ti, tf)
            }

            std::sort(endpoints.begin(), endpoints.end(), Compare());

            /***/
            /*
            cout << "generate_list_of_updates() imprimiendo los endpoints ordenados:" << endl;
            for (uint64_t i = 0; i < endpoints.size(); i++)
                cout << endpoints[i].ep << " " << ((endpoints[i].is_right_endpoint)?"right endpoint": "left endpoint") << endl;
            */
            /***/

            std::vector<std::pair<temporal_interval, std::vector<update_type>>> ground_intervals;
            bool flag = false;

            for (uint64_t i = 1; i < endpoints.size(); /*i++*/) {
                temporal_interval cur_interval(endpoints[i-1].ep, endpoints[i].ep);

                ground_intervals.push_back(std::pair<temporal_interval, std::vector<update_type/*uint64_t*/>>(cur_interval, std::vector<update_type/*uint64_t*/>()));

                while (i < endpoints.size() && endpoints[i].ep == endpoints[i-1].ep) {
                        flag = true;
                        ground_intervals[ground_intervals.size()-1].second.push_back(update_type(endpoints[i-1].quad, endpoints[i-1].is_right_endpoint)/*endpoints[i-1].quad*/);
                        i++;
                }
                    
                if (flag) {
                    flag = false;
                    ground_intervals[ground_intervals.size()-1].first.second = endpoints[i].ep;
                    ground_intervals[ground_intervals.size()-1].second.push_back(update_type(endpoints[i-1].quad, endpoints[i-1].is_right_endpoint)/*endpoints[i-1].quad*/);
                    i++;
                } else {
		            ground_intervals[ground_intervals.size()-1].second.push_back(update_type(endpoints[i-1].quad, endpoints[i-1].is_right_endpoint)/*endpoints[i-1].quad*/);
                    i++;
		        }
            }
            
            /*
            cout << "En generate_list_of_updates() mostrando los intervalos generados:" << endl;
            for (uint64_t i = 0; i < ground_intervals.size(); i++)
                cout << "[" << ground_intervals[i].first.first << ", " << ground_intervals[i].first.second << ")" << endl;
            
            
            std::vector<spot_quad> new_D;
            for (uint64_t i = 0; i < ground_intervals.size(); i++) {
                for (uint64_t j = 0; j < ground_intervals[i].second.size(); j++) {
                    uint64_t cur_tuple = ground_intervals[i].second[j].tuple_index;
                    cout << ground_intervals[i].second[j].is_delete << endl;
                    new_D.push_back({D[cur_tuple][0], D[cur_tuple][1], D[cur_tuple][2],
                                     ground_intervals[i].first.first, ground_intervals[i].first.second}
                                   );
                }

            }

            cout <<  "En generate_list_of_updates() mostrando las nuevas tuplas generadas:" << endl;
            for (uint64_t i = 0; i < new_D.size(); i++) {
                cout << new_D[i][0] << " " << new_D[i][1] << " " << new_D[i][2] << " " << new_D[i][3] << " " << new_D[i][4] << " " << endl;
            }
            */
            return ground_intervals; //new_D;
        }


 
        template<class trie_type> 
        trie_type create_time_first_trie(std::vector<spot_quad> &D, 
                                         std::vector<std::pair<temporal_interval, 
                                         std::vector<update_type/*uint64_t*/>>> &D_T,
                                         uint8_t order, bool stores_T, uint64_t n_bits, 
                                         uint64_t n_tuple_components) {
            std::vector<temporal_interval> interval_list;
            sdsl::bit_vector last_update_per_interval;

            uint64_t n_updates = 0;
            if (stores_T) {
                for (uint64_t i = 0; i < D_T.size(); i++) {
                   interval_list.push_back(D_T[i].first);
                   n_updates += D_T[i].second.size();
	            }
                /***/
                /*
                cout << "create_time_first_trie(): n_updates = " << n_updates << endl;
	            */
                /***/
                last_update_per_interval = sdsl::bit_vector(n_updates, 0);
	            uint64_t j = 0;
	            for (uint64_t i = 0; i < D_T.size(); i++) {
	                j += D_T[i].second.size();
		            last_update_per_interval[j-1] = 1;
	            }
	            /***/
                /*
                cout << "create_time_first_trie() mostrando el BV de updates per interval:" << endl;
                for (uint64_t j = 0; j < last_update_per_interval.size(); j++)
	                cout << last_update_per_interval[j];
	            cout << endl;
                */
               /***/
                /*uint64_t canti = 0;
                for (uint64_t kk = 0; kk < last_update_per_interval.size(); kk++)
                    if (last_update_per_interval[kk]) canti++;

                cout << "Bit vector tiene " << last_update_per_interval.size() << " bits y " << canti << " 1s" << endl;
                
                cout << "**** " << last_update_per_interval[last_update_per_interval.size()-1] << endl;
                */
            }        

        /*
            for (uint64_t i = 0; i < ground_intervals.size(); i++)
                cout << "[" << ground_intervals[i].first.first << ", " << ground_intervals[i].first.second << ")" << endl;
        */
            std::vector<spot_quad> new_D;
            std::vector<bool> is_delete;
            for (uint64_t i = 0; i < D_T.size(); i++) {
                for (uint64_t j = 0; j < D_T[i].second.size(); j++) {
                    uint64_t cur_tuple = D_T[i].second[j].tuple_index;
                    new_D.push_back({D[cur_tuple][spot_orders[order][1]], D[cur_tuple][spot_orders[order][2]], D[cur_tuple][spot_orders[order][3]],
                                     D_T[i].first.first, D_T[i].first.second}
                                   );
                    is_delete.push_back(D_T[i].second[j].is_delete);
                }
            }

            // sorts the tuples according to order
            
            std::vector<uint64_t> idx(new_D.size());
            std::iota(idx.begin(), idx.end(), 0); // generates the id permutation

            std::sort(idx.begin(), idx.end(), comparator_order_idx(new_D, order));
            
            std::vector<spot_quad> newest_D(idx.size());
            std::vector<bool> newest_is_delete(idx.size());
            for (uint64_t i = 0; i < idx.size(); ++i) {
                newest_D[i] = new_D[idx[i]];
                newest_is_delete[i] = is_delete[idx[i]];
            }

            is_delete.swap(newest_is_delete);
            new_D.swap(newest_D);

            newest_D.clear();
            newest_is_delete.clear();
            //std::sort(new_D.begin(), new_D.end(), comparator_order(order));

            /***/
            
            /*cout <<  "En create_time_first_trie() mostrando las nuevas tuplas generadas:" << endl;
            for (uint64_t i = 0; i < new_D.size(); i++) {
                cout << new_D[i][spot_orders[order][1]] << " " << new_D[i][spot_orders[order][2]] << " " << new_D[i][spot_orders[order][3]] << " " << new_D[i][spot_orders[order][0]] << " " << new_D[i][spot_orders[order][4]] << " " << endl;
            }*/
            //exit(1);
            /***/

            // now build the "wavelet matrices" for the sequence of updates 
            temporal_wm<> temp_ds(new_D, is_delete, n_tuple_components, n_bits, n_tuple_components==2);
            is_delete.clear(); // OJO con esto

            if (stores_T) {
                // TSPO
                return trie_type(interval_list, temp_ds, last_update_per_interval, TIME_FIRST_FULL_INTERVALS);
            } else {
                if (n_tuple_components == 3) {
                    // TPOS, TOSP
                    return trie_type(interval_list, temp_ds, last_update_per_interval, TIME_FIRST_FULL_NO_INTERVALS);
                } else {
                    // TOP, TPS, TSO
                    return trie_type(interval_list, temp_ds, last_update_per_interval, TIME_FIRST_PARTIAL);
                }
            }
        }

        // Given a set of tuples (that share certain components), grinds their time intervals, returning
        // the corresponding set of intervals
        //std::vector<spot_quad>

        std::vector<temporal_interval> grind_intervals_3(std::vector<spot_quad> &D, uint64_t l, uint64_t r) {
            std::vector<temporal_interval> interval_set;

            std::vector<temporal_endpoint_type> endpoints;

            for (uint64_t i = l; i <= r; i++) {
                endpoints.push_back(D[i][3]);
                endpoints.push_back(D[i][4]/*+1*/);   // Eliminate the +1 when assuming that intervals in the input graph will be in the form [ti, tf)
            }
            std::sort(endpoints.begin(), endpoints.end());
            auto last = std::unique(endpoints.begin(), endpoints.end());
            endpoints.erase(last, endpoints.end());

            /*cout << "en grind_intervals_3 los endpoints son" << endl;
            for (uint64_t i = 0; i < endpoints.size(); i++)
                cout << endpoints[i] << endl;
            */

            /*uint32_t temp;
            while (min_heap.size() > 0) {
                temp = min_heap.top().endpoint;
                min_heap.pop();
                interval_set.push_back(std::pair<uint32_t, uint32_t>(temp, min_heap.top().endpoint));
            }*/

            //if (endpoints.size() == 0) cout << "Cuidado" << endl;
            for (uint64_t i = 0; i < endpoints.size()-1; i++) {
                //cout << i << " " << endpoints.size()-1 << endl;
                interval_set.push_back(temporal_interval(endpoints[i], endpoints[i+1]));
            }
            //cout << "----" << endl;

            /*cout << "Para el rango " << l << ", " << r << " de D tenemos los intervalos: " << endl;
            for (uint64_t i = 0; i < interval_set.size(); i++)
                cout << " [" << interval_set[i].first << ", " << interval_set[i].second << ")";

            cout << "****" << endl;
            */

            return interval_set;
        }

        template<class trie_type>
        trie_type create_partial_trie_2T(vector<spot_quad> &D, uint8_t order) {

            std::sort(D.begin(), D.end(), comparator_order(order));
            /*cout << "El orden en create_partial_trie_2T es " << (uint32_t)order << endl;

            for (uint64_t i = 0; i < D.size(); i++)
                cout <<  D[i][spot_orders[order][0]] << " " <<  D[i][spot_orders[order][1]] << " " <<  D[i][spot_orders[order][2]] << " " <<  D[i][spot_orders[order][3]] << " " <<  D[i][spot_orders[order][4]] << endl;
            */

            uint64_t first, last;
            std::vector<std::vector<temporal_interval>> interval_set_vect;

            first = 0;
            for (uint64_t i = 1; i < D.size(); i++) {
                //cout << "++++++" << endl;
                while (i < D.size()
                       && D[i][spot_orders[order][0]] == D[i-1][spot_orders[order][0]]
                       && D[i][spot_orders[order][1]] == D[i-1][spot_orders[order][1]]
                      ) {
                    /*cout <<  D[i-1][spot_orders[order][0]] << " " <<  D[i][spot_orders[order][0]] << endl;
                    cout <<  D[i-1][spot_orders[order][1]] << " " <<  D[i][spot_orders[order][1]] << endl;
                    cout << "------" << endl;*/
                    ++i;
                }
                last = i-1;
                //cout << "2T() first = " << first << " last = " << last << endl; 
                interval_set_vect.push_back(grind_intervals_3(D, first, last));
                first = i;
            }
            //cout << "2T() out of for first = " << first << " last = " << D.size()-1 << endl;
            if (first <= D.size()-1)
                interval_set_vect.push_back(grind_intervals_3(D, first, D.size()-1));

            /***/
            /*cout << "interval_set_vect has " << interval_set_vect.size() << " elements" << endl;
            for (uint64_t i = 0; i < interval_set_vect.size(); i++) {
                for (uint64_t j = 0; j < interval_set_vect[i].size(); j++)
                   cout << " [" << interval_set_vect[i][j].first << ", " << interval_set_vect[i][j].second << ")";;
                cout << endl;
            }
            */
            /***/
            uint64_t c = 0;
            for (uint64_t i = 0; i < interval_set_vect.size(); i++)
                c += interval_set_vect[i].size();

            sdsl::bit_vector bv = sdsl::bit_vector(c, 1);
            sdsl::int_vector<> dummy_seq; //(1);

            bv[0] = 0;
            uint64_t j = 1;
            for (uint64_t i = 0; i <  interval_set_vect.size(); i++) {
                j += interval_set_vect[i].size();
                if (j-1 < bv.size())
                    bv[j-1] = 0;
            }

            //dummy_seq[0] = 1; // just a dummy value, this trie has no integer sequence associated
            //sdsl::util::bit_compress(dummy_seq);

            std::vector<temporal_interval> interval_seq;

            for (j = 0; j < interval_set_vect.size(); j++)
                for (uint64_t i = 0; i < interval_set_vect[j].size(); i++)
                    interval_seq.push_back(interval_set_vect[j][i]);

            /***/
            /*cout << "*** Trie para el orden " << (uint64_t)order << " ***" << endl;
            for (uint64_t i = 0; i < bv.size(); i++) {
                cout << bv[i];
            }
            cout << endl;
            */
            /***/

            /*for (uint64_t i = 0; i < seq_compact.size(); i++) {
                cout << seq_compact[i] << " ";
            }
            cout << endl;*/
            return trie_type(bv, dummy_seq, interval_seq, TRIE_3_LEVELS_T_LAST);
        }

        template<class trie_type>
        trie_type create_partial_trie_1T(vector<spot_quad> &D, uint8_t order) {
            std::sort(D.begin(), D.end(), comparator_order(order));                                                                       
            /*cout << "El orden en create_partial_trie_1T es " << (uint32_t)order << endl;                                                
            for (uint64_t i = 0; i < D.size(); i++)                                                                                                       cout <<  D[i][spot_orders[order][0]] << " " <<  D[i][spot_orders[order][1]] << " " <<  D[i][spot_orders[order][2]] << " " <<  D[i][spot_orders[order][3]] << " " <<  D[i][spot_orders[order][4]] << endl;*/

            uint64_t first, last;
            std::vector<std::vector<temporal_interval>> interval_set_vect;

            first = 0;
            for (uint64_t i = 1; i < D.size(); i++) {
                //cout << "++++++" << endl;
                while (i < D.size()
                       && D[i][spot_orders[order][0]] == D[i-1][spot_orders[order][0]]
                      ) {
                    /*cout <<  D[i-1][spot_orders[order][0]] << " " <<  D[i][spot_orders[order][0]] << endl;
                    cout <<  D[i-1][spot_orders[order][1]] << " " <<  D[i][spot_orders[order][1]] << endl;
                    cout << "------" << endl;*/
                    ++i;
                }

                last = i-1;
                interval_set_vect.push_back(grind_intervals_3(D, first, last));
                first = i;
            }
            if (first < D.size()) interval_set_vect.push_back(grind_intervals_3(D, first, D.size()-1));

            uint64_t c = 0;
            for (uint64_t i = 0; i < interval_set_vect.size(); i++)
                c += interval_set_vect[i].size();

            sdsl::bit_vector bv = sdsl::bit_vector(c, 1);
            sdsl::int_vector<> dummy_seq;//(1);

            bv[0] = 0;
            uint64_t j = 1;
            for (uint64_t i = 0; i <  interval_set_vect.size(); i++) {
                j += interval_set_vect[i].size();
                if (j-1 < bv.size())
                    bv[j-1] = 0;
            }

            //dummy_seq[0] = 1; // just a dummy value, this trie has no integer sequence associated
            //sdsl::util::bit_compress(dummy_seq);

            std::vector<temporal_interval> interval_seq;

            for (j = 0; j < interval_set_vect.size(); j++)
                for (uint64_t i = 0; i < interval_set_vect[j].size(); i++)
                    interval_seq.push_back(interval_set_vect[j][i]);

            /***/
            /*
            cout << "interval_set_vect has " << interval_set_vect.size() << " elements" << endl;
            for (uint64_t i = 0; i < interval_set_vect.size(); i++) {
                for (uint64_t j = 0; j < interval_set_vect[i].size(); j++)
                   cout << " [" << interval_set_vect[i][j].first << ", " << interval_set_vect[i][j].second << ")";
                cout << endl;
            }

            cout << "*** Trie para el orden " << (uint64_t)order << " ***" << endl;
            for (uint64_t i = 0; i < bv.size(); i++) {
                cout << bv[i];
            }
            cout << endl;
            */
            /***/

            /*for (uint64_t i = 0; i < seq_compact.size(); i++) {
                cout << seq_compact[i] << " ";
            }
            cout << endl;*/
            return trie_type(bv, dummy_seq, interval_seq, TRIE_2_LEVELS_T_LAST);
        }
}
#endif
