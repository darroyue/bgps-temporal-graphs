//
// Created by Diego Arroyuelo on 20/09/24.
//

#ifndef CLTJ_CLTJ_INDEX_METATRIE_HPP
#define CLTJ_CLTJ_INDEX_METATRIE_HPP

//#include <cltj_compact_trie_v2.hpp>
#include <cltj_compact_trie_v3.hpp>
#include <cltj_uncompact_trie_v2.hpp>
#include <cltj_metatrie.hpp>
#include <cltj_config.hpp>

namespace cltj {

    template<class Trie>
    class cltj_index_metatrie {

    public:
        typedef uint64_t size_type;
        typedef uint32_t value_type;
        typedef Trie metatrie_type;

    private:
        std::array<metatrie_type, 3> m_tries;

        TrieV2* create_trie(const vector<spo_triple> &D, const spo_order_type &order, size_type &n_nodes){
            TrieV2* root = new TrieV2();
            TrieV2* node;
            bool ok;
            for(const auto &spo: D){
                node = root;
                for(size_type i = 0; i < 3; ++i){
                    node = node->insert(spo[order[i]], ok);
                    if(ok) ++n_nodes;
                }
            }
            return root;
        }

        void copy(const cltj_index_metatrie &o){
            m_tries = o.m_tries;
        }

    public:

        const std::array<metatrie_type, 3> &tries = m_tries;
        cltj_index_metatrie() = default;

        cltj_index_metatrie(vector<spo_triple> &D){

            m_tries[0] = metatrie_type(D, 0/*spo_orders[0]*/, 1/*spo_orders[1]*/);
            // Hay que poner D en orden POS
            m_tries[1] = metatrie_type(D, 2/*spo_orders[2]*/, 3/*spo_orders[3]*/);
            // Hay que poner D en orden PSO
            m_tries[2] = metatrie_type(D, 4/*spo_orders[4]*/, 5/*spo_orders[5]*/);
             
/*
            for(size_type i = 0; i < 3; ++i){
                size_type n_nodes = 1;
                TrieV2* trie1 = create_trie(D, spo_orders[2*i], n_nodes);
                TrieV2* trie2 = create_trie(D, spo_orders[2*i+1],n_nodes);
                m_tries[i] = metatrie_type(trie1, trie2, n_nodes,i);
                delete trie1;
                delete trie2;
            }
*/

        }

        //! Copy constructor
        cltj_index_metatrie(const cltj_index_metatrie &o) {
            copy(o);
        }

        //! Move constructor
        cltj_index_metatrie(cltj_index_metatrie &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        cltj_index_metatrie &operator=(const cltj_index_metatrie &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        cltj_index_metatrie &operator=(cltj_index_metatrie &&o) {
            if (this != &o) {
                m_tries = std::move(o.m_tries);
            }
            return *this;
        }

        void swap(cltj_index_metatrie &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_tries, o.m_tries);
        }

        inline metatrie_type* get_metatrie(size_type i){
            return &m_tries[i];
        }

        size_type serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name = "") const {
            sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            for(const auto & trie : m_tries){
                written_bytes += trie.serialize(out, child, "tries");
            }
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream &in) {
            for(auto & trie : m_tries){
                trie.load(in);
            }
        }

    };

    typedef cltj::cltj_index_metatrie<cltj::compact_trie_v3> compact_ltj_metatrie;   // OJO DIEGO: revisar esto
    typedef cltj::cltj_index_metatrie<cltj::uncompact_trie_v2> uncompact_ltj;

}

#endif //CLTJ_CLTJ_INDEX_ADRIAN_HPP
