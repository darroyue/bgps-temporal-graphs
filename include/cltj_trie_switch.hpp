

size_type trie_switch_down(size_type nfixed, state_type state) {

    const auto* trie = m_ptr_index->get_trie(m_trie_i);

    if (nfixed == 2) {
        // two states have been instantiated in this iterator, going down with the third one (by parameter state)
        // We must carry out a switch if we were at ST, SO, PT, PS, OT, OP
        if (m_fixed[1] == t) {
            // We are in a normal trie at level 2 corresponding to T
            // So we are at ST, PT, or OT
            // In this case, choose_trie() already choosed the corresponding trie that starts with T and which
            // we are switching to
            // We need to do trie switch and move to some of the orders 
            // that are represented with a temporal data structure (as it starts with T)
            // We first search in the intervals, then go down using the persistent data structure
            auto* trie_aux = m_ptr_index->get_trie(TSPO); 
            
            auto p = trie_aux->binary_search_interval_seek(m_path_label[1], 0, trie_aux->root_degree()-1);

            uint64_t l_update = trie_aux->get_last_update_of_interval(p.second); 
            auto cur_node = trie->get_temporal_root();
            std::pair<std::pair<size_type, size_type>, size_type> node_pair;
            value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[0], node_pair);
            // We use the "it" array within m_status to store information that will be used by parent() from leap()            
            m_status[nfixed].it[0] = node_pair.second; // position within the current range
            m_status[nfixed].it[1] = node_pair.first.first; // left endpoint of the current range 
            m_status[nfixed].it[2] = node_pair.first.second; // right endpoinnt of the current range
            m_status[nfixed].it[3] = 0;
            return 0; // returns a void node in this case, as we already stored all the info in m_status that is needed to go down 
        } else if (m_fixed[0] != t) {
            // WARNING, this if might not be needed
            if ((m_fixed[0] == s && m_fixed[1] == o) ||
                (m_fixed[0] == p && m_fixed[1] == s) ||
                (m_fixed[0] == o && m_fixed[1] == p)) {
                // SO, PS, or OP
                // trie switch to another regular trie
                // first we go down in this trie using the reverse path
                auto* trie_aux = m_ptr_index->get_trie(m_trie_i); 
                auto cnt = trie_aux->root_degree();
                size_type beg, end;
                beg = trie_aux->first_child(0);
                end = beg + cnt - 1;
                auto p = trie_aux->binary_search_seek(m_path_label[1], beg, end);

                size_type cur_node = trie_aux->nodeselect(p.second);
                cnt = trie_aux->children(cur_node);
                beg = trie_aux->first_child(cur_node);
                end = beg + cnt - 1;
                p = trie_aux->binary_search_seek(m_path_label[0], beg, end);
                // Now we move to m_trie_i using the metatrie concept, hence we use variable trie
                // WARNING
                auto node = trie_aux->nodeselect(p.second); 
                m_status[m_nfixed].it[0] = node;

                trie = m_ptr_index->get_trie(m_trie_i+1);
                size_type temp = (m_status_i == 0)? 1: 0;

                m_status[m_nfixed].it[1] = trie->nodeselect(p.second-trie_aux->root_degree()-1);  
                return node; 
            } 
        } 
    } else if (nfixed == 3) {
        // 3 fixed components, going down by the fourth using state
        if (m_fixed[2] == t) {
            size_type pos_1, pos_2, pos_3;
            // going down to some order ending with T
            if ((m_fixed[0] == s && m_fixed[1] == p) ||
                (m_fixed[0] == p && m_fixed[1] == o) ||
                (m_fixed[0] == o && m_fixed[1] == s)) {
                // SPT, POT, OST
                pos_1 = 2; pos_2 = 0; pos_3 = 1;
            } else {
                // SOT, PST, OPT
                pos_1 = 2; pos_2 = 1; pos_3 = 0;
            }
        
            auto trie_aux = m_ptr_index->get_trie(TSPO); // gets the trie with intervals on level 0
            auto p = trie_aux->binary_search_interval_seek(m_path_label[pos_1], 0, trie_aux->root_degree()-1);
            uint64_t l_update = trie_aux->get_last_update_of_interval(p.second); 
            
            auto cur_node = trie->get_temporal_root();
            std::pair<std::pair<size_type, size_type>, size_type> node_pair;
            value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[pos_2], node_pair);

            cur_node.first = node_pair.first.first;
            cur_node.second = node_pair.first.second;
            l_update = node_pair.second;
            succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[pos_3], node_pair);

            m_status[nfixed].it[0] = node_pair.second;
            m_status[nfixed].it[1] = node_pair.first.first;
            m_status[nfixed].it[2] = node_pair.first.second;
            m_status[nfixed].it[3] = 0;
            return 0; // returns dummy node in this case
        } else if (m_fixed[1] == t) {
            if ((m_fixed[0] == s && m_fixed[2] == o) || 
                (m_fixed[0] == p && m_fixed[2] == s) ||
                (m_fixed[0] == o && m_fixed[2] == p)) {
                // STO, PTS, OTP
                size_type pos_1 = 1, pos_2 = 2, pos_3 = 0;

                auto* trie_aux = m_ptr_index->get_trie(TSPO); 
                auto p = trie_aux->binary_search_interval_seek(m_path_label[pos_1], 0, trie_aux->root_degree()-1);
                uint64_t l_update = trie_aux->get_last_update_of_interval(p.second);

                auto cur_node = trie->get_temporal_root();
                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[pos_2], node_pair);
                
                cur_node.first = node_pair.first.first;
                cur_node.second = node_pair.first.second;
                l_update = node_pair.second;
                succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[pos_3], node_pair);
    
                m_status[nfixed].it[0] = node_pair.second; // position within the current range
                m_status[nfixed].it[1] = node_pair.first.first; // left endpoint of the current range 
                m_status[nfixed].it[2] = node_pair.first.second; // right endpoint of the current range
                m_status[nfixed].it[3] = 0; // succ
                return 0; // returns a dummy node in this case
            } // STP, PTO, and OTP do not need a switch 
        } else if (m_fixed[0] == t) {
            // orders that start with T. Some of them need to switch to a different order for the fourth component
            if ((m_fixed[1] == s && m_fixed[2] == o) || 
                (m_fixed[1] == p && m_fixed[2] == s) ||
                (m_fixed[2] == o && m_fixed[2] == p)) {
                // TSO, TPS, TOP
                size_type pos_1 = 0, pos_2 = 2, pos_3 = 1;

                auto* trie_aux = m_ptr_index->get_trie(TSPO); 
                auto p = trie_aux->binary_search_interval_seek(m_path_label[pos_1], 0, trie_aux->root_degree()-1);
                uint64_t l_update = trie_aux->get_last_update_of_interval(p.second);

                auto cur_node = trie->get_temporal_root();
                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[pos_2], node_pair);
                
                cur_node.first = node_pair.first.first;
                cur_node.second = node_pair.first.second;
                l_update = node_pair.second;
                succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[pos_3], node_pair);
    
                m_status[nfixed].it[0] = node_pair.second; // position within the current range
                m_status[nfixed].it[1] = node_pair.first.first; // left endpoint of the current range 
                m_status[nfixed].it[2] = node_pair.first.second; // right endpoint of the current range
                m_status[nfixed].it[3] = 0; // succ
                return 0; // returns a dummy node in this case
            } // TSP, TPO, and TOS do not need a switch 
        } else {
            // orders that have T at the end (i.e., join first), some of them need to switch to another regular trie
            // In particular, if the order is not SPOT nor PSOT (the latter already switched to SPOT in O), 
            // we need to switch to SPO
            size_type pos_s, pos_p, pos_o;
            if (m_fixed[0] == s && m_fixed[1] == o) {
                pos_s = 0; pos_p = 2; pos_o = 1;
            } else if (m_fixed[0] == p && m_fixed[1] == o) {
                pos_s = 2; pos_p = 0; pos_o = 1;
            } else if (m_fixed[0] == o && m_fixed[1] == s) {
                pos_s = 1; pos_p = 2; pos_o = 0;
            } else if (m_fixed[0] == o && m_fixed[1] == p) {
                pos_s = 2; pos_p = 1; pos_o = 0;
            } 

            auto cnt = trie->root_degree();
            size_type beg, end; 
            beg = trie->first_child(0);
            end = beg + cnt - 1;
            auto p = trie->binary_search_seek(m_path_label[pos_s], beg, end);

            size_type cur_node = trie->nodeselect(p.second);
            cnt = trie->children(cur_node);
            beg = trie->first_child(cur_node);
            end = beg + cnt - 1;
            p = trie->binary_search_seek(m_path_label[pos_p], beg, end);
            
            cur_node = trie->nodeselect(p.second);
            cnt = trie->children(cur_node);
            beg = trie->first_child(cur_node);
            end = beg + cnt - 1;
            p = trie->binary_search_seek(m_path_label[pos_o], beg, end);

            return trie->nodeselect(p.second);            
        }
    }        
}
