#include <bits/stdc++.h>
using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

#define TIME_STEP 10

// Class containing the thread and associated parameters
class Process{
public:
    int id;
    int arrival_time;
    int burst_time;
    int executed_time = 0;
    int completion_time;
    int waiting_time;
    int turn_around_time;
    int priority = -1;

    atomic<bool> paused{true};     // true if thread is paused
    atomic<bool> finished{false};  // true if thread is completed
    thread t;                      // the thread
    mutex mtx;
    condition_variable cv;         // to pause and run the thread

    // Constructor to initialize the thread parameters
    Process(){}
    Process(int id, int arrival_time, int burst_time, int priority){
        this->id = id;
        this->arrival_time = arrival_time;
        this->burst_time = burst_time;
        this->priority = priority;
        start_thread();  // initialize the thread
    }
    // Destructor
    ~Process(){
        finished = true;  // make sure thread is finished
        cv.notify_all();
        join();           // join the thread to prevent segfault
    }
    
    void start_thread(){  // initialize the thread
        t = thread(&Process::simulate, this);
    }

    void run_thread(){
        lock_guard<mutex> lock(mtx);
        paused = false;    // change status
        cout << "resuming process " << id << endl;
        cv.notify_all();   // notify the change to the cv.wait
    }

    void pause_thread(){
        lock_guard<mutex> lock(mtx);
        paused = true;    // pause the thread
        cout << "pausing process " << id << endl;
    }

    void simulate(){   // simulates the thread with sleep statement
        unique_lock<mutex> ul(mtx);  // mutex to control the thread

        while(executed_time<burst_time and !finished){    // run the function for burst time period
            cv.wait(ul, [this]{return !paused||finished;});  // run only if 'paused' is set to false
            if(finished) break;     // exit if process is declared finished

            ul.unlock();            // unlocks the mutex
            cout << "Thread " << id  << endl;
            sleep_for(milliseconds(TIME_STEP));
            ul.lock();              // locks the mutex to update variable value

            executed_time +=TIME_STEP;  // thread should be locked while updating variables
        }
        finished = true;
        cout << "Thread " << id << " finished execution" << endl;
    }

    void store_data(int finishing_time){   // updates process varaibles once the thread is completed
        completion_time = finishing_time;
        turn_around_time = completion_time - arrival_time;
        waiting_time = turn_around_time - burst_time;
    }

    void join() {   // join the thread
        if (t.joinable()) {
            t.join();
        }
    }
};
// Scheduler Base Class
class Scheduler{
public:
    deque<Process> p;     // contains 'Process' objects
    int num_process  = 0; // Number of processes

    // add single process
    void add_process(int id, int arrival_time, int burst_time, int priority){
        p.emplace_back(id, arrival_time, burst_time, priority);
        num_process++;
    }

    // add a vector containing processes
    void add_process(const vector<tuple<int, int, int,int>>& v) {
        for (const auto& process : v) {
            int id, arrival_time, burst_time,priority;
            tie(id, arrival_time, burst_time,priority) = process;
            p.emplace_back(id, arrival_time, burst_time, priority);
            num_process++;
        }
    }

    // save process information in a file (CSV format)
    void export_meta_data(string file_name){
        ofstream file(file_name);
        file << "id,arrival_time,burst_time,priority,completion_time,turn_around_time,waiting_time" << endl;

        for(int i=0; i<num_process; i++)
        file << p[i].id <<"," << p[i].arrival_time << "," << p[i].burst_time <<"," << p[i].priority << "," << p[i].completion_time << "," << p[i].turn_around_time << ", " << p[i].waiting_time << endl;
    }
};

// First Come First Serve Scheduler
class FCFS : public Scheduler{
public:
    struct CompareOrder{  
        FCFS* scheduler;
        CompareOrder(FCFS* s) : scheduler(s){}
        bool operator()(int a, int b){
            return scheduler->p[a].arrival_time > scheduler->p[b].arrival_time;
        }
    };
    // stores threads in the order which they arrive
    priority_queue<int, vector<int>, CompareOrder> order;
    FCFS() : order(CompareOrder(this)) {}

    // push all the process(indices) into the queue
    void push_to_queue(){for(int i=0; i<num_process; i++) order.push(i);}

    void run(bool file_output = true, string file_name="fcfs_data.csv") {
        /*
        runs FCFS simulation
        INPUT: whether to save output in a file(bool), file output file name (string)
        */
        push_to_queue(); // load the processes

        auto start_time = high_resolution_clock::now();
        int time_step = TIME_STEP/2;

        ofstream file(file_name); // initialize output file

        while(!order.empty()){    // run until all threads are completed

            auto t1 = high_resolution_clock::now();

            int current_time = duration_cast<milliseconds>(t1 - start_time).count();
            int cur_ind = order.top();
            
            if(p[cur_ind].arrival_time>current_time){   // no active threads
                file << current_time << "," << -1 << endl;
                sleep_for(milliseconds(time_step));
                continue;
            } 
            p[cur_ind].run_thread();        // run the earliest thread
            while(!p[cur_ind].finished) {;  // wait till the thread is completed (non-preemptive)
                if(file_output){
                    t1 = high_resolution_clock::now();
                    current_time = duration_cast<milliseconds>(t1 - start_time).count();
                    file << current_time << "," << p[cur_ind].id << endl;
                } 
                    
                sleep_for(milliseconds(time_step));
            }
            // store data of finished thread
            if(p[cur_ind].finished) p[cur_ind].store_data(current_time);
            
            // remove thread from queue
            order.pop();
        }

        cout << "scheduling complete" << endl;
    }
};


// SHORTEST JOB FIRST SCHEDULER
class SJT : public Scheduler{
public:
    struct CompareOrder{
        SJT* scheduler;
        CompareOrder(SJT* s) : scheduler(s){}
        bool operator()(int a, int b){
            return scheduler->p[a].arrival_time > scheduler->p[b].arrival_time;
        }
    };
    
    // assign higher priority to task with lower burst time
    struct ComparePriority{
        SJT* scheduler;
        ComparePriority(SJT* s) : scheduler(s){}
        bool operator()(int a, int b){
            return scheduler->p[a].burst_time > scheduler->p[b].burst_time;
        }
    };
    
    priority_queue<int, vector<int>, CompareOrder> order;   // order in which process arrive
    priority_queue<int, vector<int>, ComparePriority> pq;   // priority order for sjt

    SJT() : order(CompareOrder(this)), pq(ComparePriority(this)) {} // constructor

    void push_to_queue() {for(int i=0; i<num_process; i++) order.push(i);}

    void run(bool file_output = true, string file_name="sjt_gantt_data.csv") {
        push_to_queue();

        auto start_time = high_resolution_clock::now();
        int time_step = TIME_STEP/2;

        ofstream file(file_name);

        while(!(order.empty() and pq.empty())){
            auto t1 = high_resolution_clock::now();
            int current_time = duration_cast<milliseconds>(t1 - start_time).count();
            
            // push active tasks into the queue
            while(!order.empty()){
                int ind = order.top();
                if(p[ind].arrival_time < current_time){
                    pq.push(ind);
                    order.pop();
                }
                else break;
            }
            if(pq.empty()){
                sleep_for(milliseconds(time_step));  // no active task
                file << current_time << "," << -1 << endl;
                continue;
            }
            int cur_ind = pq.top();   // run the highest priority task
            p[cur_ind].run_thread();

            while(!p[cur_ind].finished){  // wait till task is completed (non-preemptive)
                if(file_output){
                    t1 = high_resolution_clock::now();
                    current_time = duration_cast<milliseconds>(t1 - start_time).count();
                    file << current_time << "," << p[cur_ind].id << endl;
                }

                sleep_for(milliseconds(time_step));
            }

            if(p[cur_ind].finished) p[cur_ind].store_data(current_time);
            pq.pop();
        }

        cout << "scheduling complete" << endl;
    }
};

// SHORTEST TIME REMAINING FIRST SCHEDULING
class STRF :public Scheduler {
public :
    struct CompareOrder{
        STRF* scheduler;
        CompareOrder(STRF* s) : scheduler(s){}
        bool operator()(int a, int b){
            return scheduler->p[a].arrival_time > scheduler->p[b].arrival_time;
        }
    };
    // assign high priority to task with less runtime left
    struct ComparePriority{
        STRF* scheduler;
        ComparePriority(STRF* s) : scheduler(s){}
        bool operator()(int a, int b){
            return scheduler->p[a].burst_time-scheduler->p[a].executed_time > scheduler->p[b].burst_time-scheduler->p[b].executed_time;
        }
    };
    
    priority_queue<int, vector<int>, CompareOrder> order;
    priority_queue<int, vector<int>, ComparePriority> pq;

    STRF() : order(CompareOrder(this)), pq(ComparePriority(this)) {}

    void push_to_queue(){for(int i=0; i<num_process; i++) order.push(i);}

    void run(bool file_output = true, string file_name="strf_gantt_data.csv"){
        push_to_queue();

        auto start_time = high_resolution_clock::now();
        int time_step = TIME_STEP/2;
        ofstream file(file_name);

        int cur_ind, prev_ind=-1;  // keep track of previous process
        while(!(order.empty() and pq.empty())){
            auto t1 = high_resolution_clock::now();
            int current_time = duration_cast<milliseconds>(t1 - start_time).count();
            
            while(!order.empty()){  // push active threads into queue
                int ind = order.top();
                if(p[ind].arrival_time < current_time){
                    pq.push(ind);
                    order.pop();
                }
                else break;
            }
            if(pq.empty()){         // no active task
                sleep_for(milliseconds(time_step));  
                file << current_time << "," << -1 << endl;
                continue;
            }
            cur_ind = pq.top();
            
            // pause previous task if there is change in priority (Preemptive scheduling)
            if(prev_ind != cur_ind) if(prev_ind!=-1 and !p[prev_ind].finished) p[prev_ind].pause_thread();  
            // unpause current task if it not already running
            if(p[cur_ind].paused) p[cur_ind].run_thread();   

            if(file_output) file << current_time << "," << p[cur_ind].id << endl;

            sleep_for(milliseconds(time_step));

            if(p[cur_ind].finished){
                p[cur_ind].store_data(current_time);
                pq.pop();
            }

            prev_ind = cur_ind;
        }

        cout << "scheduling complete" << endl;
    }
};

// ROUND ROBIN SCHEDULER
class RR : public Scheduler{
public:
    struct CompareOrder{
        RR* scheduler;
        CompareOrder(RR* s) : scheduler(s){}
        bool operator()(int a, int b){
            return scheduler->p[a].arrival_time > scheduler->p[b].arrival_time;
        }
    };
    
    priority_queue<int, vector<int>, CompareOrder> order;
    deque<int> queue;  // use deque to schedule threads

    RR() : order(CompareOrder(this)){}

    void push_to_queue(){ for(int i=0; i<num_process; i++) order.push(i);}

    void run(bool file_output = true, string file_name="rr_gantt_data.csv") {
        push_to_queue();

        auto start_time = high_resolution_clock::now();
        int time_step = TIME_STEP/2;

        ofstream file(file_name);

        int quant = 40;   // time alloted for each cycle

        int cur_ind, prev_ind=-1;
        while(!(order.empty() and queue.empty())){
            auto t1 = high_resolution_clock::now();
            int current_time = duration_cast<milliseconds>(t1 - start_time).count();
            
            while(!order.empty()){ // add active tasks to the queue
                int ind = order.top();
                if(p[ind].arrival_time < current_time){
                    queue.push_back(ind);
                    order.pop();
                }
                else break;
            }
            
            if(queue.empty()){  // no active task
                sleep_for(milliseconds(time_step));
                if(file_output) file << current_time << "," << -1 << endl;
                continue;
            }
            cur_ind = queue.front();  

            if(prev_ind!=-1 and !p[prev_ind].finished) p[prev_ind].pause_thread();  // pause previous thread
            p[cur_ind].run_thread();  // run current thread

            int rr_time = 0;
            while(rr_time<quant){   // wait for specified time
                t1 = high_resolution_clock::now();
                current_time = duration_cast<milliseconds>(t1 - start_time).count();
                if(file_output) file << current_time << "," << p[cur_ind].id << endl;

                sleep_for(milliseconds(TIME_STEP));
                rr_time += TIME_STEP;
            }

            if(p[cur_ind].finished){    // remove process if finished
                p[cur_ind].store_data(current_time);
                queue.pop_front();
            }
            else{                       // push process to the back of the queue
                queue.push_back(queue.front());
                queue.pop_front();
            }
            prev_ind = cur_ind;
        }

        cout << "scheduling complete" << endl;
    }
};

//PRIORITY SCHEDULING
class PS :public Scheduler {
public :
    struct CompareOrder{
        PS* scheduler;
        CompareOrder(PS* s) : scheduler(s){}
        bool operator()(int a, int b){
            return scheduler->p[a].arrival_time > scheduler->p[b].arrival_time;
        }
    };
    
    // task with higher assigned priority gets higher priority
    struct ComparePriority{
        PS* scheduler;
        ComparePriority(PS* s) : scheduler(s){}
        bool operator()(int a, int b){
            return scheduler->p[a].priority < scheduler->p[b].priority;
        }
    };
    
    priority_queue<int, vector<int>, CompareOrder> order;
    priority_queue<int, vector<int>, ComparePriority> pq;

    PS() : order(CompareOrder(this)), pq(ComparePriority(this)) {}

    void push_to_queue(){
        for(int i=0; i<num_process; i++) order.push(i);
    }

    void run(bool file_output = true, string file_name="ps_gantt_data.csv"){
        push_to_queue();
        auto start_time = high_resolution_clock::now();
        int time_step = TIME_STEP/2;
        ofstream file(file_name);
        int cur_ind, prev_ind=-1;
        while(!(order.empty() and pq.empty())){
            auto t1 = high_resolution_clock::now();
            int current_time = duration_cast<milliseconds>(t1 - start_time).count();
            
            while(!order.empty()){
                int ind = order.top();
                if(p[ind].arrival_time < current_time){
                    pq.push(ind);
                    order.pop();
                }
                else break;
            }
            if(pq.empty()){
                sleep_for(milliseconds(time_step));
                file << current_time << "," << -1 << endl;
                continue;
            }
            cur_ind = pq.top();
            
            if(prev_ind != cur_ind) if(prev_ind!=-1 and !p[prev_ind].finished) p[prev_ind].pause_thread();
            if(p[cur_ind].paused) p[cur_ind].run_thread();

            if(file_output) file << current_time << "," << p[cur_ind].id << endl;

            sleep_for(milliseconds(time_step));

            if(p[cur_ind].finished){
                p[cur_ind].store_data(current_time);
                pq.pop();
            }
            prev_ind = cur_ind;
        }

        cout << "scheduling complete" << endl;
    }
};

// FUNCTION TO READ PROCESSES INFORMATION FROM CSV FILE
void read_data_from_file(vector<tuple<int, int, int,int>> &processes, string file_name){
    /*
    csv format - process id, arrival time, burst time, priority
    */
    ifstream file(file_name);
    string line;

    while (getline(file, line)) { // Read each line
        stringstream ss(line);
        string value;
        vector<int> row;

        while (getline(ss, value, ',')) { // Split by comma
            row.push_back(stoi(value)); // Convert string to int
        }

        if (row.size() == 4) { // Only if the row has 4 elements
            processes.emplace_back(row[0], row[1], row[2], row[3]);
        }
    }

    file.close();
}