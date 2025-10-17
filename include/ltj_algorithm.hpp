/*
 * ltj_algorithm.hpp
 * Copyright (C) 2020 Author removed for double-blind evaluation
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



#ifndef TEMPORAL_LTJ_ALGORITHM_HPP
#define TEMPORAL_LTJ_ALGORITHM_HPP


#include <triple_pattern.hpp>
#include <quad_pattern.hpp>
#include <ltj_iterator_v2.hpp>
#include <veo_simple.hpp>
#include <veo_adaptive.hpp>
#include <results_collector.hpp>

#define EXPT_TIME_SOL 1

namespace ltj {

    template<class iterator_t = ltj_iterator_v2<cltj::compact_ltj, uint8_t, uint64_t>,
             class veo_t = veo::veo_adaptive<iterator_t, util::trait_size> >
    class ltj_algorithm {

    public:
        typedef uint64_t value_type;
        typedef uint64_t size_type;
        typedef iterator_t ltj_iter_type;
        typedef typename ltj_iter_type::var_type var_type;
        typedef typename ltj_iter_type::index_scheme_type index_scheme_type;
        typedef typename ltj_iter_type::value_type const_type;
        typedef veo_t veo_type;
        typedef unordered_map<var_type, vector<ltj_iter_type*>> var_to_iterators_type;
        typedef vector<pair<var_type, value_type>> tuple_type;
        typedef chrono::high_resolution_clock::time_point time_point_type;
        typedef ::util::results_collector<tuple_type> results_type;

    private:
        //const vector<triple_pattern>* m_ptr_triple_patterns;
        const vector<quad_pattern>* m_ptr_quad_patterns;
        veo_type m_veo;
        index_scheme_type* m_ptr_ring;
        vector<ltj_iter_type> m_iterators;
        var_to_iterators_type m_var_to_iterators;
        bool m_is_empty = false;


        void copy(const ltj_algorithm &o) {
            m_ptr_quad_patterns = o.m_ptr_quad_patterns;
            m_veo = o.m_veo;
            m_ptr_ring = o.m_ptr_ring;
            m_iterators = o.m_iterators;
            m_var_to_iterators = o.m_var_to_iterators;
            m_is_empty = o.m_is_empty;
        }


        inline void add_var_to_iterator(const var_type var, ltj_iter_type* ptr_iterator){
            auto it =  m_var_to_iterators.find(var);
            if (it != m_var_to_iterators.end()) {
                it->second.push_back(ptr_iterator);
            } else {
                vector<ltj_iter_type*> vec = {ptr_iterator};
                m_var_to_iterators.insert({var, vec});
            }
        }

    public:


        ltj_algorithm() = default;

        ltj_algorithm(const vector<quad_pattern>* quad_patterns, index_scheme_type* ring){

            m_ptr_quad_patterns = quad_patterns;
            m_ptr_ring = ring;

            size_type i = 0;
            m_iterators.reserve(m_ptr_quad_patterns->size());
            for(const auto& quad : *m_ptr_quad_patterns){
                //Bulding iterators
                m_iterators.emplace_back(ltj_iter_type(&quad, m_ptr_ring));
                if(m_iterators[i].is_empty()){
                    m_is_empty = true;
                    return;
                }

                //For each variable we add the pointers to its iterators
                if(quad.o_is_variable()) {
                    add_var_to_iterator(quad.term_o.value, &(m_iterators[i]));
                }
                if(quad.p_is_variable()) {
                    add_var_to_iterator(quad.term_p.value, &(m_iterators[i]));
                }
                if(quad.s_is_variable()) {
                    add_var_to_iterator(quad.term_s.value, &(m_iterators[i]));
                }
                if(quad.t_is_variable()) {
                    add_var_to_iterator(quad.term_t.value, &(m_iterators[i]));
                }
                ++i;
            }

            m_veo = veo_type(m_ptr_quad_patterns, &m_iterators, &m_var_to_iterators, m_ptr_ring);

        }

        //! Copy constructor
        ltj_algorithm(const ltj_algorithm &o) {
            copy(o);
        }

        //! Move constructor
        ltj_algorithm(ltj_algorithm &&o) {
            *this = move(o);
        }

        //! Copy Operator=
        ltj_algorithm &operator=(const ltj_algorithm &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        ltj_algorithm &operator=(ltj_algorithm &&o) {
            if (this != &o) {
                m_ptr_quad_patterns = move(o.m_ptr_quad_patterns);
                m_veo = move(o.m_veo);
                m_ptr_ring = move(o.m_ptr_ring);
                m_iterators = move(o.m_iterators);
                m_var_to_iterators = move(o.m_var_to_iterators);
                m_is_empty = o.m_is_empty;
            }
            return *this;
        }

        void swap(ltj_algorithm &o) {
            std::swap(m_ptr_quad_patterns, o.m_ptr_quad_patterns);
            std::swap(m_veo, o.m_veo);
            std::swap(m_ptr_ring, o.m_ptr_ring);
            std::swap(m_iterators, o.m_iterators);
            std::swap(m_var_to_iterators, o.m_var_to_iterators);
            std::swap(m_is_empty, o.m_is_empty);
        }


        /**
        *
        * @param res               Results
        * @param limit_results     Limit of results
        * @param timeout_seconds   Timeout in seconds
        */
        void join(/*vector<tuple_type> &res,*/
                  results_type &res,
                  const size_type limit_results = 0, const size_type timeout_seconds = 0){
            if(m_is_empty) return;
            time_point_type start = chrono::high_resolution_clock::now();
            tuple_type t(m_veo.size());
            search(0, t, res, start, limit_results, timeout_seconds);
        };


        /**
         *
         * @param j                 Index of the variable
         * @param tuple             Tuple of the current search
         * @param res               Results
         * @param start             Initial time to check timeout
         * @param limit_results     Limit of results
         * @param timeout_seconds   Timeout in seconds
         */
        bool search(const size_type j, tuple_type &tuple, results_type &res,
                    const time_point_type start,
                    const size_type limit_results = 0, const size_type timeout_seconds = 0){

            //(Optional) Check timeout
            if(timeout_seconds > 0){
                time_point_type stop = chrono::high_resolution_clock::now();
                auto sec = chrono::duration_cast<chrono::seconds>(stop-start).count();
                if(sec > timeout_seconds) {
                    return false;
                }
            }

            //(Optional) Check limit
            if(limit_results > 0 && res.size() == limit_results) {
                return false;
            }

            if(j == m_veo.size()){
                //Report results
                //res.emplace_back(tuple);
                res.add(tuple);
#if EXPT_TIME_SOL
                if(res.size() % 1000 == 0){
                    time_point_type stop = chrono::high_resolution_clock::now();
                    auto sec = chrono::duration_cast<chrono::seconds>(stop-start).count();
                    //std::cerr << res.size() << ";" << sec << std::endl;
                }
#endif
                /*cout << "Add result" << endl;
                for(const auto &dat : tuple){
                    cout << "{" << (uint64_t) dat.first << "=" << dat.second << "} ";
                }
                cout << endl;*/
            } else {
                var_type x_j = m_veo.next();
                //cout << "Variable: " << (uint64_t) x_j << endl;
                //cout << (uint64_t) x_j << endl;
                vector<ltj_iter_type*>& itrs = m_var_to_iterators[x_j];
                bool ok;
                //itrs[0]->print_path();
                if( itrs.size() == 1 && itrs[0]->in_last_level()) {//Lonely variables
                    //cout << "Seeking (last level)" << endl;
                    auto results = itrs[0]->seek_all(x_j);
                    //cout << "Results: " << results.size() << endl;
                    //cout << "Seek (last level): (" << (uint64_t) x_j << ": size=" << results.size() << ")" <<endl;
                    for (const auto &c : results) {
                        //1. Adding result to tuple
                        tuple[j] = {x_j, c};
                        //2. Going down in the trie by setting x_j = c (\mu(t_i) in paper)
                        itrs[0]->down(x_j, c);
                        m_veo.down();
                        //2. Search with the next variable x_{j+1}
                        ok = search(j + 1, tuple, res, start, limit_results, timeout_seconds);
                        if (!ok) return false;
                        //4. Going up in the trie by removing x_j = c
                        itrs[0]->up(x_j);
                        m_veo.up();
                    }
                } else {
                    // OJO aqui
                    if (itrs[0]->is_variable_time(x_j)) { 
                        std::pair<value_type, value_type> c = seek_interval(x_j);
                    
                        //cout << ">>>>>>> Seek c = [" << c.first << ", " << c.second << ")" <<endl;
                        while (c.second != 0) { //If empty c.second==0
                            //1. Adding result to tuple
                            tuple[j] = {x_j, c.first/*ojo aqui, hay que reportar el intervalo completo, por ahora solo el first */};
                            //2. Going down in the tries by setting x_j = c (\mu(t_i) in paper)
                            for (ltj_iter_type* iter : itrs) {
                                iter->down(x_j, c.second-1/*OJO, revisar aqui, cuando bajo en una componente temporal solo guardo el extremo derecho de los intervalos*/);
                            }
                            m_veo.down();
                            //3. Search with the next variable x_{j+1}
                            ok = search(j + 1, tuple, res, start, limit_results, timeout_seconds);
                            if (!ok) return false;
                            //4. Going up in the tries by removing x_j = c
                            for (ltj_iter_type *iter : itrs) {
                                iter->up(x_j);
                            }
                            m_veo.up();
                            //5. Next constant for x_j     
                            c = seek_interval(x_j, c.second/*OJO aqui, creo que no hace falta un +1*/ );
                            //c = seek(x_j, c + 1);
                            //cout << "Seek (bucle): (" << (uint64_t) x_j << ": " << c << ")" <<endl;
                        }
                    }
                    else { 
                        value_type c = seek(x_j);
                    
                        //cout << "Seek (init): (" << (uint64_t) x_j << ": " << c << ")" <<endl;
                        while (c != 0) { //If empty c=INFTY
                            //1. Adding result to tuple
                            tuple[j] = {x_j, c};
                            //2. Going down in the tries by setting x_j = c (\mu(t_i) in paper)
                            for (ltj_iter_type* iter : itrs) {
                                iter->down(x_j, c);
                            }
                            m_veo.down();
                            //3. Search with the next variable x_{j+1}
                            ok = search(j + 1, tuple, res, start, limit_results, timeout_seconds);
                            if(!ok) return false;
                            //4. Going up in the tries by removing x_j = c
                            for (ltj_iter_type *iter : itrs) {
                                iter->up(x_j);
                            }
                            m_veo.up();
                            //5. Next constant for x_j     
                            c = seek(x_j, c+1);
                            //c = seek(x_j, c + 1);
                            //cout << "Seek (bucle): (" << (uint64_t) x_j << ": " << c << ")" <<endl;
                        }
                    }
                }
                m_veo.done();
            }
            return true;
        };


        /**
         *
         * @param x_j   Variable
         * @param c     Constant. If it is unknown the value is -1
         * @return      The next constant that matches the intersection between the triples of x_j.
         *              If the intersection is empty, it returns 0.
         */

        value_type seek(const var_type x_j, value_type c=-1) {
            vector<ltj_iter_type*>& itrs = m_var_to_iterators[x_j];
            value_type c_i, c_prev = 0, i = 0, n_ok = 0;
            while (true) {
                //Compute leap for each triple that contains x_j
                //std::cout << "Leap of " << (::uint64_t) x_j << " in iterator: " << i << std::endl;
                if (c == -1) {
                    c_i = itrs[i]->leap(x_j).first;
                } else {
                    c_i = itrs[i]->leap(x_j, c).first;
                }
                //std::cout << "Gets " << (::uint64_t) c_i << std::endl;
                if (c_i == 0) {
                    for (auto &itr : itrs) {
                        itr->leap_done();
                    }
                    return 0; //Empty intersection
                }
                n_ok = (c_i == c_prev) ? n_ok + 1 : 1;
                if(n_ok == itrs.size()) return c_i;
                c = c_prev = c_i;
                i = (i+1 == itrs.size()) ? 0 : i+1;
            }
            for (auto &itr : itrs) {
                itr->leap_done();
            }
        }

        std::pair<value_type, value_type> seek_interval(const var_type x_j, value_type c=-1) {
            vector<ltj_iter_type*>& itrs = m_var_to_iterators[x_j];
            value_type l = 0, i, k = itrs.size();
            std::pair<value_type, value_type> c_i, newc_i;
            while (true) {
                //Compute leap for each triple that contains x_j
                //std::cout << "Leap of " << (::uint64_t) x_j << " in iterator: " << i << std::endl;
                if (c == -1) {
                    //cout << "en ltj::seek_interval() voy a llamar a leap del iterator" << endl;
                    c_i = itrs[l]->leap(x_j);
                } else {
                    //cout << "en ltj::seek_interval() voy a llamar a leap del iterator" << endl;
                    c_i = itrs[l]->leap(x_j, c);
                }
                //std::cout << "Gets " << (::uint64_t) c_i << std::endl;
                // leap returns an interval of the form [s, e), so e==0 means the interval is empty 
                
                if (c_i.second == 0) {
                    for (auto &itr : itrs) {
                        itr->leap_done();
                    }
                    return make_pair(0, 0); //Empty intersection
                }
                
                i = (l+1)%k;
                while (i != l) {
                    //cout << "en ltj::seek_interval() voy a llamar a leap del iterator" << endl;
                    newc_i = itrs[i]->leap(x_j, c_i.first);
                    if (newc_i.second == 0) {
                        for (auto &itr : itrs) {
                            itr->leap_done();
                        }
                        return make_pair(0, 0); //Empty intersection
                    }

                    if (newc_i.first > c_i.first) {
                        c_i = newc_i;
                        //cout << "l antes = " << l << endl;
                        l = i;
                        //cout << " l despues = " << l << endl; 
                    } else {
                        c_i.second = std::min(c_i.second, newc_i.second);
                    }

                    i = (i+1)%k;
                }
                //cout << "> ******* En seek_interval returning c_i = [" << c_i.first << ", " << c_i.second << ")" << endl;
                return c_i;
            }
            for (auto &itr : itrs) {
                itr->leap_done();
            }
        }


        /*value_type seek_interval(const var_type x_j, value_type c = -1) {
            vector<ltj_iter_type*>& itrs = m_var_to_iterators[x_j];
            size_t k = itrs.size();
            vector<pair<value_type, value_type>> intervals(k); // intervalos [s_i, e_i)
    
            // Inicializa todos los intervalos activos en torno a c
            for (size_t i = 0; i < k; ++i) {
                if (c == -1)
                    intervals[i] = itrs[i]->leap(x_j);
                else
                    intervals[i] = itrs[i]->leap(x_j, c);

                if (intervals[i].second == 0) { // sin más intervalos
                    for (auto& itr : itrs) itr->leap_done();
                    return 0;
                }
            }

            while (true) {
                value_type s_max = 0;        // inicio máximo de todos los intervalos
                value_type e_min = UINT64_MAX; // fin mínimo de todos los intervalos

                // Calcula los extremos de la intersección actual
                for (size_t i = 0; i < k; ++i) {
                    s_max = std::max(s_max, intervals[i].first);
                    e_min = std::min(e_min, intervals[i].second);
                }

                if (s_max < e_min) {
                    // Hay intersección común: el punto más bajo válido es s_max
                    for (auto& itr : itrs) itr->leap_done();
                    return s_max;
                }

                // Si no hay intersección, avanzar el conjunto cuyo intervalo termina antes
                value_type target = s_max;
                size_t i_min = 0;
                for (size_t i = 0; i < k; ++i) {
                    if (intervals[i].second == e_min) {
                        i_min = i;
                        break;
                    }
                }

                intervals[i_min] = itrs[i_min]->leap(x_j, target);
                if (intervals[i_min].second == 0) {
                    for (auto& itr : itrs) itr->leap_done();
                    return 0; // no hay más intersección posible
                }
            }

            for (auto& itr : itrs) itr->leap_done();
            return 0; // debería ser inalcanzable
        }*/




        void print_veo(unordered_map<uint8_t, string> &ht){
            cout << "veo: ";
            for (uint64_t j = 0; j < m_veo.size(); ++j) {
                cout << "?" << ht[m_veo.next()] << " ";
            }
            cout << endl;
        }

        void print_query(unordered_map<uint8_t, string> &ht) {
            cout << "Query: " << endl;
            for( size_type i = 0; i <  m_ptr_quad_patterns->size(); ++i) {
                m_ptr_quad_patterns->at(i).print(ht);
                if (i < m_ptr_quad_patterns->size()-1) {
                    cout << " . ";
                }
            }
            cout << endl;
        }

        void print_results(vector<tuple_type> &res, unordered_map<uint8_t, string> &ht) {
            cout << "Results: " << endl;
            uint64_t i = 1;
            for (tuple_type &tuple :  res) {
                cout << "[" << i << "]: ";
                for (pair<var_type, value_type> &pair : tuple) {
                    cout << "?" << ht[pair.first] << "=" << pair.second << " ";
                }
                cout << endl;
                ++i;
            }
        }


    };
}

#endif //TEMPORAL_LTJ_ALGORITHM_HPP
