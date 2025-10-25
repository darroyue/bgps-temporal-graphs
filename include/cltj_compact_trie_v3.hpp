// Compact tries for joins in temporal graphs


#ifndef CLTJ_COMPACT_TRIE_V3_H
#define CLTJ_COMPACT_TRIE_V3_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <sdsl/vectors.hpp>
#include <sdsl/select_support_mcl.hpp>
//#include <cltj_regular_trie_v2.hpp>
#include <succ_support_v.hpp>
#include <cltj_config.hpp>
#include <cltj_temporal_wm.hpp>

namespace cltj {

    class compact_trie_v3 {

    public:

        typedef uint64_t size_type;
        typedef uint32_t value_type;

        // tries for orders TSPO 
        #define TIME_FIRST_FULL_INTERVALS      0

        // tries for orders TPOS, TOSP
        #define TIME_FIRST_FULL_NO_INTERVALS   1

        // tries for orders TOP, TPS, TSO
        #define TIME_FIRST_PARTIAL             2

        // trie for order SPOT
        #define S_FIRST_FULL                   3

        // tries for orders SPT, POT, OST
        #define TRIE_3_LEVELS_T_LAST           4

        // tries for orders POS, OSP 
        #define TRIE_3_LEVELS_NO_T             5

        // tries for orders ST, PT, OT
        #define TRIE_2_LEVELS_T_LAST           6

        // tries for orders SO, PS, OP 
        #define TRIE_2_LEVELS_NO_T             7        

    private:
        sdsl::bit_vector m_bv;    // trie topology
        sdsl::int_vector<> m_seq; // sequence of values stored at nodes
        sdsl::int_vector<> m_tempint_left; // sequence of leftmost endpoints of temporal intervals
        sdsl::int_vector<> m_tempint_right;// sequence of rightmost endpoints of temporal intervals

        cds::succ_support_v<0> m_succ0;
        sdsl::select_support_mcl<0> m_select0;

        size_type m_root_degree;
        uint8_t m_trie_kind; // indicates the type of trie

        sdsl::bit_vector m_last_update_per_int;  // bit vector indicating the last update per interval
        sdsl::select_support_mcl<1> m_last_update_select1;
        temporal_wm<> m_temporal_ds; 

        void copy(const compact_trie_v3 &o) {
            m_bv = o.m_bv;
            m_seq = o.m_seq;
            m_tempint_left = o.m_tempint_left;
            m_tempint_right = o.m_tempint_right;
            m_succ0 = o.m_succ0;
            m_succ0.set_vector(&m_bv);
            m_select0 = o.m_select0;
            m_select0.set_vector(&m_bv);
            m_root_degree = o.m_root_degree;
            m_trie_kind = o.m_trie_kind;
            m_temporal_ds = o.m_temporal_ds;
            m_last_update_per_int = o.m_last_update_per_int;
            m_last_update_select1 = o.m_last_update_select1; 
        }

    public:

        const sdsl::int_vector<> &seq = m_seq;

        compact_trie_v3() = default;

        // Constructor for tries with no temporal component 
        compact_trie_v3(sdsl::bit_vector &_bv, sdsl::int_vector<> &_seq, uint8_t kind_of_trie) {
             m_trie_kind = kind_of_trie;
             m_bv = _bv; 
             m_seq = _seq;
             sdsl::util::init_support(m_succ0, &m_bv);
             sdsl::util::init_support(m_select0, &m_bv);
             m_root_degree = m_succ0(1);
        }

        // Constructor for tries with temporal component
        compact_trie_v3(sdsl::bit_vector &_bv, sdsl::int_vector<> &_seq, std::vector<temporal_interval> &interval_seq, uint8_t kind_of_trie) {
             m_trie_kind = kind_of_trie;
             m_bv = _bv;
             m_seq = _seq;
             m_tempint_left = sdsl::int_vector<>(interval_seq.size());
             m_tempint_right = sdsl::int_vector<>(interval_seq.size());
             for (uint64_t i = 0; i < interval_seq.size(); i++) {
                 m_tempint_left[i] = interval_seq[i].first;
                 m_tempint_right[i] = interval_seq[i].second /*+ ((m_trie_kind == S_FIRST_FULL)?1:0)*/;
             }
             sdsl::util::bit_compress(m_tempint_left);
             sdsl::util::bit_compress(m_tempint_right);
             sdsl::util::init_support(m_succ0, &m_bv);
             sdsl::util::init_support(m_select0, &m_bv);
             m_root_degree = m_succ0(1);
        }

        // Constructor for time-first tries 
        compact_trie_v3(std::vector<temporal_interval> &interval_seq, temporal_wm<> &temp_ds,
                        sdsl::bit_vector &last_update_bv, uint8_t kind_of_trie) {
             m_trie_kind = kind_of_trie;
             m_temporal_ds = temp_ds;
             m_last_update_per_int = last_update_bv;
             sdsl::util::init_support(m_last_update_select1, &m_last_update_per_int);
             m_tempint_left = sdsl::int_vector<>(interval_seq.size());
             m_tempint_right = sdsl::int_vector<>(interval_seq.size());
             for (uint64_t i = 0; i < interval_seq.size(); i++) {
                 m_tempint_left[i] = interval_seq[i].first;
                 m_tempint_right[i] = interval_seq[i].second;
             }
             sdsl::util::bit_compress(m_tempint_left);
             sdsl::util::bit_compress(m_tempint_right);
             m_root_degree = interval_seq.size(); 
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
                m_tempint_left = std::move(o.m_tempint_left);
                m_tempint_right = std::move(o.m_tempint_right);
                m_succ0 = std::move(o.m_succ0);
                m_succ0.set_vector(&m_bv);
                m_select0 = std::move(o.m_select0);
                m_select0.set_vector(&m_bv);
                m_root_degree = o.m_root_degree;
                m_trie_kind = o.m_trie_kind;
                m_temporal_ds = o.m_temporal_ds;
                m_last_update_per_int = o.m_last_update_per_int;
                m_last_update_select1 = std::move(o.m_last_update_select1);
                m_last_update_select1.set_vector(&m_last_update_per_int); 
            }
            return *this;
        }

        void swap(compact_trie_v3 &o) {
            std::swap(m_bv, o.m_bv);
            std::swap(m_seq, o.m_seq);
            std::swap(m_tempint_left, o.m_tempint_left);
            std::swap(m_tempint_right, o.m_tempint_right);
            sdsl::util::swap_support(m_succ0, o.m_succ0, &m_bv, &o.m_bv);
            sdsl::util::swap_support(m_select0, o.m_select0, &m_bv, &o.m_bv);
            std::swap(m_root_degree, o.m_root_degree);
            std::swap(m_trie_kind, o.m_trie_kind);
            std::swap(m_temporal_ds, o.m_temporal_ds);
            std::swap(m_last_update_per_int, o.m_last_update_per_int);
            sdsl::util::swap_support(m_last_update_select1, o.m_last_update_select1, &m_last_update_per_int, &o.m_last_update_per_int); // OJO
        }

        /*
            Degree of the trie root. For time-first tries, it equals
            the numer of intervals that are children of the trie root
        */
         size_type root_degree() {
            return m_root_degree - (m_trie_kind == TIME_FIRST_FULL_INTERVALS);
         }

        std::pair<size_type, size_type> get_temporal_root() {
           return m_temporal_ds.get_root();
        }

        size_type temporal_ds_node_degree(size_type depth, size_type pos, std::pair<size_type, size_type> node) {
            return m_temporal_ds.node_degree(depth, pos, node);
        }

        size_type get_last_update_of_interval(size_type pos) {
            return m_last_update_select1(pos+1/*OJO con el +1*/);
        }

        std::pair<size_type, size_type> get_interval_at_pos(size_type pos) {
            return make_pair(m_tempint_left[pos-seq.size()], m_tempint_right[pos - seq.size()]);
        }

        size_type number_of_updates() {
            return m_last_update_per_int.size();
        }

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


        // Returns the interval containing val. If there is no such interval, returns the "successor" interval
        pair<temporal_interval, uint32_t> binary_search_interval_seek(uint32_t val, uint32_t i, uint32_t f) const {
            i = i - m_seq.size(); 
            f = f - m_seq.size(); 
            
            if (m_tempint_right[f] <= val) return make_pair(temporal_interval(0,0), m_seq.size()+f+1);
            if (m_tempint_left[i] > val) return make_pair(temporal_interval(m_tempint_left[i], m_tempint_right[i]), m_seq.size()+i); // return leftmost endpoint of first interval
            uint32_t mid;
            uint32_t orig_i = i;
            uint32_t orig_f = f;

            while (i < f) {
                mid = (f + i) / 2;
                if (m_tempint_left[mid] < val)
                    i = mid + 1;   
                else
                    f = mid;      
            }

            if (i == orig_f && m_tempint_left[i] <= val && val < m_tempint_right[i])
                return make_pair(temporal_interval(val, m_tempint_right[i]), m_seq.size()+i);

            // if val is within the previous interval, returns it
            if (i > orig_i && val < m_tempint_right[i-1])
                return make_pair(temporal_interval(val, m_tempint_right[i-1]), m_seq.size()+i-1);
            else
                // otherwise returns the successor
                return make_pair(temporal_interval(m_tempint_left[i], m_tempint_right[i]), m_seq.size()+i);
        }

        // depth is given in terms of quad components (i.e., it can be 0, 1, 2). It must be transformed
        // into bits 
        size_type temporal_successor(size_type depth, std::pair<size_type, size_type> &node_interval, 
                                     int64_t pos, value_type val,
                                     std::pair<std::pair<size_type, size_type>, size_type> &node_pair) {

            return m_temporal_ds.leap(depth*m_temporal_ds.get_n_bits(), (int64_t)node_interval.first, 
                                      (int64_t)node_interval.second, (int64_t)pos, val, 
                                      m_temporal_ds.get_n_bits(), node_pair);
        }

        void print() const {
            for (size_type i = 0; i < m_bv.size(); ++i) {
                std::cout << (uint) m_bv[i];
            }
            std::cout << std::endl;
        }

        //! Serializes the data structure into the given ostream
        size_type serialize(std::ostream &out, sdsl::structure_tree_node *v = nullptr, std::string name = "") const {
            sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += write_member(m_trie_kind, out, child, "trie_kind");
            written_bytes += write_member(m_root_degree, out, child, "root_degree");
            if (m_trie_kind != TIME_FIRST_FULL_INTERVALS && m_trie_kind != TIME_FIRST_FULL_NO_INTERVALS 
             && m_trie_kind != TIME_FIRST_PARTIAL) {
                written_bytes += m_bv.serialize(out, child, "bv");
                written_bytes += m_succ0.serialize(out, child, "succ0");
                written_bytes += m_select0.serialize(out, child, "select0");
            }

            if (m_trie_kind == S_FIRST_FULL || m_trie_kind == TRIE_3_LEVELS_NO_T 
             || m_trie_kind == TRIE_2_LEVELS_NO_T) {
                written_bytes += m_seq.serialize(out, child, "seq");
            }

            if (m_trie_kind == TIME_FIRST_FULL_INTERVALS || m_trie_kind == S_FIRST_FULL 
             || m_trie_kind == TRIE_3_LEVELS_T_LAST || m_trie_kind == TRIE_2_LEVELS_T_LAST) {
                written_bytes += m_tempint_left.serialize(out, child, "m_tempint_left");
                written_bytes += m_tempint_right.serialize(out, child, "m_tempint_right");
            }

            if (m_trie_kind == TIME_FIRST_FULL_INTERVALS || m_trie_kind == TIME_FIRST_FULL_NO_INTERVALS 
             || m_trie_kind == TIME_FIRST_PARTIAL) {
                written_bytes += m_temporal_ds.serialize(out, child, "temporal_ds");
            }

            if (m_trie_kind == TIME_FIRST_FULL_INTERVALS) { 
                written_bytes += m_last_update_per_int.serialize(out, child, "last_update_per_int");
                written_bytes += m_last_update_select1.serialize(out, child, "last_update_select1");
            }

            sdsl::structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        void load(std::istream &in) {
            uint64_t i = 1;
            read_member(m_trie_kind, in);
            read_member(m_root_degree, in);

            if (m_trie_kind != TIME_FIRST_FULL_INTERVALS && m_trie_kind != TIME_FIRST_FULL_NO_INTERVALS 
             && m_trie_kind != TIME_FIRST_PARTIAL) {
                m_bv.load(in);
                m_succ0.load(in, &m_bv);
                m_select0.load(in, &m_bv);
            }

            if (m_trie_kind == S_FIRST_FULL || m_trie_kind == TRIE_3_LEVELS_NO_T 
             || m_trie_kind == TRIE_2_LEVELS_NO_T) {
                m_seq.load(in);
            }

            if (m_trie_kind == TIME_FIRST_FULL_INTERVALS || m_trie_kind == S_FIRST_FULL 
             || m_trie_kind == TRIE_3_LEVELS_T_LAST || m_trie_kind == TRIE_2_LEVELS_T_LAST) {
                m_tempint_left.load(in);
                m_tempint_right.load(in);
            }

            if (m_trie_kind == TIME_FIRST_FULL_INTERVALS || m_trie_kind == TIME_FIRST_FULL_NO_INTERVALS 
             || m_trie_kind == TIME_FIRST_PARTIAL) {
                m_temporal_ds.load(in);
            }

            if (m_trie_kind == TIME_FIRST_FULL_INTERVALS) { 
                m_last_update_per_int.load(in);
                m_last_update_select1.load(in, &m_last_update_per_int);
            }
        }
    };
}
#endif
