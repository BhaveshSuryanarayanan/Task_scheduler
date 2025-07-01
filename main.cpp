#include <bits/stdc++.h>
using namespace std;
#include "task_scheduler.h"

int main(){
    // streambuf* orig_buf = cout.rdbuf(); ofstream null_stream("/dev/null");cout.rdbuf(null_stream.rdbuf());

    // load processes info from file ad save in a vector of tuples
    vector<tuple<int, int, int,int>> processes;
    read_data_from_file(processes,"input_3.csv");

    cout << "FCFS" << endl;
    FCFS fcfs_scheduler;
    fcfs_scheduler.add_process(processes);                  // add the process to the object
    fcfs_scheduler.run(true,"fcfs_gantt_data.csv");         // run simulation
    fcfs_scheduler.export_meta_data("fcfs_meta_data.csv");  // export runtime data
    
    cout << "SJT" << endl;
    SJT SJT_scheduler;
    SJT_scheduler.add_process(processes);
    SJT_scheduler.run(true,"sjt_gantt_data.csv");
    SJT_scheduler.export_meta_data("sjt_meta_data.csv");

    cout << "STRF" << endl;
    STRF STRF_scheduler;
    STRF_scheduler.add_process(processes);
    STRF_scheduler.run(true,"strf_gantt_data.csv");
    STRF_scheduler.export_meta_data("strf_meta_data.csv");

    cout << "RR" << endl;
    RR RR_scheduler;
    RR_scheduler.add_process(processes);
    RR_scheduler.run(true,"rr_gantt_data.csv");
    RR_scheduler.export_meta_data("rr_meta_data.csv");

    cout << "PS" << endl;
    PS PS_scheduler;
    PS_scheduler.add_process(processes);
    PS_scheduler.run(true,"ps_gantt_data.csv");
    PS_scheduler.export_meta_data("ps_meta_data.csv");

    return 0;
}