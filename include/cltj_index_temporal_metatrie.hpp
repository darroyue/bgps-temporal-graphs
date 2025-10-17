//
// Created by Diego Arroyuelo on 20/09/24.
//

#ifndef CLTJ_CLTJ_INDEX_TEMPORAL_METATRIE_HPP
#define CLTJ_CLTJ_INDEX_TEMPORAL_METATRIE_HPP

#include <queue>
//#include <cltj_compact_trie_v2.hpp>
#include <cltj_compact_trie_v3.hpp>
//#include <cltj_uncompact_trie_v2.hpp>
//#include <cltj_metatrie.hpp>
#include <cltj_config.hpp>
#include <cltj_build_compact_tries.hpp>

namespace cltj {

    template<class Trie>
    class cltj_index_temporal_metatrie {

    public:
        typedef uint64_t size_type;
        typedef uint32_t value_type;
        typedef Trie trie_type;

    private:
        // The 18 tries of the index
        // see cltj_config.hpp for the definition of the 18 orders
        std::array<trie_type, 18> m_tries;

        void copy(const cltj_index_temporal_metatrie &o){
            m_tries = o.m_tries;
        }

        uint64_t number_of_bits(std::vector<spot_quad> &D) {
            uint32_t max = D[0][0];

            for (uint64_t i = 0; i < D.size(); i++) {
                if (D[i][0] > max) max = D[i][0];
                if (D[i][1] > max) max = D[i][1];
                if (D[i][2] > max) max = D[i][2];
            }
            /*cout << "En number_of_bits() el maximo es " << max << endl;*/
            return 8*sizeof(uint32_t) - __builtin_clz(max);
        }

    public:

        const std::array<trie_type, 18> &tries = m_tries;
        cltj_index_temporal_metatrie() = default;

        cltj_index_temporal_metatrie(vector<spot_quad> &D) {
            cout << "***** Building SPOT ****************************" << endl;
            m_tries[SPOT] = create_full_trie_SPOT<trie_type>(D);      // trie for SPOT
            cout << "***** Building SPT ****************************" << endl;
            m_tries[SPT]  = create_partial_trie_2T<trie_type>(D, SPT); // trie for SPT
            cout << "***** Building SO ****************************" << endl;
	        m_tries[SO]   = create_partial_trie_2<trie_type>(D, SO);  // trie for SO
            cout << "***** Building ST ****************************" << endl;
	        m_tries[ST]   = create_partial_trie_1T<trie_type>(D, ST); // trie for ST

            cout << "***** Building POS ****************************" << endl;
	        m_tries[POS]  = create_full_trie_no_t<trie_type>(D, POS);  // trie for POS
            cout << "***** Building POT ****************************" << endl;
	        m_tries[POT]  = create_partial_trie_2T<trie_type>(D, POT); // trie for POT
            cout << "***** Building PS ****************************" << endl;
            m_tries[PS]   = create_partial_trie_2<trie_type>(D, PS); // trie for PS
            cout << "***** Building PT ****************************" << endl;
            m_tries[PT]   = create_partial_trie_1T<trie_type>(D, PT); // trie for PT

            cout << "***** Building OSP ****************************" << endl;
            m_tries[OSP]  = create_full_trie_no_t<trie_type>(D, OSP); // trie for OSP
            cout << "***** Building OST ****************************" << endl;
            m_tries[OST]  = create_partial_trie_2T<trie_type>(D, OST); // trie for OST
            cout << "***** Building OP ****************************" << endl;
            m_tries[OP]   = create_partial_trie_2<trie_type>(D, OP); // trie for OP
            cout << "***** Building OT ****************************" << endl;
            m_tries[OT]   = create_partial_trie_1T<trie_type>(D, OT); // trie for OT

            // Now, time-first orders

            // line sweep on the time intervals to partition the quads 

            std::vector<std::pair<temporal_interval, std::vector<update_type>>> D_T = generate_list_of_updates(D);
       
            #define STORE_T true

            uint64_t n_bits;
            n_bits = number_of_bits(D); // OJO, aqui puse D, puede ser D_T 
            //cout << "Numero de bits a usar es " << n_bits << endl;
            cout << "***** Building TSPO ****************************" << endl;
            m_tries[TSPO] = create_time_first_trie<trie_type>(D, D_T, TSPO, STORE_T, n_bits, 3);
            cout << "***** Building TSO ****************************" << endl;
            m_tries[TSO]  = create_time_first_trie<trie_type>(D, D_T, TSO, !STORE_T, n_bits, 2);
            cout << "***** Building TPOS ****************************" << endl;
            m_tries[TPOS] = create_time_first_trie<trie_type>(D, D_T, TPOS, !STORE_T, n_bits, 3);
            cout << "***** Building TPS ****************************" << endl;
            m_tries[TPS]  = create_time_first_trie<trie_type>(D, D_T, TPS, !STORE_T, n_bits, 2);
            cout << "***** Building TOSP ****************************" << endl;
            m_tries[TOSP] = create_time_first_trie<trie_type>(D, D_T, TOSP, !STORE_T, n_bits, 3);
            cout << "***** Building TOP ****************************" << endl;
            m_tries[TOP]  = create_time_first_trie<trie_type>(D, D_T, TOP, !STORE_T, n_bits, 2); 
        }

        //! Copy constructor
        cltj_index_temporal_metatrie(const cltj_index_temporal_metatrie &o) {
            copy(o);
        }

        //! Move constructor
        cltj_index_temporal_metatrie(cltj_index_temporal_metatrie &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        cltj_index_temporal_metatrie &operator=(const cltj_index_temporal_metatrie &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }

        //! Move Operator=
        cltj_index_temporal_metatrie &operator=(cltj_index_temporal_metatrie &&o) {
            if (this != &o) {
                m_tries = std::move(o.m_tries);
            }
            return *this;
        }

        void swap(cltj_index_temporal_metatrie &o) {
            // m_bp.swap(bp_support.m_bp); use set_vector to set the supported bit_vector
            std::swap(m_tries, o.m_tries);
        }

        inline trie_type* get_trie(size_type i) {
            //cout << "get_trie " << i << endl;
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
            //uint64_t i = 0;
            for(auto & trie : m_tries){
                //cout << "cltj_index_temporal_metatrie loading trie " << i++ << "...";
                //fflush(stdout);
                trie.load(in);
                //cout << endl;
            }
        }

    };

    typedef cltj::cltj_index_temporal_metatrie<cltj::compact_trie_v3> compact_temporal_ltj_metatrie;
    //typedef cltj::cltj_index_metatrie<cltj::uncompact_trie_v2> uncompact_ltj;

}

#endif 
