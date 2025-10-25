/*
 * ltj_iterator.hpp
 * Copyright (C) 2023 Author removed for double-blind evaluation
 *
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LTJ_ITERATOR_V2_HPP
#define LTJ_ITERATOR_V2_HPP

#include <triple_pattern.hpp>
#include <quad_pattern.hpp>
#include <cltj_config.hpp>
#include <vector>
#include <utils.hpp>
#include <string>
#include <cltj_index_temporal_metatrie.hpp>

#define VERBOSE 0

using namespace cltj;

namespace ltj {

    template<class index_scheme_t, class var_t, class cons_t>
    class ltj_iterator_v2 {//TODO: if CLTJ is eventually integrated with the ring to form a Compact Index Schemes project then this class has to be renamed to CLTJ_iterator, for instance.

    public:
        typedef cons_t value_type;
        typedef var_t var_type;
        typedef index_scheme_t index_scheme_type;
        typedef uint64_t size_type;

        typedef struct {
            std::array<size_type, 6> it;
            size_type cnt;
            size_type beg;
            size_type end;
            /*OJO*/
            size_type trie;
        } level_data_type;
        typedef std::array<level_data_type, 5> status_type;
        typedef std::array<bool, 5> redo_array_type;
        //std::vector<value_type> leap_result_type;


    private:
        const quad_pattern *m_ptr_quad_pattern;
        index_scheme_type *m_ptr_index; //TODO: should be const
        
        bool m_is_empty = false;
        size_type m_nfixed = 0;
        std::array<state_type, 4> m_fixed;

        std::array<size_type, 4> m_path_label; 

        size_type m_trie_i = 0;
        size_type m_status_i = 0;
        status_type m_status;
        redo_array_type m_redo;
        bool m_switch = false; // indicates whether to do a trie switch or not
        bool m_below_time_level = false;

        void copy(const ltj_iterator_v2 &o) {
            m_ptr_quad_pattern = o.m_ptr_quad_pattern;
            m_ptr_index = o.m_ptr_index;
            m_nfixed = o.m_nfixed;
            m_fixed = o.m_fixed;
            m_is_empty = o.m_is_empty;
            m_trie_i = o.m_trie_i;
            m_status_i = o.m_status_i;
            m_status = o.m_status;
            m_redo = o.m_redo;
            m_path_label = o.m_path_label;
            m_switch = o.m_switch;
            m_below_time_level = o.m_below_time_level;
        }

        void print_status() {
            std::cout << "fixed: " << m_nfixed << std::endl;
            for(int i = 0; i < m_status.size(); ++i){
                std::cout << "it0=" << m_status[i].it[0] << " it1=" << m_status[i].it[1] 
                          << " cnt=" << m_status[i].cnt  << " beg=" << m_status[i].beg 
                          << " end=" << m_status[i].end << std::endl;
            }
        }

        void print_redo() {
            for(int i = 0; i < m_redo.size(); ++i) {
                std::cout << m_redo[i] << " ";
            }
            std::cout << std::endl;
        }

        void print_path() {
            cout << "Current path: ";
            for (int i = 0; i < m_nfixed; i++) cout << m_path_label[i] << " ";
            cout << endl;
        }

        void process_constants() {
            //cout << "process_constants()" << endl;

            //cout << "S" << endl;
            if (!m_ptr_quad_pattern->s_is_variable()) {
                std::pair<size_type, size_type> c = leap_constant(s, m_ptr_quad_pattern->term_s.value);
                if (c.first != m_ptr_quad_pattern->term_s.value) {
                    m_is_empty = true;
                    return;
                }
                m_path_label[m_nfixed] = m_ptr_quad_pattern->term_s.value;
                down(s);
            }

            //cout << "P" << endl;
            if (!m_ptr_quad_pattern->p_is_variable()) {
                std::pair<size_type, size_type> c = leap_constant(p, m_ptr_quad_pattern->term_p.value);
                if (c.first != m_ptr_quad_pattern->term_p.value) {
                    m_is_empty = true;
                    return;
                }
                m_path_label[m_nfixed] = m_ptr_quad_pattern->term_p.value;
                down(p);
            }

            //cout << "O" << endl;
            if (!m_ptr_quad_pattern->o_is_variable()) {
                std::pair<size_type, size_type> c = leap_constant(o, m_ptr_quad_pattern->term_o.value);
                if (c.first != m_ptr_quad_pattern->term_o.value) {
                    m_is_empty = true;
                    return;
                }
                m_path_label[m_nfixed] = m_ptr_quad_pattern->term_o.value;
                down(o);
            }

            //cout << "T" << endl;
            if (!m_ptr_quad_pattern->t_is_variable()) {
                value_type t_value = m_ptr_quad_pattern->term_t.value;
                std::pair<size_type, size_type> c = leap_constant(t, t_value);
                if (! (c.first <= t_value && t_value < c.second)) {
                    m_is_empty = true;
                    return;
                }
                m_path_label[m_nfixed] = t_value;
                down(t);
            }
        }

        void choose_trie_down(state_type state) {
            if(m_nfixed == 0) {
                // at depth 0 in the trie, so we are going down by the first state
                m_trie_i = 4*state;   //OJO con el 4, antes era 2
                m_status_i = 0;
                m_switch = false;
            } else if (m_nfixed == 1) {
                // at depth 1 in the trie, a single state is already fixed
                // so we are going down by the second
                if (state == s) { //Fix variables
                    switch (m_fixed[m_nfixed-1]) {
                        case o: m_trie_i = OSP;
                                m_status_i = 0; 
                                m_switch = false;
                                break;
                        case p: m_trie_i = SPOT;
                                m_status_i = 2; 
                                m_switch = true;
                                break;
                        case t: m_trie_i = TSPO;
                                m_status_i = 0;  
                                m_switch = false;
                                break; 
                    }
                } else if (state == p) {
                    switch (m_fixed[m_nfixed-1]) {
                        case s: m_trie_i = SPOT;
                                m_status_i = 0;
                                m_switch = false;
                                break;
                        case o: m_trie_i = POS;
                                m_status_i = 2; // at node PO in the metatrie, the second child of its metatrie parent
                                m_switch = true;
                                break;
                        case t: m_trie_i = TPOS;
                                m_status_i = 2; 
                                m_switch = false;  
                                break;
                    }
                } else if (state == o) {
                    switch(m_fixed[m_nfixed-1]) {
                        case s: m_trie_i = OSP; 
                                m_status_i = 2; 
                                m_switch = true;
                                break;
                        case p: m_trie_i = POS;
                                m_status_i = 0; 
                                m_switch = false;
                                break;
                        case t: m_trie_i = TOSP; 
                                m_status_i = 2;
                                m_switch = false;
                                break;
                    }
                } else { // state == t
                    switch (m_fixed[m_nfixed-1]) {
                        case s: m_trie_i = TSPO;
                                m_status_i = 3;
                                m_switch = true;
                                break;
                        case p: m_trie_i = TPOS;
                                m_status_i = 3;
                                m_switch = true;
                                break;
                        case o: m_trie_i = TOSP;
                                m_status_i = 3;
                                m_switch = true;
                                break;
                    }
                }
            } else if (m_nfixed == 2) {
                // at depth 2 in the trie, 2 states are already fixed
                // so we are going down by the third
                size_type t_i = m_trie_i;
                m_trie_i = trie_map_2[t_i][state].m_trie;
                m_switch = trie_map_2[t_i][state].m_switch;
                m_status_i = trie_map_2[t_i][state].m_status;   
            } else if (m_nfixed == 3) {
                // at depth 2 in the trie, 2 states are already fixed
                // so we are going down by the fourth
                size_type t_i = m_trie_i;
                m_trie_i = trie_map_3[t_i][state].m_trie;
                m_switch = trie_map_3[t_i][state].m_switch;
                m_status_i = trie_map_3[t_i][state].m_status;
            }
        }

        void choose_trie_leap(state_type state, size_type &t_i, size_type &s_i) {
              
            if(m_nfixed == 0) {
                // at depth 0 in the trie, so we are going down by the first state
                t_i = 4*state;  
                s_i = 0;
                //m_switch = false;
            } else if (m_nfixed == 1) {
                // at depth 1 in the trie, a single state is already fixed
                // so we are going down by the second
                if (state == s) { //Fix variables
                    switch (m_fixed[m_nfixed-1]) {
                        case o: t_i = OSP;
                                s_i = 0; 
                                break;
                        case p: t_i = PS;
                                s_i = 2; 
                                break;
                        case t: t_i = TSPO;
                                s_i = 0;   
                                break; 
                    }
                } else if (state == p) {
                    switch (m_fixed[m_nfixed-1]) {
                        case s: t_i = SPOT;
                                s_i = 0; 
                                break;
                        case o: t_i = OP;
                                s_i = 2; 
                                break;
                        case t: t_i = TPOS;
                                s_i = 1;
                                break;
                    }
                } else if (state == o) {
                    switch(m_fixed[m_nfixed-1]) {
                        case s: t_i = SO; 
                                s_i = 2; 
                                break;
                        case p: t_i = POS;
                                s_i = 0; 
                                break;
                        case t: t_i = TOSP; 
                                s_i = 2; 
                                break;
                    }
                } else { // state == t
                    switch (m_fixed[m_nfixed-1]) {
                        case s: t_i = ST;
                                s_i = 3;
                                break;
                        case p: t_i = PT;
                                s_i = 3;
                                break;
                        case o: t_i = OT;
                                s_i = 3;  
                                break;
                    }
                }
            } else if (m_nfixed == 2) {
                // at depth 2 in the trie, 2 states are already fixed
                // so we are going down by the third
               t_i = trie_map_leap_2[m_trie_i][state].m_trie;
               //m_switch = trie_map_2[m_trie_i][state].m_switch;
               s_i = trie_map_leap_2[m_trie_i][state].m_status;   
            } else if (m_nfixed == 3) {
                // at depth 2 in the trie, 2 states are already fixed
                // so we are going down by the fourth
               t_i = trie_map_leap_3[m_trie_i][state].m_trie;
               //m_switch = trie_map_3[m_trie_i][state].m_switch;
               s_i = trie_map_leap_3[m_trie_i][state].m_status;
            }
        }

    public:
        /*
            Returns the key of the current position of the iterator
        */
        const size_type &nfixed = m_nfixed;


        inline bool is_variable_subject(var_type var) {
            return m_ptr_quad_pattern->term_s.is_variable && var == m_ptr_quad_pattern->term_s.value;
        }

        inline bool is_variable_predicate(var_type var) {
            return m_ptr_quad_pattern->term_p.is_variable && var == m_ptr_quad_pattern->term_p.value;
        }

        inline bool is_variable_object(var_type var) {
            return m_ptr_quad_pattern->term_o.is_variable && var == m_ptr_quad_pattern->term_o.value;
        }

        inline bool is_variable_time(var_type var) {
            return m_ptr_quad_pattern->term_t.is_variable && var == m_ptr_quad_pattern->term_t.value;
        }

        inline const bool is_empty() {
            return m_is_empty;
        }

        inline size_type parent() const;


        ltj_iterator_v2() = default;
        ltj_iterator_v2(const quad_pattern *quad, index_scheme_type *index) {
            m_ptr_quad_pattern = quad;
            m_ptr_index = index;

            m_status[0].it[0] = 0;
            m_status[0].it[1] = 0;
            m_status[0].beg = 1;
            m_status[0].end = 0;
            m_status[0].cnt = 1;
            //m_status[1][0].it = 2;
            //m_status[1][0].last = 1;
            m_redo[0] = true;
            m_redo[1] = true;
            m_redo[2] = true;
            m_redo[3] = true;

            process_constants();
        }

        const quad_pattern* get_quad_pattern() const {
            return m_ptr_quad_pattern;
        }
        //! Copy constructor
        ltj_iterator_v2(const ltj_iterator_v2 &o) {
            copy(o);
        }

        //! Move constructor
        ltj_iterator_v2(ltj_iterator_v2 &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        ltj_iterator_v2 &operator=(const ltj_iterator_v2 &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        ltj_iterator_v2 &operator=(ltj_iterator_v2 &&o) {
            if (this != &o) {
                m_ptr_quad_pattern = std::move(o.m_ptr_quad_pattern);
                m_ptr_index = std::move(o.m_ptr_index);
                m_nfixed = std::move(o.m_nfixed);
                m_fixed = std::move(o.m_fixed);
                m_is_empty = std::move(o.m_is_empty);
                m_trie_i = std::move(o.m_trie_i);
                m_status_i = std::move(o.m_status_i);
                m_status = std::move(o.m_status);
                m_redo = std::move(o.m_redo);
                m_path_label = std::move(o.m_path_label);
                m_switch = std::move(o.m_switch);
                m_below_time_level = std::move(o.m_below_time_level);
            }
            return *this;
        }

        void swap(ltj_iterator_v2 &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_ptr_quad_pattern, o.m_ptr_quad_pattern);
            std::swap(m_ptr_index, o.m_ptr_index);
            std::swap(m_nfixed, o.m_nfixed);
            std::swap(m_fixed, o.m_fixed);
            std::swap(m_is_empty, o.m_is_empty);
            std::swap(m_trie_i, o.m_trie_i);
            std::swap(m_status_i, o.m_status_i);
            std::swap(m_status, o.m_status);
            std::swap(m_redo, o.m_redo);
            std::swap(m_path_label, o.m_path_label);
            std::swap(m_switch, o.m_switch);
            std::swap(m_below_time_level, o.m_below_time_level);
        }

        #include <cltj_trie_switch.hpp>

        void down(state_type state) {

            if (at_time_level()) {
                m_below_time_level = true; // we were at time level and are descending now
            }

            m_status[m_nfixed].trie = m_trie_i;  // saves the trie at previous level
            m_fixed[m_nfixed] = state;
            choose_trie_down(state);
            ++m_nfixed;

            if(m_nfixed == 1) {
                if (state != t) {
                    // we are in a regular trie
                    // At the first level, in the regular tries 3 different tries share the first level
                    // So we must select 3 different nodes just in case
                    const auto* trie = m_ptr_index->get_trie(m_trie_i);
                    auto pos = m_status[m_nfixed].beg;
                    m_status[m_nfixed].it[0] = trie->nodeselect(pos);
                    // WARNING
                    const auto* trie_aux = m_ptr_index->get_trie(m_trie_i+2);
                    m_status[m_nfixed].it[2] = trie_aux->nodeselect(m_status[m_nfixed].beg-1); // -1 as there is no root in this trie
                    trie_aux = m_ptr_index->get_trie(m_trie_i+3);
                    m_status[m_nfixed].it[3] = trie_aux->nodeselect(m_status[m_nfixed].beg-1);
                } else {
                    // we are in a time-first order
                    const auto* trie = m_ptr_index->get_trie(m_trie_i);
                    m_status[m_nfixed].it[0] = trie->get_last_update_of_interval(m_status[m_nfixed].beg);
                    auto node = trie->get_temporal_root();
                    m_status[m_nfixed].it[1] = node.first; 
                    m_status[m_nfixed].it[2] = node.second;
                    m_status[m_nfixed].it[3] = 0; 

                    m_below_time_level = true;
                }
            } else if (!m_below_time_level) { 
            
                if (m_nfixed == 2) {
                    // We are instantiating the second component in a regular trie
                    // m_switch indicates whether trie switching is needed or not
                    // m_status_i indicates whether we must use the meta trie or not
                    if (m_switch) { 
                        // We should be in SO, ST, PS, PT, OP, OT, so trie switch
                        size_type switch_node = trie_switch_down(m_nfixed, state);
                        //m_status[m_nfixed].it[m_status_i] = switch_node;
                    } else {  
                        // We are in SP, PO, or OS, no trie switch but we are moving to nodes
                        // in the metatrie that have 2 children, need to store them both for parent()
                        const auto* trie = m_ptr_index->get_trie(m_trie_i);
                        auto pos = m_status[m_nfixed].beg;
                        m_status[m_nfixed].it[0] = trie->nodeselect(pos);
                        // WARNING
                        const auto* trie_aux = m_ptr_index->get_trie(m_trie_i+1); // in al 3 cases the trie is the following one
                        m_status[m_nfixed].it[1] = trie_aux->nodeselect(m_status[m_nfixed].beg-trie->root_degree()-1); 
                    }
                }
                else if (m_nfixed == 3) {
                    // still on a regular trie, with 3 fixed components
                    if (state == t) {
                        // SPT, PST, POT, OPT, OST, SOT
                        // in all cases, trie switching is needed to a temporal (time first) data structure
                        size_type switch_node = trie_switch_down(m_nfixed, state);
                    } else {
                        // 3 components do not contain T, just S, P, O.
                        if (m_switch) {
                            // SOP, POS, OSP, OPS
                            size_type switch_node = trie_switch_down(m_nfixed, state);
                            m_status[m_nfixed].it[m_status_i] = switch_node;
                        } else {
                            // SPO, PSO
                            // we are moving to a node of degree 0 in the metatrie, no need to store multiple nodes in "it"
                            const auto* trie = m_ptr_index->get_trie(m_trie_i);
                            auto pos = m_status[m_nfixed].beg;
                            m_status[m_nfixed].it[0] = trie->nodeselect(pos);
                        }
                    }
                }
            } else {
                // We are in a temporal data structure, either because of the order starts by T, or we
                // have switched to an order starting with T
                if (m_switch) {
                    size_type switch_node = trie_switch_down(m_nfixed, state);
                } else {
                    const auto* trie = m_ptr_index->get_trie(m_trie_i);
                    size_type beg = m_status[m_nfixed].beg;
                    std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].cnt, m_status[m_nfixed].end);
                    std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                    value_type succ = trie->temporal_successor(m_nfixed-2, cur_node, beg, m_path_label[m_nfixed-1], node_pair);
                    m_status[m_nfixed].it[0] = node_pair.second; 
                    m_status[m_nfixed].it[1] = node_pair.first.first; 
                    m_status[m_nfixed].it[2] = node_pair.first.second; 
                    m_status[m_nfixed].it[3] = 0;
                }
            }
        }

        void leap_done() {
            m_redo[m_nfixed] = true;
        }

        void down(var_type var, value_type c) { //Go down in the trie
            m_path_label[m_nfixed] = c; // keep the current path label

            state_type state;
            if (is_variable_subject(var)) {
                state = s;
            } else if (is_variable_predicate(var)) {
                state = p;
            } else if (is_variable_object(var)) {
                state = o;
            } else {
                state = t;
            }

            down(state);
        };

        //Reverses the intervals and variable weights. Also resets the current value.
        void up(var_type var){ //Go up in the trie
            if (m_nfixed > 0 && at_time_level())
                m_below_time_level = false; // if after going up we are at time level, then we are not below it anymore
            --m_nfixed;
            m_trie_i = m_status[m_nfixed].trie; 
        };
        
        std::pair<value_type, value_type> leap(var_type var, size_type c = -1ULL) { //Return the minimum in the range
            //If c=-1 we need to get the minimum value for the current level.

            bool below_time_level_leap = at_time_level() || m_below_time_level;

            state_type state = o;
            if (is_variable_subject(var)) {
                state = s;
            } else if (is_variable_predicate(var)) {
                state = p;
            } else if (is_variable_time(var)) {
                state = t;
            }

            size_type prev_trie = m_trie_i;
            size_type t_i = m_trie_i;

            choose_trie_leap(state, t_i, m_status_i);            
            const auto* trie = m_ptr_index->get_trie(t_i);

            size_type beg, end;
            if(m_redo[m_nfixed]) { //First time of leap
                m_status[m_nfixed].trie = prev_trie; // saves the trie at previous level
                if (m_nfixed == 0 && state == t) {
                    // time first order, so there is no "trie" topology in this case, cannot apply children or first_child
                    // as in regular tries. In this case, the full interval list corresponds to the root of the time-first tries
                    beg = 0;
                    end = trie->root_degree()-1;
                    m_status[m_nfixed+1].beg = beg;
                    m_status[m_nfixed+1].end = end;
                    m_status[m_nfixed+1].cnt = end-beg+1;
                    m_redo[m_nfixed] = false;
                } else if (!below_time_level_leap) {
                    // we are in a regular trie
                    // WARNING: time levels that are not time-first orders must pass over here
                    auto cnt = trie->children(parent());
                    beg = trie->first_child(parent());
                    end = beg + cnt - 1;
                    m_status[m_nfixed+1].beg = beg;
                    m_status[m_nfixed+1].end = end;
                    m_status[m_nfixed+1].cnt = cnt;
                    m_redo[m_nfixed] = false;
                } else {
                    // we are in a temporal data structure, m_nfixed >= 1 
                    if (m_nfixed == 1) {
                        // first level of the temporal data structure
                        beg = m_status[m_nfixed].it[0]; 
                        auto cnt = m_status[m_nfixed].it[1]; 
                        end = m_status[m_nfixed].it[2];
                        m_status[m_nfixed+1].beg = beg;
                        m_status[m_nfixed+1].end = end;
                        m_status[m_nfixed+1].cnt = cnt;
                        m_redo[m_nfixed] = false;
                    } else {
                        // In a temporal data structure, at level > 1 of it 
                        beg = m_status[m_nfixed].it[0];
                        auto cnt = m_status[m_nfixed].it[1]; 
                        end = m_status[m_nfixed].it[2];
                        m_status[m_nfixed+1].beg = beg;
                        m_status[m_nfixed+1].end = end;
                        m_status[m_nfixed+1].cnt = cnt;
                        m_redo[m_nfixed] = false;
                    }
                }
            } else {
                beg = m_status[m_nfixed+1].beg;
                end = m_status[m_nfixed+1].end;
            }
            size_type value;
            if (c == -1ULL) {
                if (below_time_level_leap) {
                    std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].it[1], m_status[m_nfixed].it[2]);
                    std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                    // Now it looks for the successor of 0 in the current node
                    uint64_t init_value =  m_status[m_nfixed].it[3]; 
                    uint64_t s = 0;
                    if (m_nfixed == 2 && (t_i == TOP || t_i == TPS || t_i == TSO))
                        s = 1;

                    value = trie->temporal_successor(m_nfixed-1-s, cur_node, beg, init_value, node_pair);
                    m_status[m_nfixed].it[3] = value+1;
                } else if (state == t) {
                    auto value = trie->get_interval_at_pos(beg);
                    m_status[m_nfixed+1].beg = beg;
                    return value; // returns the interval
                } else {
                    value = trie->seq[beg];
                    m_status[m_nfixed+1].beg = beg; //First position in the sequence
                }
            } else {
                if (below_time_level_leap) {
                    std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].it[1], m_status[m_nfixed].it[2]);
                    std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                    uint64_t s = 0;
                    if (m_nfixed == 2 && (t_i == TOP || t_i == TPS || t_i == TSO))
                        s = 1;
                    // Now it looks for the successor of c in the current node
                    value = trie->temporal_successor(m_nfixed-1-s, cur_node, beg, c, node_pair);
                    if (value == INFTY) value = 0;
                } else if (state == t) {
                    auto p = trie->binary_search_interval_seek(c, beg, end);
                    if (p.second > end) return make_pair(0, 0); 
                    m_status[m_nfixed+1].beg = p.second;
                    return p.first;
                } else {
                    const auto p  = trie->binary_search_seek(c, beg, end);
                    if(p.second > end) return make_pair(0, 0);
                    value = p.first;
                    m_status[m_nfixed+1].beg = p.second; //Position of the first value gt c
                }
            }

            return make_pair(value, value);
        }


        std::pair<value_type, value_type> leap_constant(state_type state, size_type c = -1ULL) { //Return the minimum in the range
            //If c=-1 we need to get the minimum value for the current level.
            
            bool below_time_level_leap = at_time_level() || m_below_time_level;

            size_type cnt;
            size_type prev_trie = m_trie_i;
            size_type t_i = m_trie_i;

            choose_trie_leap(state, t_i, m_status_i);
            const auto* trie = m_ptr_index->get_trie(t_i);

            size_type beg, end;
            if(m_redo[m_nfixed]) { //First time of leap
                m_status[m_nfixed].trie = prev_trie; // saves the trie at previous level
                if (m_nfixed == 0 && state == t) {
                    // time first order, so there is no "trie" topology in this case, cannot apply children or first_child
                    // as in regular tries. In this case, the full interval list corresponds to the root of the time-first tries
                    beg = 0;
                    end = trie->root_degree()-1;
                    m_status[m_nfixed+1].beg = beg;
                    m_status[m_nfixed+1].end = end;
                    m_status[m_nfixed+1].cnt = end-beg+1;
                    m_redo[m_nfixed] = false;
                } else if (!below_time_level_leap) {
                    // we are in a regular trie
                    // WARNING: time levels that are not time-first orders must pass over here
                    cnt = trie->children(parent());
                    beg = trie->first_child(parent());
                    end = beg + cnt - 1;
                    m_status[m_nfixed+1].beg = beg;
                    m_status[m_nfixed+1].end = end;
                    m_status[m_nfixed+1].cnt = cnt;
                    m_redo[m_nfixed] = false;
                } else {
                    // we are in a temporal data structure, m_nfixed >= 1 
                    if (m_nfixed == 1) {
                        // first level of the temporal data structure
                        beg = m_status[m_nfixed].it[0];  
                        auto cnt = m_status[m_nfixed].it[1]; 
                        end = m_status[m_nfixed].it[2];
                        m_status[m_nfixed+1].beg = beg;
                        m_status[m_nfixed+1].end = end;
                        m_status[m_nfixed+1].cnt = cnt;
                        m_redo[m_nfixed] = false;
                    } else {
                        // In the temporal data structure, at level > 1 
                        beg = m_status[m_nfixed].it[0];
                        auto cnt = m_status[m_nfixed].it[1]; 
                        end = m_status[m_nfixed].it[2];
                        m_status[m_nfixed+1].beg = beg;
                        m_status[m_nfixed+1].end = end;
                        m_status[m_nfixed+1].cnt = cnt;
                        m_redo[m_nfixed] = false;
                    }
                }
            } else {
                beg = m_status[m_nfixed+1].beg;
                end = m_status[m_nfixed+1].end;
            }
            size_type value;
            if (c == -1ULL) {
                if (below_time_level_leap) {
                    std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].it[1], m_status[m_nfixed].it[2]);
                    std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                    // Now it looks for the successor of 0 in the current node
                    uint64_t init_value =  m_status[m_nfixed].it[3]; 
                    value = trie->temporal_successor(m_nfixed-1, cur_node, beg, init_value, node_pair);
                    m_status[m_nfixed].it[3] = value + 1;           
                } else if (state == t) {
                    auto value = trie->get_interval_at_pos(beg);
                    m_status[m_nfixed+1].beg = beg;
                    return value; // returns the interval
                } else {
                    value = trie->seq[beg];
                    m_status[m_nfixed+1].beg = beg; //First position in the sequence
                }
            } else {
                if (m_below_time_level) {
                    std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].it[1], m_status[m_nfixed].it[2]);
                    std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                    // Now it looks for the successor of c in the current node
                    value = trie->temporal_successor(m_nfixed-1, cur_node, beg, c, node_pair);
                    if (value == INFTY) value = 0;
                } else if (state == t) {
                    auto p = trie->binary_search_interval_seek(c, beg, end);
                    if (p.second > end) return make_pair(0, 0); // [0,0) indicates failure
                    m_status[m_nfixed+1].beg = p.second;
                    return p.first;
                } else {
                    const auto p  = trie->binary_search_seek(c, beg, end);
                    if(p.second > end) return make_pair(0, 0);
                    value = p.first;
                    m_status[m_nfixed+1].beg = p.second; //Position of the first value gt c
                }
            }

            return make_pair(value, value);
        }


        bool in_last_level() {
            return m_nfixed == 3;
        }

        bool at_time_level() {
            if (m_nfixed == 0) return false;
            return m_fixed[m_nfixed-1] == t;
        }

        inline size_type children(state_type state) const { 
            size_type t_i, s_i=0;
            //if (state == t) return 0;             
            t_i = m_trie_i;
            choose_trie_leap(state, t_i, s_i);
            
            if (t_i < TSPO) {
                auto trie = m_ptr_index->get_trie(t_i);
                auto it = m_status[m_nfixed].it[s_i];
                return trie->children(it);
            }
            else {
                if (m_nfixed == 0) {
                    auto trie = m_ptr_index->get_trie(SPOT);
                    return trie->number_of_updates(); //trie->root_degree();
                } else {
                    auto trie = m_ptr_index->get_trie(t_i);
                    size_type beg = m_status[m_nfixed].it[0];
                    std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].it[1], m_status[m_nfixed].it[2]);                
                    size_type d = trie->temporal_ds_node_degree(m_nfixed-1, beg, cur_node);
                    return d;
                }
            }
        }

        inline size_type subtree_size_fixed1(state_type state) const {
            size_type t_i, s_i;
            if (state == s) { //Fix variables
                t_i = (m_fixed[m_nfixed-1] == o) ? 4 : 3 ;
                s_i = (m_fixed[m_nfixed-1] == o) ? 0 : 1;
            } else if (state == p) {
                t_i = (m_fixed[m_nfixed-1] == s) ? 0 : 5 ;
                s_i = (m_fixed[m_nfixed-1] == s) ? 0 : 1;
            } else {
                t_i = (m_fixed[m_nfixed - 1] == p) ? 2 : 1;
                s_i = (m_fixed[m_nfixed - 1] == p) ? 0 : 1;
            }

            size_type leftmost_leaf, rightmost_leaf;
            const auto* trie =  m_ptr_index->get_trie(t_i);
            if (s_i == 0) {
                auto it = m_status[m_nfixed].it[s_i];
                //Count children
                auto cnt = trie->children(it);
                //Leftmost
                auto first = trie->child(it, 1);
                leftmost_leaf = trie->first_child(first);
                //Rightmost
                it = trie->child(it, cnt);
                cnt = trie->children(it);
                rightmost_leaf = trie->first_child(it) + cnt - 1;
            } else {
                const auto* trie_aux =  m_ptr_index->get_trie(t_i-1);
                auto it = m_status[m_nfixed].it[0];
                    //Count children
                auto cnt = trie_aux->children(it);
                    //Leftmost
                auto first = trie_aux->child(it, 1);
                leftmost_leaf = trie_aux->first_child(first);
                    //Rightmost
                it = trie_aux->child(it, cnt);
                cnt = trie_aux->children(it);
                rightmost_leaf = trie_aux->first_child(it) + cnt - 1;
            }
            return rightmost_leaf - leftmost_leaf + 1;
        }

        inline size_type subtree_size_fixed2() const {
            const auto* trie =  m_ptr_index->get_trie(m_trie_i);
            if (m_nfixed == 2 && m_status_i == 1) {
                switch (m_trie_i) {
                    case 1: trie = m_ptr_index->get_trie(4);  // switches SOP -> OSP
                            break;
                    case 3: trie = m_ptr_index->get_trie(0);  // switches PSO -> SPO
                            break;
                    case 5: trie = m_ptr_index->get_trie(2);  // switches OPS -> POS
                            break;
                }
            }

            auto it = m_status[m_nfixed].it[m_status_i];
            return trie->children(it);
        }

        std::vector<uint64_t> seek_all(var_type x_j){
            std::vector<uint64_t> results;
            
            const auto* trie = m_ptr_index->get_trie(m_trie_i);

            if (m_trie_i < TSPO) {
                uint32_t cnt = trie->children(parent());
                size_type beg = trie->first_child(parent());

                if (is_variable_time(x_j)) {
                    for(auto i = beg; i < beg + cnt; ++i) {
                        results.emplace_back(trie->get_interval_at_pos(i).first);
                    }
                } else {
                    for(auto i = beg; i < beg + cnt; ++i) {
                        results.emplace_back(trie->seq[i]);
                    }
                }
            } else {
                size_type beg = m_status[m_nfixed].it[0];
                std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].it[1], m_status[m_nfixed].it[2]);
                std::pair<std::pair<size_type, size_type>, size_type> node_pair;                
                value_type succ = trie->temporal_successor(m_nfixed-1, cur_node, beg, 0, node_pair);
                int i = 0;
                while (succ != INFTY) {
                    results.emplace_back(succ);
                    succ = trie->temporal_successor(m_nfixed-1, cur_node, beg, succ+1, node_pair);
                    i++;
                }
            }

            return results;
        }
   
    };

    template<class index_scheme_t, class var_t, class cons_t>
    uint64_t ltj_iterator_v2<index_scheme_t, var_t, cons_t>::parent() const {
        return m_status[m_nfixed].it[m_status_i];
    }

}

#endif //LTJ_ITERATOR_HPP
