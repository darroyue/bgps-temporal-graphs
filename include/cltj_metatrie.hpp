
#ifndef CLTJ_METATRIE_HPP
#define CLTJ_METATRIE_HPP

#include <algorithm>
//#include <configuration.hpp>
#include <cltj_compact_trie_v3.hpp>
#include <cltj_regular_trie_v2.hpp>
#include <cltj_config.hpp>

namespace cltj {

    class cltj_metatrie {

    public:

        typedef uint64_t size_type;
        typedef uint32_t value_type;

    private:

        sdsl::int_vector<>    m_root_seq;   // note the BV of the first level of the trie does not need to be stored
        cltj::compact_trie_v3 m_left_child;
        cltj::compact_trie_v3 m_right_child;
        uint8_t               m_first_level_column;   // column corresponding to the first level of the meta trie: S=0,P=1,O=2


        void copy(const cltj_metatrie &o) {
            m_root_seq = o.m_root_seq;
            m_left_child = o.m_left_child;
	    m_right_child = o.m_right_child;
	    m_first_level_column = o.m_first_level_column;
        }


    public:
    
        cltj_metatrie() = default;
  
        cltj_metatrie(/*const TrieV2* trie1, const TrieV2* trie2, uint64_t n_nodes,*/
                      std::vector<spo_triple> &D, uint8_t order1/*[]*/, uint8_t order2/*[]*/) {

            std::cout << "Building order: " << (int)order1 << std::endl;//[0] << " " << (int)order1[1] << " " << (int)order1[2] << std::endl;
            std::cout << " ------------" << std::endl;
            fflush(stdout);

            // Sorts according to order1
            /*sort(D.begin(), D.end(), [order1, order2](const spo_triple& a,
                 const spo_triple& b) {return a[order1[2]] < b[order1[2]];});

            stable_sort(D.begin(), D.end(), [order1, order2](const spo_triple& a,
                 const spo_triple& b) {return a[order1[1]] < b[order1[1]];});

            stable_sort(D.begin(), D.end(), [order1, order2](const spo_triple& a,
                 const spo_triple& b) {return a[order1[0]] < b[order1[0]];});            
            */

            std::sort(D.begin(), D.end(), comparator_order(order1));

            /*for (uint64_t i = 0; i < D.size(); i++) 
                std::cout << D[i][spo_orders[order1][0]] << ", " << D[i][spo_orders[order1][1]] << ", " << D[i][spo_orders[order1][2]] << std::endl; 
            */

            m_left_child = compact_trie_v3(D, spo_orders[order1], 2);

            // now builds the metatrie root
            uint64_t c = 1;
            for (uint64_t i = 1; i < D.size(); i++)
                if (D[i][spo_orders[order1][0]] != D[i-1][spo_orders[order1][0]]) 
                    c++;

            m_root_seq = sdsl::int_vector<>(c);

            c = 0;
            for (uint64_t i = 1; i < D.size(); i++)
                if (D[i][spo_orders[order1][0]] != D[i-1][spo_orders[order1][0]]) 
                    m_root_seq[c++] = D[i-1][spo_orders[order1][0]];

            m_root_seq[c++] = D[D.size()-1][spo_orders[order1][0]];

            /*std::cout << "m_root_seq:" << std::endl;
            for (uint64_t i = 0; i < m_root_seq.size(); i++)
                std::cout << m_root_seq[i] << endl;
            */
            sdsl::util::bit_compress(m_root_seq);

            // Now sorts according to order2
            std::cout << "Building order: " << (int)order2 << std::endl;//[0] << " " << (int)order2[1] << " " << (int)order2[2] << std::endl;
            std::cout << " ------------" << std::endl;
            /*sort(D.begin(), D.end(), [order1, order2](const spo_triple& a,
                 const spo_triple& b) {return a[order2[2]] < b[order2[2]];});

            stable_sort(D.begin(), D.end(), [order1, order2](const spo_triple& a,
                 const spo_triple& b) {return a[order2[1]] < b[order2[1]];});

            stable_sort(D.begin(), D.end(), [order1, order2](const spo_triple& a,
                 const spo_triple& b) {return a[order2[0]] < b[order2[0]];});
            */
            std::sort(D.begin(), D.end(), comparator_order(order2));

            /*for (uint64_t i = 0; i < D.size(); i++)
                std::cout << D[i][spo_orders[order2][0]] << ", " << D[i][spo_orders[order2][1]] << ", " << D[i][spo_orders[order2][2]] << std::endl;
            */
            m_right_child = compact_trie_v3(D, spo_orders[order2], 1);

            //getchar();
        }

        //! Copy constructor
        cltj_metatrie(const cltj_metatrie &o) {
            copy(o);
        }

        //! Move constructor
        cltj_metatrie(cltj_metatrie &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        cltj_metatrie &operator=(const cltj_metatrie &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        cltj_metatrie &operator=(cltj_metatrie &&o) {
            if (this != &o) {
                m_root_seq = std::move(o.m_root_seq);
                m_left_child = std::move(o.m_left_child);
                m_right_child = std::move(o.m_right_child);
                m_first_level_column = std::move(o.m_first_level_column);
            }
            return *this;
        }

        void swap(cltj_metatrie &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_root_seq, o.m_root_seq);
            std::swap(m_left_child, o.m_left_child);
            std::swap(m_right_child, o.m_right_child);
            std::swap(m_first_level_column, o.m_first_level_column);
        }

        size_type serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name = "") const {
            sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_root_seq.serialize(out, child, "root_seq");
            written_bytes += m_left_child.serialize(out, child, "left_child");
            written_bytes += m_right_child.serialize(out, child, "right_child");
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream &in) {
            m_root_seq.load(in);
            m_left_child.load(in);
            m_right_child.load(in);
        }

    };
}
#endif
