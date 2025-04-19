#include "topology.h"


using namespace std;

// 节点类实现
Node::Node(int id, NodeType type) : id(id), type(type) {}

void Node::print() {
    string typeStr;
    switch(type) {
        case CORE: typeStr = "CORE"; break;
        case AGG: typeStr = "AGG"; break;
        case TOR: typeStr = "TOR"; break;
        case HOST: typeStr = "HOST"; break;
    }
    cout << "Node ID: " << id << ", Type: " << typeStr << endl;
}

// 链路类实现
Link::Link(int id, Node* src, Node* dst, double bandwidth) : id(id), src(src), dst(dst), bandwidth(bandwidth) {}

void Link::print() {
    cout << "Link ID: " << id << ", Source Node ID: " << src->id 
         << ", Destination Node ID: " << dst->id 
         << ", Bandwidth: " << bandwidth << endl;
}


void Link::printState() {
    cout << "------------Link State--------------------" << endl;
    if(flows.size() == 0) {
        cout << "Link ID: " << id << " is empty." << endl;
        return;
    }
    print();
    cout << "Link Throughput: " << throughput << endl;
    cout << "Flows: ";
    for(auto flow : flows){
        cout << flow->src->id << "->" << flow->dst->id << " ";
    }
    cout << endl;
}

// 拓扑类实现
void Topology::print() {
    cout << "Topology Description: " << desc << endl;
    cout << "Nodes:" << endl;
    for (auto node : nodes) {
        node->print();
    }
    
    cout << "Links:" << endl;
    for (auto link : links) {
        link->print();
    }
}

void Topology::generateFatTree(int k, int pods, double linkBw) {
    // 清空现有拓扑
    for (auto node : nodes) delete node;
    for (auto link : links) delete link;
    nodes.clear();
    links.clear();

    desc = "Fat Tree Topology (k=" + to_string(k) + ", pods=" + to_string(pods) + ")";
    
    // 计算各层交换机数量
    int coreSwitches = (k/2) * (k/2);
    int aggSwitchesPerPod = k/2;
    int torSwitchesPerPod = k/2;
    int hostsPerTor = k/2;
    
    // 生成节点 (从主机开始编号)
    int nodeId = 0;
    
    // 1. 生成主机
    for (int p = 0; p < pods; p++) {
        for (int t = 0; t < torSwitchesPerPod; t++) {
            for (int h = 0; h < hostsPerTor; h++) {
                nodes.push_back(new Node(nodeId++, HOST));
            }
        }
    }
    
    // 2. 生成TOR交换机
    for (int p = 0; p < pods; p++) {
        for (int t = 0; t < torSwitchesPerPod; t++) {
            nodes.push_back(new Node(nodeId++, TOR));
        }
    }
    
    // 3. 生成AGG交换机
    for (int p = 0; p < pods; p++) {
        for (int a = 0; a < aggSwitchesPerPod; a++) {
            nodes.push_back(new Node(nodeId++, AGG));
        }
    }
    
    // 4. 生成CORE交换机
    for (int c = 0; c < coreSwitches; c++) {
        nodes.push_back(new Node(nodeId++, CORE));
    }
    
    // 生成链路
    nodeId = 0;
    int linkId = 0;
    
    // 1. 主机到TOR的链路
    for (int p = 0; p < pods; p++) {
        for (int t = 0; t < torSwitchesPerPod; t++) {
            Node* tor = nodes[nodes.size() - torSwitchesPerPod*pods - aggSwitchesPerPod*pods - coreSwitches + p*torSwitchesPerPod + t];
            for (int h = 0; h < hostsPerTor; h++) {
                Node* host = nodes[nodeId++];
                Link* link = new Link(linkId++, host, tor, linkBw); // 主机到TOR
                links.push_back(link);
                host->links.push_back(link); // 将链路添加到TOR的链路列表中
                // 反向链路
                Link* reverseLink = new Link(linkId++, tor, host, linkBw); // TOR到主机
                links.push_back(reverseLink);
                tor->links.push_back(reverseLink); // 将链路添加到主机的链路列表中
            }
        }
    }
    
    // 2. TOR到AGG的链路
    for (int p = 0; p < pods; p++) {
        int podOffset = nodes.size() - torSwitchesPerPod*pods - aggSwitchesPerPod*pods - coreSwitches + p*torSwitchesPerPod;
        for (int t = 0; t < torSwitchesPerPod; t++) {
            Node* tor = nodes[podOffset + t];
            for (int a = 0; a < aggSwitchesPerPod; a++) {
                Node* agg = nodes[nodes.size() - aggSwitchesPerPod*pods - coreSwitches + p*aggSwitchesPerPod + a];

                Link* link = new Link(linkId++, tor, agg, linkBw); // TOR到AGG
                links.push_back(link);
                tor->links.push_back(link); // 将链路添加到AGG的链路列表中
                // 反向链路
                Link* reverseLink = new Link(linkId++, agg, tor, linkBw); // AGG到TOR
                links.push_back(reverseLink);
                agg->links.push_back(reverseLink); // 将链路添加到TOR的链路列表中
            }
        }
    }
    
    // 3. AGG到CORE的链路
    for (int p = 0; p < pods; p++) {
        for (int a = 0; a < aggSwitchesPerPod; a++) {
            Node* agg = nodes[nodes.size() - aggSwitchesPerPod*pods - coreSwitches + p*aggSwitchesPerPod + a];
            for (int c = a; c < coreSwitches; c += aggSwitchesPerPod) {
                Node* core = nodes[nodes.size() - coreSwitches + c];
                Link* link = new Link(linkId++, agg, core, linkBw); // AGG到CORE
                links.push_back(link);
                agg->links.push_back(link); // 将链路添加到CORE的链路列表中
                // 反向链路
                Link* reverseLink = new Link(linkId++, core, agg, linkBw); // CORE到AGG
                links.push_back(reverseLink);
                core->links.push_back(reverseLink); // 将链路添加到AGG的链路列表中
            }
        }
    }
}

void Topology::generateOneBigSwitch(int k, double linkBw){
    // 清空现有拓扑
    for (auto node : nodes) delete node;
    for (auto link : links) delete link;
    nodes.clear();
    links.clear();

    desc = "One Big Switch Topology (k=" + to_string(k) + ")";
    
    // generate HOST
    int nodeId = 0;
    for (int i = 0; i < k; ++i) {
        nodes.push_back(new Node(nodeId++, HOST));
    }

    // generate a switch 
    Node* switchNode = new Node(nodeId++, AGG);
    nodes.push_back(switchNode);

    
    // generate links, connecting switch and host
    for (int i = 0; i < k; ++i) {
        Node* host = nodes[i];
        Link* link = new Link(i, host, switchNode, linkBw);
        links.push_back(link);
        host->links.push_back(link); // 将链路添加到主机的链路列表中
        // 反向链路
        Link* reverseLink = new Link(i + k, switchNode, host, linkBw);
        links.push_back(reverseLink);
        switchNode->links.push_back(reverseLink); // 将链路添加到交换机的链路列表中
    }
}