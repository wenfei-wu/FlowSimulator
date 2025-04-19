#include "workload.h"
#include <algorithm>
#include <unordered_set>
#include <queue>
#include <unordered_map>
#include <random>

using namespace std;

void Flow::print(){
    cout<<"Flow "<< id <<" from Node "<<src->id<<" to Node "<<dst->id<<endl;
    cout<<"Data Size: "<<dataSize<<endl;
    cout<<"Start Time: "<<startTime<<endl;
    cout<<"Path: ";
    for(auto node : path){
        cout<<node->id<<" ";
    }
    cout<<endl;
    cout<<"Path Links: ";
    for(auto link : path_links){
        cout<<link->id<<" ";
    }
    cout<<endl;
    cout<<"Flow State: ";
    switch(state){
        case NOTSTART: cout<<"NOTSTART"; break;
        case RUN: cout<<"RUN"; break;
        case COMPLETE: cout<<"COMPLETE"; break;
    }
    cout<<endl;

}

double Flow::stableTime(double globalTime) {
    // 计算流的稳定时间    

    if (state == NOTSTART) {
        return startTime-globalTime < 0 ? 0 : startTime - globalTime;
    }
    else if (state == RUN) {
        // 计算当前流的稳定时间
        return remainingDataSize / throughput ;
    }
    else {
        return numeric_limits<double>::infinity();
    }
}
void Flow::progress(double globalTime, double deltaTime) {
    // 更新流的状态
    if (state == NOTSTART) {
        if(globalTime + deltaTime >= startTime - 1e-5) {
            state = RUN;
            remainingDataSize = dataSize;
            // cout << "Flow " << id << " from Node " << src->id << " to Node " << dst->id << " started at time " << globalTime + deltaTime << endl;
        }
    }
    else if(state == RUN) {
        // 计算剩余数据大小
        remainingDataSize -= throughput * deltaTime;
        // 检查是否完成
        if (remainingDataSize < 1e-5) {
            state = COMPLETE;
            remainingDataSize = 0;
            // cout << "Flow " << id << " from Node " << src->id << " to Node " << dst->id << " completed at time " << globalTime + deltaTime << endl;
        }
    }  
}

void Flow::routingECMP(Topology* topology) {
    // 清空现有路径
    path.clear();
    
    // 如果源和目的相同，直接返回
    if (src == dst) {
        path.push_back(src);
        return;
    }

    // BFS队列和访问记录
    queue<Node*> q;
    unordered_map<Node*, int> dist;
    unordered_map<Node*, vector<Node*>> prev;
    
    q.push(src);
    dist[src] = 0;
    
    // BFS遍历
    while (!q.empty()) {
        Node* current = q.front();
        q.pop();
        
        // 如果到达目标节点，停止继续搜索更长的路径
        if (current == dst) continue;
        
        // 遍历所有邻居
        for (Link* link : current->links) {
            Node* neighbor = link->dst;
            
            // 如果邻居未被访问过
            if (dist.find(neighbor) == dist.end()) {
                dist[neighbor] = dist[current] + 1;
                prev[neighbor].push_back(current);
                q.push(neighbor);
            }
            // 如果找到相同距离的路径
            else if (dist[neighbor] == dist[current] + 1) {
                prev[neighbor].push_back(current);
            }
        }
    }
    
    // 如果没有路径到达目标节点
    if (prev.find(dst) == prev.end()) {
        return;
    }
    
    // 收集所有最短路径
    vector<vector<Node*>> allPaths;
    vector<Node*> currentPath;
    
    function<void(Node*)> buildPaths = [&](Node* node) {
        currentPath.push_back(node);
        
        if (node == src) {
            vector<Node*> path = currentPath;
            reverse(path.begin(), path.end());
            allPaths.push_back(path);
        } else {
            for (Node* p : prev[node]) {
                buildPaths(p);
            }
        }
        
        currentPath.pop_back();
    };
    
    buildPaths(dst);
    
    // 随机选择一条路径
    int randomIndex = rand() % allPaths.size();
    path = allPaths[randomIndex];

    // generate path_links
    for (size_t i = 0; i < path.size() - 1; ++i) {
        Node* srcNode = path[i];
        Node* dstNode = path[i + 1];
        
        // 找到对应的链路
        for (Link* link : srcNode->links) {
            if (link->dst == dstNode) {
                path_links.push_back(link);
                break;
            }
        }
    }
}

void Workload::generateSimpleFlows(){
    Node *src = topology->nodes[0];
    Node *dst = topology->nodes[1];
    Flow *flow = new Flow(0, src, dst, 100.0, 0.0);
    flows.push_back(flow);

    Node *src2 = topology->nodes[topology->nodes.size()-1];
    Node *dst2 = topology->nodes[1];
    Flow *flow2 = new Flow(1, src2, dst2, 30.0, 0.5);
    flows.push_back(flow2);
}

void Workload::generateRandomFlows(int count, int dataSizeRange, double timeRange) {
    auto nodeDist = [&](int range) { return rand() % range; };
    auto sizeDist = [&](int range) { return 1 + rand() % range; };
    auto timeDist = [&]() { return static_cast<double>(rand()) / RAND_MAX * timeRange; };
    
    for (int i = 0; i < count; ++i) {
        Node* src = topology->nodes[nodeDist(topology->nodes.size())];
        Node* dst = topology->nodes[nodeDist(topology->nodes.size())];
        
        // 确保源和目的节点不同
        while (src == dst) {
            dst = topology->nodes[nodeDist(topology->nodes.size())];
        }
        
        int dataSize = sizeDist(dataSizeRange);
        double startTime = timeDist(); // 随机生成开始时间
        // 创建流对象并添加到工作负载中
        Flow* flow = new Flow(i, src, dst, dataSize, startTime);
        flows.push_back(flow);
    }
}

void Workload::routing() {
    for (Flow* flow : flows) {
        flow->routingECMP(topology);
    }
}

void Workload::print() {
    cout << "Workload Information:" << endl;
    for (Flow* flow : flows) {
        flow->print();
    }
}


void Workload::updateState() {
    // water filling here

    // empty link throughput and flows
    for(Link* link : topology->links) {
        link->throughput = 0;
        link->flows.clear();
    }

    // get all active flows, empty flow throughput, add flow to links
    activeFlows.clear();   
    for(Flow* flow : flows) {
        if (flow->state == RUN) {
            flow->throughput = 0;
            activeFlows.push_back(flow);
            for(Link* link : flow->path_links) {
                link->flows.push_back(flow);
            }
        }
    }

    // loop until no active flows
    while(activeFlows.size() > 0) {
        double minThroughput = std::numeric_limits<double>::infinity();
        for(Link* link : topology->links) {
            if(link->flows.size() > 0) {
                double linkThroughput = link->bandwidth - link->throughput;
                if(linkThroughput / link->flows.size() < minThroughput) {
                    minThroughput = linkThroughput / link->flows.size() ;
                }
            }
        }
        // if no link can be filled, break
        if(minThroughput == std::numeric_limits<double>::infinity()) {
            cout << "No link can be filled, ERROR !!!!!!!!!!!!!!" << endl;
            break;
        }
        for(Flow* flow : activeFlows) {
            flow->throughput += minThroughput;
        }
        for(Link* link : topology->links) {
            if(link->flows.size() > 0) {
                link->throughput += minThroughput * link->flows.size();
                // check if link is full
                if(link->throughput >= link->bandwidth - 1e-6) {
                    // remove all flows from this link
                    for(Flow* flow : link->flows) {
                        activeFlows.erase(std::remove(activeFlows.begin(), activeFlows.end(), flow), activeFlows.end());
                    }
                    link->flows.clear();
                }
            }
        }
    }
} 

double Workload::stableTime(double globalTime) {
    double minTime = std::numeric_limits<double>::infinity();    
    for (Flow* flow : flows) {
        double time = flow->stableTime(globalTime);
        if (time < minTime) {
            minTime = time;
        }
    }
    
    return minTime;
}
void Workload::progress(double globalTime, double deltaTime) {
    for (Flow* flow : flows) {
        flow->progress(globalTime, deltaTime);
    }
}