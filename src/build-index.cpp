#include <iostream>
#include <cltj_index_temporal_metatrie.hpp>
//#include <cltj_index_spo.hpp>

using namespace std;

using namespace std::chrono;
using timer = std::chrono::high_resolution_clock;

int main(int argc, char **argv){
    try{

        if(argc != 2){
            cout<< argv[0] << " <dataset>" <<endl;
            return 0;
        }

        std::string dataset = argv[1];
        std::string index_name = dataset + ".cltj";
        vector<cltj::spot_quad> D;

        std::ifstream ifs(dataset);
        uint32_t s, p , o, t1, t2;
        cltj::spot_quad spot;
        uint64_t i = 0;
	uint64_t a, b, c;
	ifs >> a;
	ifs >> b;
	ifs >> c;
	cout << "loading graph " << a << " " << b << " " << c << endl;
        ifs >> s >> p >> o >> t1 >> t2;
        do {
            //ifs >> s >> p >> o;
            spot[0] = s; spot[1] = p; spot[2] = o; spot[3] = t1; spot[4] = t2+1;
            D.emplace_back(spot);
            i++;
            if (i%10000000==0) std::cout << i << std::endl;
            ifs >> s >> p >> o >> t1 >> t2;
        } while (!ifs.eof());

        D.shrink_to_fit();
        std::cout << "Dataset: " << D.size() << " tuples " <<  3*D.size()*sizeof(::uint32_t) << " bytes." << std::endl;
        fflush(stdout);
        //sdsl::memory_monitor::start();

        auto start = timer::now();
        cltj::cltj_index_temporal_metatrie<cltj::compact_trie_v3> index(D);
        auto stop = timer::now();

        //sdsl::memory_monitor::stop();

        sdsl::store_to_file(index, index_name);

        cout << "Index saved" << endl;
        cout << duration_cast<seconds>(stop-start).count() << " seconds." << endl;
        cout << /*sdsl::memory_monitor::peak()*/ sdsl::size_in_bytes(index) << " bytes." << endl;
        // ti.indexNewTable(file_name);

        // ind.save();
    }
    catch(const char *msg){
        cerr<<msg<<endl;
    }
    return 0;
}
