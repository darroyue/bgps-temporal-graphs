#ifndef CLTJ_COMPACT_TRIE_V3_H
#define CLTJ_COMPACT_TRIE_V3_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <sdsl/vectors.hpp>
#include <sdsl/select_support_mcl.hpp>
#include <cltj_regular_trie_v2.hpp>
#include <succ_support_v.hpp>
#include <cltj_config.hpp>

namespace cltj {

    class compact_trie_v3 {


    public:

        typedef uint64_t size_type;
        typedef uint32_t value_type;

    private:
        sdsl::bit_vector m_bv;
        sdsl::int_vector<> m_seq;
        //sdsl::rank_support_v<1> m_rank1;
        cds::succ_support_v<0> m_succ0;
        sdsl::select_support_mcl<0> m_select0;

        uint8_t m_height; // height of the trie         

        void copy(const compact_trie_v3 &o) {
            m_bv = o.m_bv;
            m_seq = o.m_seq;
            //m_rank1 = o.m_rank1;
            //m_rank1.set_vector(&m_bv);
            m_succ0 = o.m_succ0;
            m_succ0.set_vector(&m_bv);
            m_select0 = o.m_select0;
            m_select0.set_vector(&m_bv);
        }

        /*inline size_type rank0(const size_type i) const {
            return i - m_rank1(i);
        }*/

    public:

        const sdsl::int_vector<> &seq = m_seq;

        compact_trie_v3() = default;

        compact_trie_v3(/*const TrieV2 *trie, const uint64_t n_nodes*/
                        const std::vector<spo_triple> &D, uint8_t order[], uint64_t n_levels) {

            uint64_t c0 = 1;
	    std::vector<uint64_t> v0;

            for (uint64_t i = 1; i < D.size(); i++) {
	        if (D[i][order[0]] != D[i-1][order[0]]) {
		    v0.push_back(c0);
		    c0 = 1;
		} else c0++;
	    }
	    v0.push_back(c0);

            
            uint64_t c1, c2;
            std::vector<uint64_t> v1, v2;
            std::vector<uint64_t> v_seq_l1;            

            for (uint64_t i = 0, k = 0; i < v0.size(); i++) {
	        c1 = 1;
		c2 = 1;
	        for (uint64_t j = 1; j < v0[i]; j++) {
		    k++;
		    if (D[k][order[1]] != D[k-1][order[1]]) {
		        if (n_levels >= 2) v2.push_back(c2);
                        v_seq_l1.push_back(D[k-1][order[1]]);
			c2 = 1;
			c1++;
		    } else c2++;
		}
                v_seq_l1.push_back(D[k][order[1]]);
		if (n_levels >= 2) v2.push_back(c2);
		v1.push_back(c1);
                k++;
	    } 

            /*std::cout << "Mostrando los vectores en compact_trie_v3" << std::endl;
            std::cout << "v0: " << std::endl;
            for (uint64_t i = 0; i < v0.size(); i++)
                std::cout << v0[i] << ", " << std::endl;

            std::cout << "v1: " << std::endl;
            for (uint64_t i = 0; i < v1.size(); i++)
                std::cout << v1[i] << ", " << std::endl;

            std::cout << "v2: " << std::endl;
            for (uint64_t i = 0; i < v2.size(); i++)
                std::cout << v2[i] << ", " << std::endl;
            */

            uint64_t c = 0;
            for (uint64_t i = 0; i < v1.size(); i++)
                c += v1[i];

            c += v1.size();
            
            if (n_levels >= 2) {
                for (uint64_t i = 0; i < v2.size(); i++)
                    c += v2[i];
                
                c += v2.size();

                m_bv = sdsl::bit_vector(c, 1);
                m_seq = sdsl::int_vector<>(v_seq_l1.size()+D.size());
                uint64_t j =0;

                for (uint64_t i = 0; i < v1.size(); i++) {
                    j += v1[i];
                    m_bv[j++] = 0;
                }
 
                for (uint64_t i = 0; i < v2.size(); i++) {
                    j += v2[i];
                    m_bv[j++] = 0;
                }
          
                for (j = 0; j < v_seq_l1.size(); j++)
                    m_seq[j] = v_seq_l1[j]; 
 
                for (uint64_t i = 0; i < D.size(); i++)
                    m_seq[j++] = D[i][order[2]];

                /*std::cout << "m_bv: ";
                for (uint64_t i = 0; i < m_bv.size(); i++)
                    std::cout << m_bv[i];
                std::cout << std::endl;

                std::cout << "m_seq:" << std::endl;
                for (uint64_t i = 0; i < m_seq.size(); i++)
                    std::cout << m_seq[i] << std::endl; 
                */
                sdsl::util::bit_compress(m_seq);
            } else {
                m_bv = sdsl::bit_vector(c, 1);
                m_seq = sdsl::int_vector<>(v_seq_l1.size());
                uint64_t j =0;

                for (uint64_t i = 0; i < v1.size(); i++) {
                    j += v1[i];
                    m_bv[j++] = 0;
                }

                for (j = 0; j < v_seq_l1.size(); j++)
                    m_seq[j] = v_seq_l1[j];

                /*std::cout << "m_bv: ";
                for (uint64_t i = 0; i < m_bv.size(); i++)
                    std::cout << m_bv[i];
                std::cout << std::endl;

                std::cout << "m_seq:" << std::endl;
                for (uint64_t i = 0; i < m_seq.size(); i++)
                    std::cout << m_seq[i] << std::endl;
                */

                sdsl::util::bit_compress(m_seq);
            }

            sdsl::util::init_support(m_succ0, &m_bv);
            sdsl::util::init_support(m_select0, &m_bv);

        }

        //! Copy constructor
        compact_trie_v3(const compact_trie_v3 &o) {
            copy(o);
        }

        //! Move constructor
        compact_trie_v3(compact_trie_v3 &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        compact_trie_v3 &operator=(const compact_trie_v3 &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        compact_trie_v3 &operator=(compact_trie_v3 &&o) {
            if (this != &o) {
                m_bv = std::move(o.m_bv);
                m_seq = std::move(o.m_seq);
                //m_rank1 = std::move(o.m_rank1);
                m_succ0 = std::move(o.m_succ0);
                //m_rank1.set_vector(&m_bv);
                m_succ0.set_vector(&m_bv);
                m_select0 = std::move(o.m_select0);
                m_select0.set_vector(&m_bv);
            }
            return *this;
        }

        void swap(compact_trie_v3 &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_bv, o.m_bv);
            std::swap(m_seq, o.m_seq);
            sdsl::util::swap_support(m_succ0, o.m_succ0, &m_bv, &o.m_bv);
            sdsl::util::swap_support(m_select0, o.m_select0, &m_bv, &o.m_bv);
        }


        /*
            Receives index in bit vector
            Returns index of next 0
        */
        /*inline uint32_t succ0(uint32_t it) const{
            return m_select0(rank0(it) + 1);
        }*/

        /*
            Receives index of current node and the child that is required
            Returns index of the nth child of current node
        */
        inline size_type child(uint32_t it, uint32_t n) const {
            return m_select0(it + 1 + n);
        }

        /*
            Receives index of node whos children we want to count
            Returns how many children said node has
        */
        size_type children(size_type i) const {
            return m_succ0(i + 1) - i;
        }

        size_type first_child(size_type i) const {
            return i;
        }

        inline size_type nodeselect(size_type i) const {
            return m_select0(i + 2);
        }


        pair<uint32_t, uint32_t> binary_search_seek(uint32_t val, uint32_t i, uint32_t f) const {
            if (m_seq[f] < val) return make_pair(0, f + 1);
            uint32_t mid;
            while (i < f) {
                mid = (i + f) / 2;
                if (m_seq[mid] < val) {
                    i = mid + 1;
                } else {
                    f = mid;
                }
            }
            return make_pair(m_seq[i], i);
        }


        void print() const {
            for (auto i = 0; i < m_bv.size(); ++i) {
                std::cout << (uint) m_bv[i];
            }
            std::cout << std::endl;
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name = "") const {
            sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_bv.serialize(out, child, "bv");
            written_bytes += m_seq.serialize(out, child, "seq");
            written_bytes += m_succ0.serialize(out, child, "succ0");
            written_bytes += m_select0.serialize(out, child, "select0");
            sdsl::structure_tree::add_size(child, written_bytes);
            // OJO DIEGO: falta escribir aqui la altura del trie
            return written_bytes;
        }

        void load(std::istream &in) {
            m_bv.load(in);
            m_seq.load(in);
            m_succ0.load(in, &m_bv);
            m_select0.load(in, &m_bv);
            // OJO DIEGO: falta leer la altura del trie
        }

    };
}
#endif
