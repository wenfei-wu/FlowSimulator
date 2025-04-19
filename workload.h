#ifndef WORKLOAD_H
#define WORKLOAD_H
#include "topology.h"
#include <vector>
#include <queue>
#include <random>
#include <iostream>

using namespace std;

class Node;
class Link;
class Topology;

enum FlowState {
    NOTSTART,
    RUN,
    COMPLETE
};

class Flow {
public:
    int id;
    Node* src; 
    Node* dst;
    double dataSize;
    double startTime;
    FlowState state;

    vector<Node*> path;
    vector<Link*> path_links;


    double throughput;
    double remainingDataSize;


    Flow(int id, Node* src, Node* dst, double dataSize, double startTime) : id(id),
            src(src), dst(dst), dataSize(dataSize), startTime(startTime),state(NOTSTART) {}
    void print();

    void routingECMP(Topology* topology);

    double stableTime(double globalTime);
    void progress(double globalTime, double deltaTime);
    void printState() {
        cout << "------------Flow--------------------" << endl;
        print();
        if(state != RUN) {
            cout << "Flow is not running." << endl;
            return;
        }
        cout << "Flow State: ";
        switch(state) {
            case NOTSTART: cout << "NOTSTART"; break;
            case RUN: cout << "RUN"; break;
            case COMPLETE: cout << "COMPLETE"; break;
        }
        cout << endl;
        cout << "Remaining Data Size: " << remainingDataSize << endl;
        cout << "Throughput: " << throughput << endl;
    }
};

class Workload {
public:
    vector<Flow*> flows;
    Topology *topology;

    Workload(Topology* topology) : topology(topology) {}
    ~Workload() {
        for (Flow* flow : flows) {
            delete flow;
        }
    }

    void print();

    void generateRandomFlows(int count, int dataSizeRange, double timeRange);
    void generateSimpleFlows();
    void routing();

    vector<Flow*> activeFlows;
    double stableTime(double globalTime);
    void progress(double globalTime, double deltaTime);
    void updateState();

    void printState() {
        cout << "Workload State:" << endl;
        for (Flow* flow : flows) {
            flow->printState();
        }
    }

};


#endif // WORKLOAD_H