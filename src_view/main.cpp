

#include <ctime>
#include <iostream>
#include <include/sealib/graph.h>
#include <src_view/graphio.h>
#include <include/sealib/rankselect.h>
#include <include/sealib/graphalgorithms.h>

int main() {
    clock_t start = clock();
    Sealib::Graph *g = Sealib::GraphIO::randomEulerianSealibGraph(40000,2500000);
    clock_t end = clock();
    std::cout << "GENERATION: " << (end - start)/CLOCKS_PER_SEC << "s" << std::endl;
    start = clock();
    Sealib::TrailStructure **ts = Sealib::GraphAlgorithms::hierholzer(g);
    end = clock();
    std::cout << "HIERHOLZER: " << (end - start)/CLOCKS_PER_SEC << "s" << std::endl;
    start = clock();
    Sealib::GraphIO::stringFromTrail(g, ts);
    end = clock();
    std::cout << "STRING: " << (end - start)/CLOCKS_PER_SEC << "s" << std::endl;
}

