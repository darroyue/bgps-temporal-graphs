            choose_trie(state);
            const auto* trie = m_ptr_index->get_trie(m_trie_i);

            if (m_nfixed == 0 && state != t) {
                // we are at S, P, or O
                size_type beg, end;
                auto cnt = trie->children(parent());
                beg = trie->first_child(parent());
                end = beg + cnt -1;
                auto p = trie->binary_search_seek(c, beg, end);
                if (p.second > end or p.first != c) 
                    return false;
                else {
                    m_status[m_nfixed+1].beg = p.second;
                    m_status[m_nfixed+1].end = end;
                    m_status[m_nfixed+1].cnt = cnt;
                    m_redo[m_nfixed] = false;
                    return true;
                }    
            }

            if (m_nfixed == 1 && /*!m_below_time_level ||*/ !at_time_level() && state != t) {
                // We are at S, P, or O, and want to go down by a state that is not T,
                // so we stay in a normal trie
                if (/*m_status_i != 0*/(m_fixed[0] == s && state == o) ||
                    (m_fixed[0] == p && state == s) ||
                    (m_fixed[0] == o && state == p)) {
                    // We are moving to SO, PS, or OP, reusing what we did in the first level of S, P or O (metatrie)
                    size_type beg, end;
                    size_type node = trie->nodeselect(m_status[m_nfixed].beg-1/*parent()*//*? CHEQUEAR!*/);  // OJO aqui con el -1, verificar
                    cout << "En el orden m_trie_i = " << m_trie_i << " node = " << node << " c = " << c << endl;  
                    auto cnt = trie->children(node);
                    beg = trie->first_child(node);
                    end = beg + cnt - 1;
                    auto p = trie->binary_search_seek(c, beg, end);
                    if (p.second > end or p.first != c) return false;
                    m_status[m_nfixed+1].beg = p.second;
                    m_status[m_nfixed+1].end = end;
                    m_status[m_nfixed+1].cnt = cnt;
                    m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                    return true;    
                } else {
                    // We are moving to SP, PO, or OS, so we just go down within the same trie
                    size_type beg, end;
                    //size_type node = trie->nodeselect(m_status[m_nfixed].beg-1);  // OJO aqui con el -1, verificar
                    auto cnt = trie->children(parent()/*node*/);
                    beg = trie->first_child(parent()/*node*/);
                    end = beg + cnt - 1;
                    cout << "beg = " << beg << " end = " << end << " cnt = " << cnt << " c = " << c << endl;
                    auto p = trie->binary_search_seek(c, beg, end);
                    if (p.second > end or p.first != c) return false;
                    m_status[m_nfixed+1].beg = p.second;
                    m_status[m_nfixed+1].end = end;
                    m_status[m_nfixed+1].cnt = cnt;
                    m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                    return true;
                }
            }  

            if (m_nfixed == 3 && state == t) {
                if ((m_fixed[0] == s && m_fixed[1] == p) ||
                    (m_fixed[0] == p && m_fixed[1] == s)) {
                    // we are at SPO or PSO (and then switched to SPO) and want to get down to T
                    //m_below_time_level = true; // we are at the time level, what comes next must be processed in the temporal data structure
                    size_type beg, end;
                    size_type cnt = trie->children(parent()/*OJO con esto!*/); 
                    beg = trie->first_child(parent()/*OJO con esto!*/);
                    end = beg + cnt - 1;
                    cout << "beg = " << beg << " end = " << end << " cnt = " << cnt << " c = " << c << endl;
                    auto p = trie->binary_search_interval_seek(c, beg, end);
                    cout << "p.second = " << p.second << " [" << p.first.first << ", " << p.first.second << "]" << endl; 
                    if (p.second > end or p.first.first != c) return false;
                    m_status[m_nfixed+1].beg = p.second;
                    m_status[m_nfixed+1].end = end;
                    m_status[m_nfixed+1].cnt = cnt;
                    m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                    return true;
                } else {
                    // we have switched to SPOT
                    size_type pos_s, pos_p, pos_o;
                    if (m_fixed[0] == s && m_fixed[1] == o) {
                        // we are in SOP and want to go down by T
                        pos_s = 0; pos_p = 2; pos_o = 1;
                    } else if (m_fixed[0] == o && m_fixed[1] == s) {
                        // we are in OSP and want to go down by T
                        pos_s = 1; pos_p = 2; pos_o = 0;
                    } else if (m_fixed[0] == o && m_fixed[1] == p) {
                        // we are in OPS and want to go down by T
                        pos_s = 2; pos_p = 1; pos_o = 0;
                    } else if (m_fixed[0] == p && m_fixed[1] == o) {
                        // we are in POS and want to go down by T
                        pos_s = 2; pos_p = 0; pos_o = 1;
                    }
                    cout << "El trie es m_trie_i = " << m_trie_i << endl;
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

                    cur_node = trie->nodeselect(p.second);
                    cnt = trie->children(cur_node);
                    beg = trie->first_child(cur_node);
                    end = beg + cnt - 1;

                    cout << "beg = " << beg << " end = " << end << " cnt = " << endl;
                    
                    auto p_aux = trie->binary_search_interval_seek(c, beg, end);
                    cout << "p_aux.second = " << p_aux.second << " [" << p_aux.first.first << ", " << p_aux.first.second << "]" << endl; 
                    if (p_aux.second > end or p_aux.first.first != c) return false;
                    m_status[m_nfixed+1].beg = p_aux.second;
                    m_status[m_nfixed+1].end = end;
                    m_status[m_nfixed+1].cnt = cnt;
                    m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                    return true;
                }
            }

            if (m_nfixed == 2 && !m_below_time_level && state != t) {
                // We are in SO, SP, PO, PS, OP, or OS, and want to go down by something distinct to T
                // So we are going to SOP, SPO, POS, PSO, OPS, or OSP
                // We must determine in wich of these cases we are 
                switch(state) {
                    case s:
                            if (m_fixed[m_nfixed-1/*OJO el -1*/] == o) {
                                // we are in PO and want to go down by S -> POS
                                // we stay at the same trie we are
                                size_type beg, end;
                                auto cnt = trie->children(parent());
                                beg = trie->first_child(parent());
                                end = beg + cnt - 1;
                                auto p = trie->binary_search_seek(c, beg, end);
                                if (p.second > end or p.first != c) return false;
                                m_status[m_nfixed+1].beg = p.second;
                                m_status[m_nfixed+1].end = end;
                                m_status[m_nfixed+1].cnt = cnt;
                                m_redo[m_nfixed] = false;
                                return true;
                            } else {
                                // we are in OP and want to go dwon by S -> OPS
                                // this needes trie switch OP -> PO and then go down by S to get POS
                                //if (m_trie_i != OP) cout << "cuidado en exists() del iterator, trie deberia ser OP y no es" << endl;
                                //trie = m_ptr_index->get_trie(trie_map_2[m_trie_i/*OJO, verificar que sea el trie OP */][state].m_trie); // OJO con esto, verificar que sea el trie correcto: POS
                                //cout << "Estabamos en el trie " << m_trie_i << " y estamos yendo al trie " << (size_type)trie_map_2[m_trie_i][state].m_trie << endl;
                                auto cnt = trie->root_degree();
                                size_type beg, end; 
                                beg = trie->first_child(0);
                                end = beg + cnt - 1;
                                auto p = trie->binary_search_seek(m_path_label[m_nfixed-1], beg, end);

                                size_type cur_node = trie->nodeselect(p.second);
                                cnt = trie->children(cur_node);
                                beg = trie->first_child(cur_node);
                                end = beg + cnt - 1;
                                p = trie->binary_search_seek(m_path_label[m_nfixed-2], beg, end);
                                cur_node = trie->nodeselect(p.second);

                                cnt = trie->children(cur_node);
                                beg = trie->first_child(cur_node);
                                end = beg + cnt - 1;
                                p = trie->binary_search_seek(c, beg, end);
                                if (p.second > end or p.first != c) return false;
                                m_status[m_nfixed+1].beg = p.second;
                                m_status[m_nfixed+1].end = end;
                                m_status[m_nfixed+1].cnt = cnt;
                                m_redo[m_nfixed] = false;
                                return true;
                            } 
                            break;
                    case p: 
                            if (m_fixed[m_nfixed-1/*OJO el -1*/] == s) {
                                // we are in OS and want to go down by P -> OSP
                                // we stay at the same trie we are
                                size_type beg, end;
                                auto cnt = trie->children(parent());
                                beg = trie->first_child(parent());
                                end = beg + cnt - 1;
                                auto p = trie->binary_search_seek(c, beg, end);
                                if (p.second > end or p.first != c) return false;
                                m_status[m_nfixed+1].beg = p.second;
                                m_status[m_nfixed+1].end = end;
                                m_status[m_nfixed+1].cnt = cnt;
                                m_redo[m_nfixed] = false;
                                return true;
                            } else {
                                // we are in SO and want to go down by P -> SOP
                                // this needs trie switch SO -> OS and then go down by P to get OSP
                                // choose_trie already did the trie switch, so m_trie_i must be OSP
                                //if (m_trie_i != OSP) cout << "cuidado en exists() del iterator, trie deberia ser SO y no es m_trie_i = " << m_trie_i << endl;
                                //trie = m_ptr_index->get_trie(trie_map_2[m_trie_i/*OJO, verificar que sea el trie SO */][state].m_trie); // OJO con esto, verificar que sea el trie correcto: OSP
                                auto cnt = trie->root_degree();
                                size_type beg, end; 
                                beg = trie->first_child(0);
                                end = beg + cnt - 1;
                                auto p = trie->binary_search_seek(m_path_label[m_nfixed-1], beg, end);

                                size_type cur_node = trie->nodeselect(p.second);
                                cnt = trie->children(cur_node);
                                beg = trie->first_child(cur_node);
                                end = beg + cnt - 1;
                                p = trie->binary_search_seek(m_path_label[m_nfixed-2], beg, end);
                                cur_node = trie->nodeselect(p.second);

                                cnt = trie->children(cur_node);
                                beg = trie->first_child(cur_node);
                                end = beg + cnt - 1;
                                p = trie->binary_search_seek(c, beg, end);
                                if (p.second > end or p.first != c) return false;
                                m_status[m_nfixed+1].beg = p.second;
                                m_status[m_nfixed+1].end = end;
                                m_status[m_nfixed+1].cnt = cnt;
                                m_redo[m_nfixed] = false;
                                return true;
                            }
                            break;
                    case o:
                            if (m_fixed[m_nfixed-1/*OJO el -1*/] == p) {
                                // we are in SP and want to go down by O -> SPO
                                // we stay at the same trie we are
                                cout << "Yes we reach here 2" << endl;
                                size_type beg, end;
                                auto cnt = trie->children(parent());
                                beg = trie->first_child(parent());
                                end = beg + cnt - 1;
                                auto p = trie->binary_search_seek(c, beg, end);
                                if (p.second > end or p.first != c) return false;
                                m_status[m_nfixed+1].beg = p.second;
                                m_status[m_nfixed+1].end = end;
                                m_status[m_nfixed+1].cnt = cnt;
                                m_redo[m_nfixed] = false;
                                return true;
                            } else {
                                // we are in PS and want to go down by O -> PSO
                                // this needs trie switch PS -> SP and then go down by O to get SPO
                                if (m_trie_i != PS) cout << "cuidado en exists() del iterator, trie deberia ser PS y no es" << endl;
                                trie = m_ptr_index->get_trie(trie_map_2[m_trie_i/*OJO, verificar que sea el trie PS */][state].m_trie); // OJO con esto, verificar que sea el trie correcto: SPO
                                auto cnt = trie->root_degree();
                                size_type beg, end; 
                                beg = trie->first_child(0);
                                end = beg + cnt - 1;
                                auto p = trie->binary_search_seek(m_path_label[m_nfixed-1], beg, end);

                                size_type cur_node = trie->nodeselect(p.second);
                                cnt = trie->children(cur_node);
                                beg = trie->first_child(cur_node);
                                end = beg + cnt - 1;
                                p = trie->binary_search_seek(m_path_label[m_nfixed-2], beg, end);
                                cur_node = trie->nodeselect(p.second);

                                cnt = trie->children(cur_node);
                                beg = trie->first_child(cur_node);
                                end = beg + cnt - 1;
                                p = trie->binary_search_seek(c, beg, end);
                                if (p.second > end or p.first != c) return false;
                                m_status[m_nfixed+1].beg = p.second;
                                m_status[m_nfixed+1].end = end;
                                m_status[m_nfixed+1].cnt = cnt;
                                m_redo[m_nfixed] = false;
                                return true;
                            }
                }
            }

            if (m_nfixed == 0 && state == t) {
                // we are at T, level 0
                cout << "Procesando time T first" << endl;
                //m_below_time_level = true; // we are at the time level, what comes next must be processed in the temporal data structure
                cout << "At trie m_trie_i = " << m_trie_i << endl;
                auto p = trie->binary_search_interval_seek(c, 0, trie->root_degree()-1);
                cout << "p.second = " << p.second << endl;
                if ((uint64_t)p.second > trie->root_degree()-1 /*OJO*/ or p.first.first != c) return false;
                m_status[m_nfixed+1].beg = p.second; // the position within the interval list
                // When we are at the interval time level, end and cnt store the current interval
                // The actual current interval (the one that is stored) could be obtained from beg, just in case
                m_status[m_nfixed+1].end = p.first.second;
                m_status[m_nfixed+1].cnt = p.first.first; // OJO con esto que puede estar mal, revisar
                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                return true;
            }
            
            if (m_nfixed == 1 && at_time_level()/*m_below_time_level*/) {
                // we must go down to level 1 in the temporal ds, being currently at level 0 (the intervals)
                const auto* trie_aux = m_ptr_index->get_trie(TSPO); // TSPO is the only one that has the intervals
                                                             // and the BV with the updates corresponding to each interval

                cout << "Estamos en la temporal en el m_trie_i = " << m_trie_i << endl;
                cout << "m_status[m_nfixed].beg = " << m_status[m_nfixed].beg << endl;
                if (trie != trie_aux) cout << "Son distintos los tries " << (uint64_t)trie << " " << (uint64_t)trie_aux << endl;
                uint64_t l_update = trie_aux->get_last_update_of_interval(m_status[m_nfixed].beg /*OJO*/);
                cout << "l_update = " << l_update << endl;
                auto temporal_root = trie->get_temporal_root();
                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                value_type succ = trie->temporal_successor(m_nfixed-1, temporal_root, l_update, c, node_pair);
                cout << "succ = " << succ << " c = " << c << endl;
                if (succ != c) return false;
                // In this case, beg stores the current position into the temporal ds
                // [cnt, end] is the interval in the temporal ds representing the current node
                m_status[m_nfixed+1].beg = node_pair.second;
                m_status[m_nfixed+1].end = node_pair.first.second;
                m_status[m_nfixed+1].cnt = node_pair.first.first;
                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                return true;
            }

            if (m_nfixed > 1 && m_fixed[0] == t) {
                // We are at level >= 2 in an order that starts with T. So we are in the temporal data structure
                // Current order is TO, TP, TS, TOP, TOS, TPS, TPO, TSO, or TSP and we will extend it
                
                if (m_nfixed == 2) {
                    // TO, TP, or TS, we just extend it, no trie switch in these cases but likely need to use the meta trie
                    // NEW
                    if ((m_fixed[1] == s && state == o) ||
                        (m_fixed[1] == p && state == s) ||
                        (m_fixed[1] == o && state == p)) {
                        // we are descending to TSO, TPS, or TOP,
                        // so we must use the metatrie
                        cout << "Yes, entered over here " << endl;
                        uint64_t l_update = m_status[m_nfixed].beg;
                        std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].cnt, m_status[m_nfixed].end);
                        std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                        value_type succ = trie->temporal_successor(0, cur_node, l_update, c, node_pair);
                        cout << "succ = " << succ << " c = " << c << endl;
                        if (succ != c) return false;
                        m_status[m_nfixed+1].beg = node_pair.second;
                        m_status[m_nfixed+1].end = node_pair.first.second;
                        m_status[m_nfixed+1].cnt = node_pair.first.first;
                        m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                        return true;
                    // END NEW
                    } else {
                        uint64_t l_update = m_status[m_nfixed].beg;
                        std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].cnt, m_status[m_nfixed].end);
                        std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                        cout << "l_update = " << l_update << " cur_node = [" << cur_node.first << ", " << cur_node.second << "] c = " << c << endl; 
                        value_type succ = trie->temporal_successor(m_nfixed-1/*OJO*/, cur_node, l_update, c, node_pair);
                        cout << "succ = " << succ << " c = " << c << endl;
                        if (succ != c) return false;
                        m_status[m_nfixed+1].beg = node_pair.second;
                        m_status[m_nfixed+1].end = node_pair.first.second;
                        m_status[m_nfixed+1].cnt = node_pair.first.first;
                        m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                        return true;
                    }
                }

                if (m_nfixed == 3) {
                    // TOP, TOS, TPS, TPO, TSO, or TSP
                    // In some cases, trie switch is needed
                    if ((m_fixed[m_nfixed-1] == p and state == s) or 
                        (m_fixed[m_nfixed-1] == s and state == o) or
                        (m_fixed[m_nfixed-1] == o and state == p)
                       ) {
                        // current order is TOP, TPS, or TSO, so trie switch is needed
                        // aqui una invariante es trie_map_3[m_trie_i].m_switch == true, sino algo esta mal
                        //OJO trie = m_ptr_index->get_trie(trie_map_3[m_trie_i][state].m_trie); // trie switch
                        
                        // Now, that we have switched, we must get down using the reverse path (of length 3)
                        
                        // El tema es que debo bajar completo por el trie iniciando desde T, lo cual esta al
                        // nivel 0 de la pila m_status
                        // OJO, revisar lo siguiente, posible fuente de error
                        auto* trie_aux = m_ptr_index->get_trie(TSPO);
                        //auto p_aux = trie_aux->binary_search_interval_seek(m_path_label[0], 0, trie_aux->root_degree()-1);
                        //cout << "m_status[1].beg = " << m_status[1].beg << " m_path_label[0] = " << m_path_label[0] << " p_aux.second = " << p_aux.second << endl; 
                        uint64_t l_update = trie_aux->get_last_update_of_interval(/*p_aux.second*/ m_status[1].beg /*OJO, revisar que alli haya guardado el ultimo instante del intervalo*/);
                        auto cur_node = trie->get_temporal_root();
                        std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                        cout << "en m_trie_i = " << m_trie_i << " l_update = " << l_update << endl;
                        cout << "cur_node = [" << cur_node.first << ", " << cur_node.second << "]" << endl;
                        value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[m_nfixed-1/*OJO*/], node_pair);
                        cout << "succ = " << succ << " elemento buscado = " << m_path_label[m_nfixed-1/*OJO*/] << endl;
                        cur_node.first = node_pair.first.first;
                        cur_node.second = node_pair.first.second;
                        l_update = node_pair.second;
                        succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[m_nfixed-2/*OJO*/], node_pair);
                        cout << "succ = " << succ << " elemento buscado = " << m_path_label[m_nfixed-2/*OJO*/] << endl;
                        //now, we are ready to go down by c
                        cur_node.first = node_pair.first.first;
                        cur_node.second = node_pair.first.second;
                        l_update = node_pair.second;
                        succ = trie->temporal_successor(2, cur_node, l_update, c, node_pair);
                        cout << "succ = " << succ << " c = " << c << endl;
                        if (succ != c) return false;
                        m_status[m_nfixed+1].beg = node_pair.second;
                        m_status[m_nfixed+1].end = node_pair.first.second;
                        m_status[m_nfixed+1].cnt = node_pair.first.first;
                        m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                        return true;
                    } else {
                        // no trie switch is needed, just go down
                        uint64_t l_update = m_status[m_nfixed].beg;
                        std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].cnt, m_status[m_nfixed].end);
                        std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                        value_type succ = trie->temporal_successor(m_nfixed-1/*OJO*/, cur_node, l_update, c, node_pair);
                        cout << "succ = " << succ << " c = " << c << endl;
                        if (succ != c) return false;
                        m_status[m_nfixed+1].beg = node_pair.second;
                        m_status[m_nfixed+1].end = node_pair.first.second;
                        m_status[m_nfixed+1].cnt = node_pair.first.first;
                        m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                        return true;
                    }
                }
            }

            if (m_nfixed == 1 && state == t) {
                // we are in ST, PT, or OT
                // Tengo que usar el nodo actual en el trie actual (estoy en el primer nivel de esos tries y
                // quiero bajar al segundo nivel usando T) para obtener el nodo actual en el nuevo trie (segundo
                // nivel). Luego tengo que hacer busqueda binaria sobre los intervalos de ese trie
                // en m_status[m_nfixed].beg creo que esta el nodo padre que necesito, al menos en los otros casos
                // hace nodeselect con eso. Estudiar como usar eso para acceder al nodo adecuado.
                //m_below_time_level = true; // we are at the time level, what comes next must be processed in the temporal data structure
                size_type beg, end;
                cout << "m_status[m_nfixed].beg = " << m_status[m_nfixed].beg << endl; 
                size_type node = trie->nodeselect(m_status[m_nfixed].beg-1);  // OJO aqui con el -1, verificar
                auto cnt = trie->children(node);
                beg = trie->first_child(node);
                end = beg + cnt - 1;
                cout << "c = " << c << " beg = " << beg << " end = " << end << endl;
                auto p = trie->binary_search_interval_seek(c, beg, end);
                cout << "p.second = " << p.second << endl;
                if (p.second > end or p.first.first != c) return false;
                m_status[m_nfixed+1].beg = p.second;
                m_status[m_nfixed+1].end = end;
                m_status[m_nfixed+1].cnt = cnt;
                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                return true;
            }

            if (m_nfixed == 2 && state == t) {
                if ((m_fixed[0] == s && m_fixed[1] == o) ||
                    (m_fixed[0] == p && m_fixed[1] == s) ||
                    (m_fixed[0] == o && m_fixed[1] == p)) {
                    // We are in SO, PS, or OP, and want to go down by T
                    // We need to do trie switching first
                    // Actually, choose_trie did the trie switching 
                    //m_below_time_level = true;

                    auto* trie_aux = m_ptr_index->get_trie(m_trie_i-1); // first we go down in this trie using the reverse path
                    auto cnt = trie_aux->root_degree();
                    size_type beg, end;
                    beg = trie_aux->first_child(0);
                    end = beg + cnt - 1;
                    auto p = trie_aux->binary_search_seek(m_path_label[m_nfixed-1], beg, end);
                    size_type cur_node = trie_aux->nodeselect(p.second);
                    cout << "En el nodo " << cur_node << " del trie " << m_trie_i-1 << endl;
                    cnt = trie_aux->children(cur_node);
                    beg = trie_aux->first_child(cur_node);
                    end = beg + cnt - 1;
                    p = trie_aux->binary_search_seek(m_path_label[m_nfixed-2], beg, end);
                    // Now we move to m_trie_i usint the metatrie concept, hence we use variable trie
                    cout << "p.second = " << p.second << endl;
                    cur_node = trie->nodeselect(p.second-trie_aux->root_degree()-1/* cur_node-1*/ /*beg - (end-cnt+1)*//*OJO, revisar esta formula!!*/);
                    cnt = trie->children(cur_node);
                    beg = trie->first_child(cur_node);
                    end = beg + cnt - 1;
                    cout << "beg = " << beg << " end = " << end << " cnt = " << cnt << " c = " << c << endl;
                    auto p2 = trie->binary_search_interval_seek(c, beg, end);
                    cout << "p2.second = " << p2.second << "[" << p2.first.first << ", " << p2.first.second << "]" << endl;
                    if (p2.second > end or p2.first.first != c)
                        return false;
                    else {
                        m_status[m_nfixed+1].beg = p2.second;
                        m_status[m_nfixed+1].end = end;
                        m_status[m_nfixed+1].cnt = cnt;
                        m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                        return true;
                    }    
                } else {
                    // We are at SPT, OST, or POT
                    size_type beg, end; 
                    
                    //m_below_time_level = true; // we are at the time level, swhat comes next must be processed in the temporal data structure

                    // A continuacion tengo que seleccionar el nodo teniendo en cuenta que al trie al que me muevo
                    // estoy en el nivel 2, pero en ese trie es el nivel 0. Tengo que usar la posicion del nodo entre
                    // los hijos del padre para hacer el selectnode.
                    // m_status[m_nfixed].beg - (m_status[m_nfixed].end - m_status[m_nfixed].cnt), o algo asi es
                    // el valor al que tengo que hacerle selectnode, depurarlo bien porque puede faltar +-1
                    auto* trie_aux = m_ptr_index->get_trie(m_trie_i-1);
                    cout << "En orden m_trie_i =  " << m_trie_i << " " << m_status[m_nfixed].beg - (m_status[m_nfixed].end - m_status[m_nfixed].cnt)  << endl;
                    size_type node = trie->nodeselect(m_status[m_nfixed].beg - trie_aux->root_degree()-1); 
                    cout << "node = " << node << endl;
                    auto cnt = trie->children(node);
                    beg = trie->first_child(node);
                    end = beg + cnt - 1;
                    cout << "beg = " << beg << " end = " << end << " cnt = " << cnt << " c = " << c << endl;
                    auto p = trie->binary_search_interval_seek(c, beg, end);
                    cout << "p.second = " << p.second << "[" << p.first.first << ", " << p.first.second << "]" << endl;
                    if (p.second > end or p.first.first != c)
                        return false;
                    else {
                        m_status[m_nfixed+1].beg = p.second;
                        m_status[m_nfixed+1].end = end;
                        m_status[m_nfixed+1].cnt = cnt;
                        m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                        return true;
                    }
                }
            }

            if (m_nfixed == 2 && m_fixed[m_nfixed-1] == t) {
                // We are in a normal trie at level 2 corresponding to T
                // So we are at ST, PT, or OT
                // We need to do trie switch and move to some of the orders 
                // that is represented with a temporal data structure (as it starts with T)
                //trie = m_ptr_index->get_trie(trie_map_2[m_trie_i][state].m_trie); // trie switch to the temporal ds
                // Ahora, hay que buscar en los intervalos, luego bajar dos niveles en la ds temporal
                auto* trie_aux = m_ptr_index->get_trie(TSPO); 
                auto p = trie_aux->binary_search_interval_seek(m_path_label[m_nfixed-1/*OJO el -1*/], 0, trie_aux->root_degree()-1);
                uint64_t l_update = trie_aux->get_last_update_of_interval(p.second); 
                auto cur_node = trie->get_temporal_root();
                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[m_nfixed-2/*OJO el -2*/], node_pair);
                //now, we are ready to go down by c
                cur_node.first = node_pair.first.first;
                cur_node.second = node_pair.first.second;
                l_update = node_pair.second;
                succ = trie->temporal_successor(1, cur_node, l_update, c, node_pair);
                if (succ != c) return false;
                m_status[m_nfixed+1].beg = node_pair.second;
                m_status[m_nfixed+1].end = node_pair.first.second;
                m_status[m_nfixed+1].cnt = node_pair.first.first;
                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                return true;
            }

            if (m_nfixed == 3 && m_fixed[2] == t) {
                // We are in a normal trie at level 3 corresponding to T
                size_type pos1, pos2, pos3;
                if ((m_fixed[0] == s && m_fixed[1] == o) ||
                    (m_fixed[0] == p && m_fixed[1] == s) ||
                    (m_fixed[0] == o && m_fixed[1] == p)
                   ) {
                    // we are in the cases SOT, PST, OPT, which are no explicitly represented,
                    // so we need first to do trie switching and then using the meta trie concept
                    pos1 = 2; pos2 = 1; pos3 = 0;
                } else {
                    // So we are at SPT, POT, or OST
                    pos1 = 2; pos2 = 0; pos3 = 1;
                }
                // We need to do trie switch and move to some of the orders 
                // that is represented with a temporal data structure (as it starts with T)
                //trie = m_ptr_index->get_trie(trie_map_3[m_trie_i][state].m_trie); // trie switch to the temporal ds
                // Ahora, hay que buscar en los intervalos, luego bajar dos niveles en la ds temporal
                cout << "m_trie_i = " << m_trie_i << endl;
                cout << "m_path_label[m_nfixed-1] = " << m_path_label[pos1/*m_nfixed-1*/] << "[0, " << trie->root_degree()-1 << "]" << endl;
                auto trie_aux = m_ptr_index->get_trie(TSPO); // gets the trie with intervals on level 0
                auto p = trie_aux->binary_search_interval_seek(m_path_label[pos1/*m_nfixed-1*/], 0, trie_aux->root_degree()-1);
                cout << "p.second = " << p.second << endl;
                uint64_t l_update = trie_aux->get_last_update_of_interval(p.second); 
                cout << "l_update = " << l_update << endl;
                
                auto cur_node = trie->get_temporal_root();
                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                cout << "m_path_label[m_nfixed-3] = " << m_path_label[pos2/*m_nfixed-3*/] << endl;
                value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[pos2/*m_nfixed-3*/], node_pair);
                cout << "succ = " << succ << " searching = " << m_path_label[pos2] << endl; 
                cur_node.first = node_pair.first.first;
                cur_node.second = node_pair.first.second;
                l_update = node_pair.second;
                cout << "m_path_label[m_nfixed-2] = " << m_path_label[pos3/*m_nfixed-2*/] << endl;
                succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[pos3/*m_nfixed-2*/], node_pair);
                cout << "succ = " << succ << endl;
                //now, we are ready to go down by c
                cur_node.first = node_pair.first.first;
                cur_node.second = node_pair.first.second;
                l_update = node_pair.second;
                succ = trie->temporal_successor(2, cur_node, l_update, c, node_pair);
                cout << "succ = " << succ << " c = " << c << endl;
                if (succ != c) return false;
                m_status[m_nfixed+1].beg = node_pair.second;
                m_status[m_nfixed+1].end = node_pair.first.second;
                m_status[m_nfixed+1].cnt = node_pair.first.first;
                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                return true;
            }

            if (m_nfixed == 3 && m_fixed[1] == t) {
                // We are at STO, STP, PTS, PTO, OTS, or OTP, and want to go down
                // by the fourth component
                // In all cases we have already carried out trie switching, and in some
                // cases we must carry out a second trie switching
                switch (m_fixed[0]) {
                    case s: if (m_fixed[2] == o) {
                                // currently at STO, descend by P to STOP
                                // We must switch again to TOSP
                                auto* trie_aux = m_ptr_index->get_trie(TSPO); 
                                auto p = trie_aux->binary_search_interval_seek(m_path_label[1]/*T*/, 0, trie_aux->root_degree()-1);
                                uint64_t l_update = trie_aux->get_last_update_of_interval(p.second);
                
                                auto cur_node = trie->get_temporal_root();
                                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                                value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[2] /*O*/, node_pair);
                                cout << "succ = " << succ << " label o = " << m_path_label[2] << endl;
                                cur_node.first = node_pair.first.first;
                                cur_node.second = node_pair.first.second;
                                l_update = node_pair.second;
                                succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[0]/*S*/, node_pair);
                                cout << "succ = " << succ << " label s = " << m_path_label[0] << endl;
                                //now, we are ready to go down by c
                                cur_node.first = node_pair.first.first;
                                cur_node.second = node_pair.first.second;
                                l_update = node_pair.second;
                                succ = trie->temporal_successor(2, cur_node, l_update, c, node_pair);
                                if (succ != c) return false;
                                cout << "succ = " << succ << " c = " << c << endl;
                                m_status[m_nfixed+1].beg = node_pair.second;
                                m_status[m_nfixed+1].end = node_pair.first.second;
                                m_status[m_nfixed+1].cnt = node_pair.first.first;
                                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                                return true;
                            } else {
                                uint64_t l_update = m_status[m_nfixed].beg;
                                std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].cnt, m_status[m_nfixed].end);
                                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                                value_type succ = trie->temporal_successor(m_nfixed-1/*OJO*/, cur_node, l_update, c, node_pair);
                                if (succ != c) return false;
                                cout << "succ = " << succ << " c = " << c << endl;
                                m_status[m_nfixed+1].beg = node_pair.second;
                                m_status[m_nfixed+1].end = node_pair.first.second;
                                m_status[m_nfixed+1].cnt = node_pair.first.first;
                                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                                return true;
                            }
                            break;
                    case p: if (m_fixed[2] == s) {
                                // currently at PTS, descend by O to PTSO
                                // We must switch again to TSPO
                                
                                auto* trie_aux = m_ptr_index->get_trie(TSPO); // TSPO 
                                auto p = trie_aux->binary_search_interval_seek(m_path_label[1]/*T*/, 0, trie_aux->root_degree()-1);
                                uint64_t l_update = trie_aux->get_last_update_of_interval(p.second);
                
                                auto cur_node = trie->get_temporal_root();
                                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                                value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[2] /*S*/, node_pair);
                                cout << "succ = " << succ << " label s = " << m_path_label[2] << endl;
                                cur_node.first = node_pair.first.first;
                                cur_node.second = node_pair.first.second;
                                l_update = node_pair.second;
                                succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[0]/*P*/, node_pair);
                                cout << "succ = " << succ << " label p = " << m_path_label[0] << endl;
                                //now, we are ready to go down by c
                                cur_node.first = node_pair.first.first;
                                cur_node.second = node_pair.first.second;
                                l_update = node_pair.second;
                                succ = trie->temporal_successor(2, cur_node, l_update, c, node_pair);
                                if (succ != c) return false;
                                cout << "succ = " << succ << " c = " << c << endl;
                                m_status[m_nfixed+1].beg = node_pair.second;
                                m_status[m_nfixed+1].end = node_pair.first.second;
                                m_status[m_nfixed+1].cnt = node_pair.first.first;
                                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                                return true;
                            } else {
                                uint64_t l_update = m_status[m_nfixed].beg;
                                std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].cnt, m_status[m_nfixed].end);
                                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                                value_type succ = trie->temporal_successor(m_nfixed-1/*OJO*/, cur_node, l_update, c, node_pair);
                                if (succ != c) return false;
                                cout << "succ = " << succ << " c = " << c << endl;
                                m_status[m_nfixed+1].beg = node_pair.second;
                                m_status[m_nfixed+1].end = node_pair.first.second;
                                m_status[m_nfixed+1].cnt = node_pair.first.first;
                                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                                return true;
                            }
                            break;
                    case o: if (m_fixed[2] == p) {
                                // currently at OTP, descend by S to OTPS
                                // We must switch again to TPOS
                                auto* trie_aux = m_ptr_index->get_trie(TSPO); 
                                auto p = trie_aux->binary_search_interval_seek(m_path_label[1]/*T*/, 0, trie_aux->root_degree()-1);
                                uint64_t l_update = trie_aux->get_last_update_of_interval(p.second);
                
                                auto cur_node = trie->get_temporal_root();
                                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                                value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[2] /*P*/, node_pair);
                                cout << "succ = " << succ << " label p = " << m_path_label[2] << endl;
                                cur_node.first = node_pair.first.first;
                                cur_node.second = node_pair.first.second;
                                l_update = node_pair.second;
                                succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[0]/*O*/, node_pair);
                                cout << "succ = " << succ << " label o = " << m_path_label[0] << endl;
                                //now, we are ready to go down by c
                                cur_node.first = node_pair.first.first;
                                cur_node.second = node_pair.first.second;
                                l_update = node_pair.second;
                                succ = trie->temporal_successor(2, cur_node, l_update, c, node_pair);
                                if (succ != c) return false;
                                cout << "succ = " << succ << " c = " << c << endl;
                                m_status[m_nfixed+1].beg = node_pair.second;
                                m_status[m_nfixed+1].end = node_pair.first.second;
                                m_status[m_nfixed+1].cnt = node_pair.first.first;
                                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                                return true;
                            } else {
                                uint64_t l_update = m_status[m_nfixed].beg;
                                std::pair<size_type, size_type> cur_node = make_pair(m_status[m_nfixed].cnt, m_status[m_nfixed].end);
                                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                                value_type succ = trie->temporal_successor(m_nfixed-1/*OJO*/, cur_node, l_update, c, node_pair);
                                if (succ != c) return false;
                                cout << "succ = " << succ << " c = " << c << endl;
                                m_status[m_nfixed+1].beg = node_pair.second;
                                m_status[m_nfixed+1].end = node_pair.first.second;
                                m_status[m_nfixed+1].cnt = node_pair.first.first;
                                m_redo[m_nfixed] = false; // OJO con esta linea, revisar
                                return true;
                            }
                            break;
                }   
            }

