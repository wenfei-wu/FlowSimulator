#include "topology.h"
#include "workload.h"
#include "simulator.h"

#include <iostream>
#include <cstdlib>

int main() {
    
    srand(0);
    // srand(time(0));
    Topology topology;
    // topology.generateFatTree(4, 2, 100.0); // 生成4端口、2个pod的胖树拓扑，链路带宽100.0
    topology.generateOneBigSwitch(2, 100.0); // 生成2端口的单大交换机拓扑，链路带宽100.0
    // topology.print();

    // 创建工作负载
    Workload workload(&topology);
    // workload.generateRandomFlows(3, 100, 10); // 生成3个随机流，数据大小范围1-100
    workload.generateSimpleFlows(); // 生成简单流
    workload.routing(); // 路由
    // workload.print(); // 打印工作负载信息

    // 创建模拟器
    Simulator simulator(&topology, &workload);
    simulator.run(); // 运行模拟器

    return 0;
}