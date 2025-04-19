#include "simulator.h"

using namespace std;

void Simulator::run(){
    // 1. 初始化
    globalTime = 0;
    
    // 2. 运行模拟器
    while (true) {
        // 3. 更新流状态
        workload->updateState();
        cout << "--------------------------" << endl;
        cout << "Global Time: " << globalTime << endl;
        double time = workload->stableTime(globalTime);
        if( time == numeric_limits<double>::infinity() ){
            break;
        }
        workload->progress(globalTime, time);
        globalTime += time;
    }
    cout << "Simulation finished." << endl;
    cout << "Final Global Time: " << globalTime << endl;
}