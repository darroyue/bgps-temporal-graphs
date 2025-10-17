// Retorna true si tuvo que hacer trie switch y down no necesita hacer nada mas
// Retorna false si no corresponde trie switch, por lo que down tiene que bajar seleccionando el nodo al que baja
bool trie_switch_down(/*size_type cur_trie,*/ size_type nfixed, state_type state) {
    // NOTA: nfixed ya viene incrementada. E.g., nfixed == 3 significa que 
    // hay 2 componentes fijas en las posiciones 0 y 1 de m_fixed, y estamos por fijar la tercera.

    // if nfixed == 1 or nfixed == 2, it only chooses (likely) a new m_trie_i

    //choose_trie(state);
    const auto* trie = m_ptr_index->get_trie(m_trie_i);

    if (nfixed == 3) {
        // habia dos componentes fijas, y estamos bajando a la tercera por state
        // We must carry out a switch if we were at ST, SO, PT, PS, OT, OP
        if (m_fixed[1] == t) {
            // ST, PT, OT
            // En este caso, choose_trie() ya nos ubico en el trie correspondiente que empieza con T
            // asi que la siguiente es una invariante que pongo mientras pruebo
            if (!(TSPO <= m_trie_i && m_trie_i <= TOP)) {
                cout << "Error 1 en trie_switch m_trie_i = " << m_trie_i << endl;
                exit(1);
            }

            // We are in a normal trie at level 2 corresponding to T
            // So we are at ST, PT, or OT
            // We need to do trie switch and move to some of the orders 
            // that is represented with a temporal data structure (as it starts with T)
            // Hay que buscar en los intervalos, luego bajar dos niveles en la ds temporal
            auto* trie_aux = m_ptr_index->get_trie(TSPO); 
            auto p = trie_aux->binary_search_interval_seek(m_path_label[1], 0, trie_aux->root_degree()-1);

            uint64_t l_update = trie_aux->get_last_update_of_interval(p.second); 
            auto cur_node = trie->get_temporal_root();
            std::pair<std::pair<size_type, size_type>, size_type> node_pair;
            value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[0], node_pair);
            
            //cur_node.first = node_pair.first.first;
            //cur_node.second = node_pair.first.second;
            //l_update = node_pair.second;
            
            // WARNING: en el caso de las temporales, cuando hago trie switching voy
            // a intertar dejar la informacion del nodo al que bajo en el stack m_status, y no en el iterador para
            // que leap haga parent(). Revisar que ese mecanismo funcione. Me parece que en este caso,
            // Leap no deberia hacer nada respecto al estado en la pila, creo.
            // WARNING: Revisar que a los siguientes indices nfixed no les falte +1
            m_status[nfixed].beg = node_pair.second; // esta es la posicion dentro del rango actual
            m_status[nfixed].cnt = node_pair.first.first; // extremo izq del rango actual 
            m_status[nfixed].beg = node_pair.first.second; // estremo der del rango actual
            // WARNING: revisar que el siguiente indice lleve -1 o no
            m_redo[nfixed-1] = false;
            return true; // OJO, estoy informando que en este caso hice trie switch y down no necesita hacer nada mas
        } else if (m_fixed[0] != t) {
            // currently at SO, SP, PS, PO, OP, OS
            if ((m_fixed[0] == s && m_fixed[1] == o) ||
                (m_fixed[0] == p && m_fixed[1] == s) ||
                (m_fixed[0] == o && m_fixed[1] == p)) {
                // SO, PS, or OP
                if (state = t) {
                    // going down to SOT, PST, or OPT
                    // trie switch to some of these regular tries
                    // first we go down in this trie using the reverse path
                    auto* trie_aux = m_ptr_index->get_trie(m_trie_i-1); 
                    auto cnt = trie_aux->root_degree();
                    size_type beg, end;
                    beg = trie_aux->first_child(0);
                    end = beg + cnt - 1;
                    auto p = trie_aux->binary_search_seek(m_path_label[1 /*OJO*/], beg, end);

                    size_type cur_node = trie_aux->nodeselect(p.second);
                    cnt = trie_aux->children(cur_node);
                    beg = trie_aux->first_child(cur_node);
                    end = beg + cnt - 1;
                    p = trie_aux->binary_search_seek(m_path_label[0 /*OJO*/], beg, end);

                    // Now we move to m_trie_i usint the metatrie concept, hence we use variable trie
                    // WARNING: revisar la siguiente linea, posible fuente de error
                    m_status[nfixed].it[0] = trie->nodeselect(p.second-trie_aux->root_degree()-1);
                    return true;    
                } else {
                    // going down to SOP, PSO, or OPS
                    // trie switch to a regular trie, either OSP, SPO, or POS
                    // choose_trie() already got one of these target tries
                    auto cnt = trie->root_degree();
                    size_type beg, end; 
                    beg = trie->first_child(0);
                    end = beg + cnt - 1;
                    auto p = trie->binary_search_seek(m_path_label[1], beg, end);

                    size_type cur_node = trie->nodeselect(p.second);
                    cnt = trie->children(cur_node);
                    beg = trie->first_child(cur_node);
                    end = beg + cnt - 1;
                    p = trie->binary_search_seek(m_path_label[0], beg, end);
                    // WARNING: revisar que lo siguiente este correcto
                    m_status[nfixed].it[0] = trie->nodeselect(p.second);
                    return true;
                }
            } else {
                // SP, PO, OS
                if (state == t) {
                    // going down to SPT, POT, OST, no trie switch needed
                    return false;
                } else {
                    // going down to SPO, POS, OSP, no trie switch needed
                    return false;
                }
            }
        } else {
            // We are at TO, TP or TS and going down, no trie switch needed
            return false;
        }
    } else if (nfixed == 4) {
        // habia 3 componente fijas, estamos bajando a la cuarta por state
        if (state == t) {
            // going down to some order ending with T
            // SPOT, SOPT, PSOT, POST, OSPT, OPST
            if ((m_fixed[0] == s && m_fixed[1] == p) ||
                (m_fixed[0] == p && m_fixed[1] == s)) {
                return false; // no switch needed in this case
            } else {
                // we must switch to SPOT (in some cases, it will be the second switch) 
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

                m_status[nfixed].it[0] = trie->nodeselect(p.second);
                return true;
            }
        } else {
            // T is one of the fixed components, lets check all possibilities
            if (m_fixed[0] == t) {
                // El orden comienza con T, estamos en una estructura temporal
                if ((m_fixed[2] == s and state == o) or 
                    (m_fixed[2] == p and state == s) or
                    (m_fixed[2] == o and state == p)) {
                    // current order is TOP, TPS, or TSO, so trie switch is needed
                    auto* trie_aux = m_ptr_index->get_trie(TSPO);
                    uint64_t l_update = trie_aux->get_last_update_of_interval(m_status[1].beg);
                    auto cur_node = trie->get_temporal_root();
                    std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                    value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[2], node_pair);

                    cur_node.first = node_pair.first.first;
                    cur_node.second = node_pair.first.second;
                    l_update = node_pair.second;
                    succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[1], node_pair);

                    // WARNING: en el caso de las temporales, cuando hago trie switching voy
                    // a intertar dejar la informacion del nodo al que bajo en el stack m_status, y no en el iterador para
                    // que leap haga parent(). Revisar que ese mecanismo funcione. Me parece que en este caso,
                    // Leap no deberia hacer nada respecto al estado en la pila, creo.
                    // WARNING: Revisar que a los siguientes indices nfixed no les falte +1
                    m_status[nfixed].beg = node_pair.second; // esta es la posicion dentro del rango actual
                    m_status[nfixed].cnt = node_pair.first.first; // extremo izq del rango actual 
                    m_status[nfixed].beg = node_pair.first.second; // estremo der del rango actual
                    // WARNING: revisar que el siguiente indice lleve -1 o no
                    m_redo[nfixed-1] = false;
                    return true; // OJO, estoy informando que en este caso hice trie switch y down no necesita hacer nada mas
                } else {
                    // We are moving to TSPO, TPOS, or TOSP, no switch needed
                    return false;
                }
            } else if (m_fixed[1] == t) {
                // orden no comienza con T
                // We are at STO, STP, PTS, PTO, OTS, or OTP, and want to go down
                // by the fourth component
                // In all cases we have already carried out trie switching, and in some
                // cases we must carry out a second trie switching
                size_type pos1, pos2, pos3;
                if ((m_fixed[0] == s && m_fixed[2] == o) ||
                    (m_fixed[0] == p && m_fixed[2] == s) ||
                    (m_fixed[0] == o && m_fixed[2] == p)) {
                    // STO, PTS, or OTP
                    pos1 = 1; pos2 = 2; pos3 = 0;

                    auto* trie_aux = m_ptr_index->get_trie(TSPO); 
                    auto p = trie_aux->binary_search_interval_seek(m_path_label[pos1], 0, trie_aux->root_degree()-1);
                    uint64_t l_update = trie_aux->get_last_update_of_interval(p.second);
    
                    auto cur_node = trie->get_temporal_root();
                    std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                    value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[pos2], node_pair);
                    
                    cur_node.first = node_pair.first.first;
                    cur_node.second = node_pair.first.second;
                    l_update = node_pair.second;
                    succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[pos3], node_pair);
        
                    // WARNING: en el caso de las temporales, cuando hago trie switching voy
                    // a intertar dejar la informacion del nodo al que bajo en el stack m_status, y no en el iterador para
                    // que leap haga parent(). Revisar que ese mecanismo funcione. Me parece que en este caso,
                    // Leap no deberia hacer nada respecto al estado en la pila, creo.
                    // WARNING: Revisar que a los siguientes indices nfixed no les falte +1
                    m_status[nfixed].beg = node_pair.second; // esta es la posicion dentro del rango actual
                    m_status[nfixed].cnt = node_pair.first.first; // extremo izq del rango actual 
                    m_status[nfixed].beg = node_pair.first.second; // estremo der del rango actual
                    // WARNING: revisar que el siguiente indice lleve -1 o no
                    m_redo[nfixed-1] = false;
                    return true; // OJO, estoy informando que en este caso hice trie switch y down no necesita hacer nada mas
                } else {
                    // STPO, PTOS, OTSP, no trie switching needed this time, switch already carried out in a previous step in this order
                    return false;
                }
            } else if (m_fixed[2] == t) {
                // We are in a normal trie at level 3 corresponding to T
                // SOT, OST, PST, SPT, OPT, POT
                size_type pos1, pos2, pos3;
                if ((m_fixed[0] == s && m_fixed[1] == o) ||
                    (m_fixed[0] == p && m_fixed[1] == s) ||
                    (m_fixed[0] == o && m_fixed[1] == p)) {
                    // we are in the cases SOT, PST, OPT, 
                    pos1 = 2; pos2 = 1; pos3 = 0;
                } else {
                    // So we are at SPT, POT, or OST
                    pos1 = 2; pos2 = 0; pos3 = 1;
                }
                auto trie_aux = m_ptr_index->get_trie(TSPO); // gets the trie with intervals on level 0
                auto p = trie_aux->binary_search_interval_seek(m_path_label[pos1], 0, trie_aux->root_degree()-1);
                uint64_t l_update = trie_aux->get_last_update_of_interval(p.second); 
                
                auto cur_node = trie->get_temporal_root();
                std::pair<std::pair<size_type, size_type>, size_type> node_pair;
                value_type succ = trie->temporal_successor(0, cur_node, l_update, m_path_label[pos2], node_pair);

                cur_node.first = node_pair.first.first;
                cur_node.second = node_pair.first.second;
                l_update = node_pair.second;
                succ = trie->temporal_successor(1, cur_node, l_update, m_path_label[pos3], node_pair);

                // WARNING: en el caso de las temporales, cuando hago trie switching voy
                // a intertar dejar la informacion del nodo al que bajo en el stack m_status, y no en el iterador para
                // que leap haga parent(). Revisar que ese mecanismo funcione. Me parece que en este caso,
                // Leap no deberia hacer nada respecto al estado en la pila, creo.
                // WARNING: Revisar que a los siguientes indices nfixed no les falte +1
                m_status[nfixed].beg = node_pair.second; // esta es la posicion dentro del rango actual
                m_status[nfixed].cnt = node_pair.first.first; // extremo izq del rango actual 
                m_status[nfixed].beg = node_pair.first.second; // estremo der del rango actual
                // WARNING: revisar que el siguiente indice lleve -1 o no
                m_redo[nfixed-1] = false;
                return true; // OJO, estoy informando que en este caso hice trie switch y down no necesita hacer nada mas
            } 
        }
    }

    if (nfixed == 3 || nfixed == 4) {
        cout << "Cuidado, hay casos sin definir en trie_switch_down(), exiting the process..." << endl;
        exit(1);
    }
    return false; 
}