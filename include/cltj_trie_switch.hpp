/*struct node_info {
    size_type s;
    size_type e;
    size_type p;
    node_info(size_type _s, size_type _e, size_type _p) {
        s = _s; e = _e; p = _p;
    }
};*/


// Retorna true si tuvo que hacer trie switch y down no necesita hacer nada mas
// Retorna false si no corresponde trie switch, por lo que down tiene que bajar seleccionando el nodo al que baja
size_type trie_switch_down(/*size_type cur_trie,*/ size_type nfixed, state_type state) {
    // NOTA: nfixed ya viene incrementada. E.g., nfixed == 2 significa que 
    // hay 1 componente fija en la posicion 0 de m_fixed, y estamos por fijar la segunda, indicada por state.

    const auto* trie = m_ptr_index->get_trie(m_trie_i);

    if (nfixed == 2) {
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
            m_status[nfixed].it[0] /*beg*/ = node_pair.second; // esta es la posicion dentro del rango actual
            m_status[nfixed].it[1] /*cnt*/ = node_pair.first.first; // extremo izq del rango actual 
            m_status[nfixed].it[2] /*beg*/ = node_pair.first.second; // estremo der del rango actual
            m_status[nfixed].it[3] = 0; // succ
            // WARNING: revisar que el siguiente indice lleve -1 o no
            //m_redo[nfixed-1] = false;
            return 0; // OJO, es un void node, en este caso ya quedo toda la info del nodo en m_status, hay que revisar ese protocolo 
        } else if (m_fixed[0] != t) {
            // currently at SO, SP, PS, PO, OP, OS
            // WARNING, creo que el siguiente if no es necesario
            if ((m_fixed[0] == s && m_fixed[1] == o) ||
                (m_fixed[0] == p && m_fixed[1] == s) ||
                (m_fixed[0] == o && m_fixed[1] == p)) {
                // SO, PS, or OP
                // trie switch to another regular trie
                // first we go down in this trie using the reverse path
                auto* trie_aux = m_ptr_index->get_trie(m_trie_i/*-1*/); 
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
                // Now we move to m_trie_i using the metatrie concept, hence we use variable trie
                // WARNING: revisar la siguiente linea, posible fuente de error
                // WARNING, en este caso estoy guardando directamente aqui los nodos
                // en los iteradores, y no en down como en otros casos. Eso es para
                // evitar retornar un par de valores. Ver como mejorar eso.
                auto node = trie_aux->nodeselect(p.second/*-trie_aux->root_degree()-1*/); 
                m_status[m_nfixed].it[0] = node;

                trie = m_ptr_index->get_trie(m_trie_i+1);
                size_type temp = (m_status_i == 0)? 1: 0;

                m_status[m_nfixed].it[1] = trie->nodeselect(p.second-trie_aux->root_degree()-1/*WARNING, fuente de error*/);  
                return node; 
            } 
        } 
    } else if (nfixed == 3) {
        // habia 3 componente fijas, estamos bajando a la cuarta por state
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

            // WARNING: en el caso de las temporales, cuando hago trie switching voy
            // a intertar dejar la informacion del nodo al que bajo en el stack m_status, y no en el iterador para
            // que leap haga parent(). Revisar que ese mecanismo funcione. Me parece que en este caso,
            // Leap no deberia hacer nada respecto al estado en la pila, creo.
            // WARNING: Revisar que a los siguientes indices nfixed no les falte +1
            m_status[nfixed].it[0] = node_pair.second;
            m_status[nfixed].it[1] = node_pair.first.first;
            m_status[nfixed].it[2] = node_pair.first.second;
            m_status[nfixed].it[3] = 0; // succ
            //m_status[nfixed].beg = node_pair.second; // esta es la posicion dentro del rango actual
            //m_status[nfixed].cnt = node_pair.first.first; // extremo izq del rango actual 
            //m_status[nfixed].beg = node_pair.first.second; // estremo der del rango actual
            // WARNING: revisar que el siguiente indice lleve -1 o no
            //m_redo[nfixed-1] = false;
            return 0; // OJO, revisar esto, es una especie de dummy node en este caso
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
    
                // WARNING: en el caso de las temporales, cuando hago trie switching voy
                // a intertar dejar la informacion del nodo al que bajo en el stack m_status, y no en el iterador para
                // que leap haga parent(). Revisar que ese mecanismo funcione. Me parece que en este caso,
                // Leap no deberia hacer nada respecto al estado en la pila, creo.
                // WARNING: Revisar que a los siguientes indices nfixed no les falte +1
                m_status[nfixed].it[0] /*beg*/ = node_pair.second; // esta es la posicion dentro del rango actual
                m_status[nfixed].it[1] /*cnt*/ = node_pair.first.first; // extremo izq del rango actual 
                m_status[nfixed].it[2] /*beg*/ = node_pair.first.second; // estremo der del rango actual
                m_status[nfixed].it[3] = 0; // succ
                // WARNING: revisar que el siguiente indice lleve -1 o no
                //m_redo[nfixed-1] = false;
                return 0; // OJO, estoy informando que en este caso hice trie switch y down no necesita hacer nada mas
            } // STP, PTO, and OTP do not need a switch 
        } else if (m_fixed[0] == t) {
            // ordenes de nivel 3 con T first que necesitan switch
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
    
                // WARNING: en el caso de las temporales, cuando hago trie switching voy
                // a intertar dejar la informacion del nodo al que bajo en el stack m_status, y no en el iterador para
                // que leap haga parent(). Revisar que ese mecanismo funcione. Me parece que en este caso,
                // Leap no deberia hacer nada respecto al estado en la pila, creo.
                // WARNING: Revisar que a los siguientes indices nfixed no les falte +1
                m_status[nfixed].it[0] /*beg*/ = node_pair.second; // esta es la posicion dentro del rango actual
                m_status[nfixed].it[1] /*cnt*/ = node_pair.first.first; // extremo izq del rango actual 
                m_status[nfixed].it[2] /*beg*/ = node_pair.first.second; // estremo der del rango actual
                m_status[nfixed].it[3] = 0; // succ
                // WARNING: revisar que el siguiente indice lleve -1 o no
                //m_redo[nfixed-1] = false;
                return 0; // OJO, estoy informando que en este caso hice trie switch y down no necesita hacer nada mas
            } // TSP, TPO, and TOS do not need a switch 
        } else {
            // ordenes de nivel 3 que no tienen t en ningun lado
            //  y que necesitan hacer switch a un trie regular
            // If the order is not SPO, we need to go to SPO
            size_type pos_s, pos_p, pos_o;
            if (m_fixed[0] == s && m_fixed[1] == o) {
                pos_s = 0; pos_p = 2; pos_o = 1;
            } /*else if (m_fixed[0] == p && m_fixed[1] == s) {
                pos_s = 1; pos_p = 0; pos_o = 2;
            }*/ else if (m_fixed[0] == p && m_fixed[1] == o) {
                pos_s = 2; pos_p = 0; pos_o = 1;
            } else if (m_fixed[0] == o && m_fixed[1] == s) {
                pos_s = 1; pos_p = 2; pos_o = 0;
            } else if (m_fixed[0] == o && m_fixed[1] == p) {
                pos_s = 2; pos_p = 1; pos_o = 0;
            } else {
                cout << "trie_switch_down() exiting because trying to do trie switch from SPO to SPO" << endl;
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
