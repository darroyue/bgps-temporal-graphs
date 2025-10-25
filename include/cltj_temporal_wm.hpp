
// The implementation of the persistent successor data structure

#ifndef CLTJ_TEMPORAL_WM_HPP
#define CLTJ_TEMPORAL_WM_HPP

#include <vector>
#include <algorithm>
#include<unordered_map>
#include <cltj_config.hpp>
#include <sdsl/sdsl_concepts.hpp>
#include <sdsl/int_vector.hpp>

using namespace sdsl;

namespace cltj {

template<class t_bitvector   = sdsl::bit_vector,
         class t_rank        = typename t_bitvector::rank_1_type> 
class temporal_wm {

public: 
        typedef int_vector<>::size_type              size_type;
        typedef int_vector<>::value_type             value_type;
        typedef t_bitvector                          bit_vector_type;
        typedef t_rank                               rank_1_type;

protected:
    size_type m_n_updates;  // number of updates in the sequence of events
    size_type m_tuple_comp;
    size_type m_n_bits;

    bit_vector_type m_B; // bit vector B in the paper
    rank_1_type m_B_rank;

    bit_vector_type m_E; // bit vector E in the paper
    rank_1_type m_E_rank;

//    bit_vector_type m_B_aux; // bit vector B in the paper
//    rank_1_type m_B_aux_rank;

//    bit_vector_type m_E_aux; // bit vector E in the paper
//    rank_1_type m_E_aux_rank;


    void copy(const temporal_wm& wm) {
            m_n_updates  = wm.m_n_updates;
            m_tuple_comp = wm.m_tuple_comp;
	        m_n_bits     = wm.m_n_bits;
            m_B          = wm.m_B;
            m_E          = wm.m_E;
            m_B_rank     = wm.m_B_rank;
            m_E_rank     = wm.m_E_rank;
            // m_B_aux      = wm.m_B_aux;
            // m_E_aux      = wm.m_E_aux;
            // m_B_aux_rank = wm.m_B_aux_rank;
            // m_E_aux_rank = wm.m_E_aux_rank;
    }

    static uint64_t get_component(const spot_quad & t, size_type component) {
        if (component == 0) return get<0>(t);
        else if (component == 1) return get<1>(t);
        else return get<2>(t);
    }


    // Hash for std::tuple<uint64_t,uint64_t,uint64_t>
    struct TripleHash {
        std::size_t operator()(const std::tuple<uint64_t,uint64_t,uint64_t> &t) const noexcept {
            auto [a,b,c] = t;
            std::size_t h = std::hash<uint64_t>{}(a);
            h ^= std::hash<uint64_t>{}(b) + 0x9e3779b9 + (h<<6) + (h>>2);
            h ^= std::hash<uint64_t>{}(c) + 0x9e3779b9 + (h<<6) + (h>>2);
            return h;
        }
    };

    // Extracts bits_to_use most-significant bits from val, which has total_bits bits
    static inline uint64_t top_bits(uint64_t val, int bits_to_use, int total_bits) {
        if (bits_to_use <= 0) return 0;
        if (bits_to_use >= total_bits) return val;
        return val >> (total_bits - bits_to_use);
    }


    static bool get_accumulated_bit(const spot_quad & t, int i, int bits_per_component) {
        int comp = i / bits_per_component;        // determines the component
        int bit_in_comp = i % bits_per_component; // position within the component

        int shift = bits_per_component - 1 - bit_in_comp;

        uint64_t val;
        if (comp == 0) val = get<0>(t);
        else if (comp == 1) val = get<1>(t);
        else val = get<2>(t);
        
        return (val >> shift) & 1ULL;
    }

    struct PrefixComparator {
        const std::vector<spot_quad> & data;
        int total_bits_used; 
        int bits_per_component; 
        int num_components;   

        PrefixComparator(const std::vector<spot_quad> & d,
                        int total_bits, int bits_per_comp, int n_comp)
                        : data(d), total_bits_used(total_bits),
                        bits_per_component(bits_per_comp), num_components(n_comp) {}

        bool operator() (size_type a, size_type b) const {
            int bits_left = total_bits_used;
            for (int c = 0; c < num_components && bits_left > 0; ++c) {
                int use = std::min(bits_left, bits_per_component);
                uint64_t va = top_bits(get_component(data[a], c), use, bits_per_component);
                uint64_t vb = top_bits(get_component(data[b], c), use, bits_per_component);
                if (va != vb) return va < vb;
                bits_left -= use;
            }
            return false;
        }
    };

    void generate_B_E_stable_prefix_sort(const std::vector<spot_quad> & data,
                             vector<bool> & delete_flags, int bits) {
        size_type n = data.size();
        vector<size_type> idx(n);
        for (size_type i = 0; i < n; ++i)
            idx[i] = i;

        size_type bit_in_B = 0, bit_in_E = 0;
        size_type diff_delete, j;
        bool cur_bit;

        diff_delete = 0;
        for (size_type i = 0; i < n; ++i) {
            diff_delete += (delete_flags[i])?-1:1;
            m_E[bit_in_E++] = (diff_delete == 0) ? 0 : 1;
        }

        for (size_type cur_bit = 1; cur_bit <= m_tuple_comp * bits; ++cur_bit) { 
            
            for (size_type k = 0; k < n; ++k) {
	            m_B[bit_in_B] = get_accumulated_bit(data[idx[k]], cur_bit-1, bits); 
                ++bit_in_B;
            }
        
            PrefixComparator cmp(data, cur_bit, bits, m_tuple_comp);
            stable_sort(idx.begin(), idx.end(), cmp);

            int cur_comp = (int)((cur_bit - 1) / bits);          // component of the i-th bit (0-based)
            int bit_in_comp = (int)((cur_bit - 1) % bits);       // index within that component (0..bits-1)
            int bits_in_this_comp = bit_in_comp + 1;        // number of bits from that component

            // hash table for (prefix0, prefix1, prefix2) keys
            std::unordered_map<std::tuple<uint64_t,uint64_t,uint64_t>, int, TripleHash> prefix_count;
            prefix_count.reserve(2*n);

            for (size_type k = 0; k < n; ++k) {
                const spot_quad &t = data[idx[k]];

                uint64_t p0 = 0, p1 = 0, p2 = 0;

                // component 0
                if (0 < cur_comp) {
                    // component 0 already processed -> use all of it
                    p0 = get_component(t, 0);
                } else if (0 == cur_comp) {
                    // component 0 partially processed (currently processing it within this pass)
                    p0 = top_bits(get_component(t, 0), bits_in_this_comp, bits);
                } else {
                    p0 = 0;
                }

                // component 1
                if (1 < cur_comp) {
                    p1 = get_component(t, 1);
                } else if (1 == cur_comp) {
                    p1 = top_bits(get_component(t, 1), bits_in_this_comp, bits);
                } else {
                    p1 = 0;
                }

                // component 2
                if (2 < cur_comp) {
                    p2 = get_component(t, 2);
                } else if (2 == cur_comp) {
                    p2 = top_bits(get_component(t, 2), bits_in_this_comp, bits);
                } else {
                    p2 = 0;
                }

                auto key = std::make_tuple(p0, p1, p2);

                if (delete_flags[idx[k]]) 
                    prefix_count[key]--;
                else 
                    prefix_count[key]++;

                // mark m_E_aux (1 if prefix_count > 0, 0 if <= 0)
                m_E[bit_in_E++] = (prefix_count[key] == 0) ? 0 : 1;
            }
        }
    }

    int64_t rank_range_B(size_type depth, size_type s, size_type e) {
        return m_B_rank(depth*m_n_updates+e+1) - m_B_rank(depth*m_n_updates + s);
    }

    int64_t rank_range_E(size_type depth, size_type s, size_type e) {
        return m_E_rank(depth*m_n_updates+e+1) - m_E_rank(depth*m_n_updates + s);
    }

    #define INFTY UINT32_MAX

    // parameter depth is the depth in the VBT
    size_type leftmost(size_type depth, int64_t s, int64_t e, int64_t p, size_type h,
                       std::pair<std::pair<size_type, size_type>, size_type> & node_pair) {
        if ((int64_t)p < 0) return INFTY;

        if (h == 0) { 
            node_pair.second = p;
            node_pair.first.first = s;
            node_pair.first.second = e;
            return 0;
        }

        int64_t r = rank_range_B(depth, s, e);
        int64_t p_prime = rank_range_B(depth, s, s+p);
        if (s <= e-r && m_E[(depth+1)*m_n_updates + s + p - p_prime] == 1) {
            size_type temp = leftmost(depth+1, s, e-r, p-p_prime, h-1, node_pair);
            if (temp == INFTY) return INFTY;
            else return temp; 
        } else {
            size_type temp = leftmost(depth+1, e-r+1, e, p_prime-1, h-1, node_pair);
            if (temp == INFTY) return INFTY;
            else return (uint64_t(1)<<(h-1)) + temp; 
        }
    }

public:
 
    temporal_wm() = default;

    temporal_wm(const std::vector<spot_quad> & update_tuples, std::vector<bool> is_delete, 
                size_type n_tuple_components, size_type n_bits, bool is_partial = false) {

        m_n_updates  = update_tuples.size();
        m_tuple_comp = n_tuple_components; 
        m_n_bits     = n_bits;

	    m_B = bit_vector_type(m_n_bits*m_n_updates*m_tuple_comp, 0);
        m_E = bit_vector_type(m_n_bits*m_n_updates*m_tuple_comp+m_n_updates, 0);

        generate_B_E_stable_prefix_sort(update_tuples, is_delete, n_bits);

        if (is_partial) {
            // we only store only the middle component, as the firt one
            // is shared with another trie via the meta trie
            m_tuple_comp = 1; // a single component
            bit_vector_type m_B_aux(m_n_bits*m_n_updates, 0);
            bit_vector_type m_E_aux((m_n_bits+1)*m_n_updates, 0);
            
            uint64_t l, r, i, i_aux;
            l = m_n_bits*m_n_updates;
            r = 2*m_n_bits*m_n_updates;
            for (i_aux = 0, i = l; i < r; i++, i_aux++) {
                m_B_aux[i_aux] = m_B[i];
                m_E_aux[i_aux] = m_E[i];
            }

            for (uint64_t k = 0; k < m_n_updates; k++) {
                m_E_aux[i_aux++] = m_E[i++];
            }

            m_B.swap(m_B_aux);
            m_E.swap(m_E_aux);
        }

        sdsl::util::init_support(m_B_rank, &m_B);
        sdsl::util::init_support(m_E_rank, &m_E);
    }

    //! Copy constructor
    temporal_wm(const temporal_wm& wm) {
        copy(wm);
    }

    //! Copy constructor
    temporal_wm(temporal_wm&& wm) {
        *this = std::move(wm);
    }

    //! Assignment operator
    temporal_wm& operator=(const temporal_wm& wm) {
        if (this != &wm) {
            copy(wm);
        }
        return *this;
    }

    //! Assignment move operator
    temporal_wm& operator=(temporal_wm&& wm) {
        if (this != &wm) {
            m_n_updates  = wm.m_n_updates;
            m_tuple_comp = wm.m_tuple_comp;
	        m_n_bits     = wm.m_n_bits;
            m_B          = std::move(wm.m_B);
            m_E          = std::move(wm.m_E);
            m_B_rank     = std::move(wm.m_B_rank);
            m_B_rank.set_vector(&m_B);
            m_E_rank     = std::move(wm.m_E_rank);
            m_E_rank.set_vector(&m_E);
            // m_B_aux          = std::move(wm.m_B_aux);
            // m_E_aux          = std::move(wm.m_E_aux);
            // m_B_aux_rank     = std::move(wm.m_B_aux_rank);
            // m_B_aux_rank.set_vector(&m_B_aux);
            // m_E_aux_rank     = std::move(wm.m_E_aux_rank);
            // m_E_aux_rank.set_vector(&m_E_aux);
        }
        return *this;
    }

    //! Swap operator
    void swap(temporal_wm& wm) {
        if (this != &wm) {
            std::swap(m_n_updates, wm.m_n_updates);
            std::swap(m_tuple_comp, wm.m_tuple_comp);
	        std::swap(m_n_bits, wm.m_n_bits);
            m_B.swap(wm.m_B); 
            m_E.swap(wm.m_E);            
            sdsl::util::swap_support(m_B_rank, wm.m_B_rank, &m_B, &(wm.m_B));
            sdsl::util::swap_support(m_E_rank, wm.m_E_rank, &m_E, &(wm.m_E));
            // m_B_aux.swap(wm.m_B_aux); 
            // m_E_aux.swap(wm.m_E_aux);            
            // sdsl::util::swap_support(m_B_aux_rank, wm.m_B_aux_rank, &m_B_aux, &(wm.m_B_aux));
            // sdsl::util::swap_support(m_E_aux_rank, wm.m_E_aux_rank, &m_E_aux, &(wm.m_E_aux));
        }
    }

    size_type get_n_bits() {
        return m_n_bits;
    }

    // produces the interval corresponding to the root of the temporal data structure
    std::pair<size_type, size_type> get_root() {
        return make_pair(0, m_n_updates-1);
    }

    // yields the number of children in the (simulated) trie
    size_type node_degree(size_type depth, size_type pos, std::pair<size_type, size_type> node) {
        return rank_range_E(depth, node.first, node.first + pos);
    }
    
    size_type leap(size_type depth, int64_t s, int64_t e, int64_t p, value_type x, size_type h,
                   std::pair<std::pair<size_type, size_type>, size_type> &  node_pair) {

        if (x > ((1ULL<<m_n_bits)-1)) return INFTY;

        if ((int64_t)p < 0 || s > e || m_E[depth*m_n_updates + s + p] == 0) return INFTY; 

        if (h == 0) {
            node_pair.second = p;
            node_pair.first.first = s;
            node_pair.first.second = e; 
            return 0;
        }

        int64_t r = rank_range_B(depth, s, e);
        int64_t p_prime = rank_range_B(depth, s, s+p);   // p is relative to s

        if ((x & (uint64_t(1)<<(h-1)))!= 0) {
            x = leap(depth+1, e-r+1, e, p_prime-1, x-(uint64_t(1)<<(h-1)), h-1, node_pair);
            if (x != INFTY) return x + (uint64_t(1)<<(h-1));
            else return INFTY;
        } else {
            x = leap(depth+1, s, e-r, p-p_prime, x, h-1, node_pair);
            if (x != INFTY) return x;
            if (r == 0 || m_E[(depth+1)*m_n_updates + e - r + p_prime] == 0) 
                return INFTY;
            else {
                x = leftmost(depth+1, e-r+1, e, p_prime-1, h-1, node_pair);
                if (x == INFTY) return INFTY;
                else return (uint64_t(1)<<(h-1)) + x;
            }
        }
    }
    
    //! Serializes the data structure into the given ostream
    size_type serialize(std::ostream& out, structure_tree_node* v=nullptr, std::string name="") const {
        structure_tree_node* child = structure_tree::add_child(v, name, sdsl::util::class_name(*this));
        size_type written_bytes = 0;
        written_bytes += write_member(m_n_updates, out, child, "n_updates");
        written_bytes += write_member(m_tuple_comp, out, child, "tuple_comp");
	    written_bytes += write_member(m_n_bits, out, child, "n_bits");
        written_bytes += m_B.serialize(out, child, "B");
        written_bytes += m_E.serialize(out, child, "E");
        written_bytes += m_B_rank.serialize(out, child, "B_rank");
        written_bytes += m_E_rank.serialize(out, child, "E_rank");
        // written_bytes += m_B_aux.serialize(out, child, "B");
        // written_bytes += m_E_aux.serialize(out, child, "E");
        // written_bytes += m_B_aux_rank.serialize(out, child, "B_aux_rank");
        // written_bytes += m_E_aux_rank.serialize(out, child, "E_aux_rank");
        structure_tree::add_size(child, written_bytes);
        return written_bytes;
    }

    //! Loads the data structure from the given istream.
    void load(std::istream& in) {
        read_member(m_n_updates, in);
        read_member(m_tuple_comp, in);
	    read_member(m_n_bits, in);
        m_B.load(in);
        m_E.load(in);
        m_B_rank.load(in, &m_B);
        m_E_rank.load(in, &m_E);
        // m_B_aux.load(in);
        // m_E_aux.load(in);
        // m_B_aux_rank.load(in, &m_B_aux);
        // m_E_aux_rank.load(in, &m_E_aux);
    }
};


} 
#endif 
