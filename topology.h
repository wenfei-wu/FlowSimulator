#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "workload.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Node;
class Link;
class Topology;
class Flow;



// 节点类型枚举
enum NodeType {
    CORE,
    AGG,
    TOR,
    HOST
};

// 节点类
class Node {
public:
    int id;
    NodeType type;
    vector<Link*> links; // 从该节点出发的链路
    
    Node(int id, NodeType type);
    void print();
};

// 链路类
class Link {
public:
    int id;
    Node* src;
    Node* dst;
    double bandwidth;

    
    Link(int id, Node* src, Node* dst, double bandwidth);
    void print();

    
    double throughput;
    vector<Flow*> flows;
    void printState();
};

// 拓扑类
class Topology {
public:
    vector<Node*> nodes;
    vector<Link*> links;
    string desc="";
    
    void print();
    void generateFatTree(int k, int pods, double linkBw);
    void generateOneBigSwitch(int k, double linkBw);

    void printState(){
        cout << "Topology State: " << desc << endl;
        for(auto link : links){
            link->printState();
        }
    }
};

#endif