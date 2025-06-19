#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <memory>
using namespace std;

struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int start_time = -1;
    int completion_time;

    Process(int pid, int arrival, int burst)
        : pid(pid), arrival_time(arrival), burst_time(burst), remaining_time(burst) {}
};

class Scheduler {
public:
    virtual void schedule(vector<Process>& processes) = 0;
    virtual string name() = 0;
};

class FCFS : public Scheduler {
public:
    void schedule(vector<Process>& processes) override {
        sort(processes.begin(), processes.end(), [](Process& a, Process& b) {
            return a.arrival_time < b.arrival_time;
        });

        int current_time = 0;
        for (auto& p : processes) {
            if (current_time < p.arrival_time)
                current_time = p.arrival_time;
            p.start_time = current_time;
            current_time += p.burst_time;
            p.completion_time = current_time;
        }

        cout << "\n--- FCFS Schedule ---\n";
        print(processes);
    }

    string name() override { return "FCFS"; }

private:
    void print(const vector<Process>& processes) {
        for (const auto& p : processes) {
            cout << "P" << p.pid << " | Start: " << p.start_time
                 << " | Completion: " << p.completion_time
                 << " | Turnaround: " << p.completion_time - p.arrival_time
                 << " | Waiting: " << p.start_time - p.arrival_time << "\n";
        }
    }
};

class SJF : public Scheduler {
public:
    void schedule(vector<Process>& processes) override {
        vector<Process*> ready_queue;
        int current_time = 0, completed = 0;
        int n = processes.size();

        while (completed < n) {
            for (auto& p : processes) {
                if (p.arrival_time <= current_time && p.remaining_time > 0) {
                    bool already_added = find(ready_queue.begin(), ready_queue.end(), &p) != ready_queue.end();
                    if (!already_added) ready_queue.push_back(&p);
                }
            }

            if (!ready_queue.empty()) {
                auto shortest = min_element(ready_queue.begin(), ready_queue.end(), [](Process* a, Process* b) {
                    return a->burst_time < b->burst_time;
                });

                Process* p = *shortest;
                ready_queue.erase(shortest);
                p->start_time = current_time;
                current_time += p->burst_time;
                p->completion_time = current_time;
                p->remaining_time = 0;
                completed++;
            } else {
                current_time++;
            }
        }

        cout << "\n--- SJF Schedule ---\n";
        print(processes);
    }

    string name() override { return "SJF"; }

private:
    void print(const vector<Process>& processes) {
        for (const auto& p : processes) {
            cout << "P" << p.pid << " | Start: " << p.start_time
                 << " | Completion: " << p.completion_time
                 << " | Turnaround: " << p.completion_time - p.arrival_time
                 << " | Waiting: " << p.start_time - p.arrival_time << "\n";
        }
    }
};

class RoundRobin : public Scheduler {
public:
    RoundRobin(int qt) : quantum(qt) {}

    void schedule(vector<Process>& processes) override {
        queue<Process*> q;
        int current_time = 0;
        int completed = 0;
        int n = processes.size();
        vector<bool> visited(n, false);

        while (completed < n) {
            for (int i = 0; i < n; ++i) {
                if (processes[i].arrival_time <= current_time && !visited[i]) {
                    q.push(&processes[i]);
                    visited[i] = true;
                }
            }

            if (!q.empty()) {
                Process* p = q.front();
                q.pop();

                if (p->start_time == -1)
                    p->start_time = current_time;

                int exec_time = min(quantum, p->remaining_time);
                current_time += exec_time;
                p->remaining_time -= exec_time;

                for (int i = 0; i < n; ++i) {
                    if (processes[i].arrival_time <= current_time && !visited[i]) {
                        q.push(&processes[i]);
                        visited[i] = true;
                    }
                }

                if (p->remaining_time > 0)
                    q.push(p);
                else {
                    p->completion_time = current_time;
                    completed++;
                }
            } else {
                current_time++;
            }
        }

        cout << "\n--- Round Robin Schedule (Quantum = " << quantum << ") ---\n";
        print(processes);
    }

    string name() override { return "Round Robin"; }

private:
    int quantum;

    void print(const vector<Process>& processes) {
        for (const auto& p : processes) {
            cout << "P" << p.pid << " | Start: " << p.start_time
                 << " | Completion: " << p.completion_time
                 << " | Turnaround: " << p.completion_time - p.arrival_time
                 << " | Waiting: " << p.completion_time - p.arrival_time - p.burst_time << "\n";
        }
    }
};

void resetProcesses(vector<Process>& processes, const vector<Process>& original) {
    processes = original;
    for (auto& p : processes) {
        p.remaining_time = p.burst_time;
        p.start_time = -1;
    }
}

int main() {
    vector<Process> original = {
        {1, 0, 5},
        {2, 2, 3},
        {3, 4, 1},
        {4, 6, 7}
    };

    vector<Process> processes;
    vector<unique_ptr<Scheduler>> schedulers;
    schedulers.emplace_back(make_unique<FCFS>());
    schedulers.emplace_back(make_unique<SJF>());
    schedulers.emplace_back(make_unique<RoundRobin>(2));

    for (auto& scheduler : schedulers) {
        resetProcesses(processes, original);
        scheduler->schedule(processes);
    }

    return 0;
}
