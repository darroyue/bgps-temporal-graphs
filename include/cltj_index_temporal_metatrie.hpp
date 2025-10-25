//
// Index definition
//

#ifndef CLTJ_CLTJ_INDEX_TEMPORAL_METATRIE_HPP
#define CLTJ_CLTJ_INDEX_TEMPORAL_METATRIE_HPP

#include <queue>
#include <cltj_compact_trie_v3.hpp>
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
            return 8*sizeof(uint32_t) - __builtin_clz(max);
        }

    public:

        const std::array<trie_type, 18> &tries = m_tries;
        cltj_index_temporal_metatrie() = default;

        cltj_index_temporal_metatrie(vector<spot_quad> &D) {

            cout << "***** Building SPOT ****************************" << endl;
            m_tries[SPOT] = create_full_trie_SPOT<trie_type>(D);      // trie for SPOT
            cout << "SPOT: " << sdsl::size_in_bytes(m_tries[SPOT]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[SPOT])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building SPT ****************************" << endl;
            m_tries[SPT]  = create_partial_trie_2T<trie_type>(D, SPT); // trie for SPT
            cout << "SPT: " << sdsl::size_in_bytes(m_tries[SPT]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[SPT])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building SO ****************************" << endl;
	        m_tries[SO]   = create_partial_trie_2<trie_type>(D, SO);  // trie for SO
            cout << "SO: " << sdsl::size_in_bytes(m_tries[SO]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[SO])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building ST ****************************" << endl;
	        m_tries[ST]   = create_partial_trie_1T<trie_type>(D, ST); // trie for ST
            cout << "ST: " << sdsl::size_in_bytes(m_tries[ST]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[ST])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building POS ****************************" << endl;
	        m_tries[POS]  = create_full_trie_no_t<trie_type>(D, POS);  // trie for POS
            cout << "POS: " << sdsl::size_in_bytes(m_tries[POS]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[POS])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building POT ****************************" << endl;
	        m_tries[POT]  = create_partial_trie_2T<trie_type>(D, POT); // trie for POT
            cout << "POT: " << sdsl::size_in_bytes(m_tries[POT]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[POT])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building PS ****************************" << endl;
            m_tries[PS]   = create_partial_trie_2<trie_type>(D, PS); // trie for PS
            cout << "PS: " << sdsl::size_in_bytes(m_tries[PS]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[PS])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building PT ****************************" << endl;
            m_tries[PT]   = create_partial_trie_1T<trie_type>(D, PT); // trie for PT
            cout << "PT: " << sdsl::size_in_bytes(m_tries[PT]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[PT])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building OSP ****************************" << endl;
            m_tries[OSP]  = create_full_trie_no_t<trie_type>(D, OSP); // trie for OSP
            cout << "OSP: " << sdsl::size_in_bytes(m_tries[OSP]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[OSP])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building OST ****************************" << endl;
            m_tries[OST]  = create_partial_trie_2T<trie_type>(D, OST); // trie for OST
            cout << "OST: " << sdsl::size_in_bytes(m_tries[OST]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[OST])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building OP ****************************" << endl;
            m_tries[OP]   = create_partial_trie_2<trie_type>(D, OP); // trie for OP
            cout << "OP: " << sdsl::size_in_bytes(m_tries[OP]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[OP])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building OT ****************************" << endl;
            m_tries[OT]   = create_partial_trie_1T<trie_type>(D, OT); // trie for OT
            cout << "OT: " << sdsl::size_in_bytes(m_tries[OT]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[OT])/D.size() << " bytes per quad)" << endl;

            // Now, time-first orders

            // first, line sweep on the time intervals to partition the quads 

            std::vector<std::pair<temporal_interval, std::vector<update_type>>> D_T = generate_list_of_updates(D);
       
            #define STORE_T true

            uint64_t n_bits;
            n_bits = number_of_bits(D); 

            cout << "***** Building TSPO ****************************" << endl;
            m_tries[TSPO] = create_time_first_trie<trie_type>(D, D_T, TSPO, STORE_T, n_bits, 3);
            cout << "TSPO: " << sdsl::size_in_bytes(m_tries[TSPO]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[TSPO])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building TSO ****************************" << endl;
            m_tries[TSO]  = create_time_first_trie<trie_type>(D, D_T, TSO, !STORE_T, n_bits, 2);
            cout << "TSO: " << sdsl::size_in_bytes(m_tries[TSO]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[TSO])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building TPOS ****************************" << endl;
            m_tries[TPOS] = create_time_first_trie<trie_type>(D, D_T, TPOS, !STORE_T, n_bits, 3);
            cout << "TPOS: " << sdsl::size_in_bytes(m_tries[TPOS]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[TPOS])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building TPS ****************************" << endl;
            m_tries[TPS]  = create_time_first_trie<trie_type>(D, D_T, TPS, !STORE_T, n_bits, 2);
            cout << "TPS: " << sdsl::size_in_bytes(m_tries[TPS]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[TPS])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building TOSP ****************************" << endl;
            m_tries[TOSP] = create_time_first_trie<trie_type>(D, D_T, TOSP, !STORE_T, n_bits, 3);
            cout << "TOSP: " << sdsl::size_in_bytes(m_tries[TOSP]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[TOSP])/D.size() << " bytes per quad)" << endl;

            cout << "***** Building TOP ****************************" << endl;
            m_tries[TOP]  = create_time_first_trie<trie_type>(D, D_T, TOP, !STORE_T, n_bits, 2); 
            cout << "TOP: " << sdsl::size_in_bytes(m_tries[TOP]) << " bytes (" 
                 << (float)sdsl::size_in_bytes(m_tries[TOP])/D.size() << " bytes per quad)" << endl;
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
            std::swap(m_tries, o.m_tries);
        }

        inline trie_type* get_trie(size_type i) {
            return &m_tries[i];
        }

        size_type serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name = "") const {
            sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            for(const auto & trie : m_tries) {
                written_bytes += trie.serialize(out, child, "tries");
            }
            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream &in) {
            for(auto & trie : m_tries) {
                trie.load(in);
            }
        }

    };

    typedef cltj::cltj_index_temporal_metatrie<cltj::compact_trie_v3> compact_temporal_ltj_metatrie;
}

#endif 
