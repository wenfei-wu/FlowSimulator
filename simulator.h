#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "topology.h"
#include "workload.h"

using namespace std;


class Simulator{
public:
    Topology* topology;
    Workload* workload;
    double globalTime;

    Simulator(Topology* topology, Workload* workload) : topology(topology), workload(workload) { globalTime=0; }

    void run();
};

#endif // SIMULATOR_H