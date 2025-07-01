# Task Scheduler in C++

A lightweight thread management simulation in C++ that mimics an operating system's process/thread scheduling. Designed to help visualize and analyze scheduling strategies such as FCFS, Round-Robin, and Priority-based scheduling.

### Features

- 🧵 Simulates process threads with:
  - Arrival Time
  - Burst Time
  - Priority (optional)
- 📊 Tracks:
  - Waiting Time
  - Turnaround Time
  - Completion Time
- Visualization:
  - Gantt Charts
  - Comparision Table

### Usage

```bash
g++ -pthread -o thread_manager main.cpp
./thread_manager
```
### File Structure
```
.  
├── main.cpp               # Entry point  
├── task_scheduler.h       # Scheduler classes and functions  
├── plotter.py             # Visualizing classes and functions  
├── analysis.ipynb         # Output Visualizer  
└── README.md              # This file  
```
  

## Detailed Description

### Classes
The programme has the following classes defined in `task_scheduler.h`
- `Process`: Stores the process thread and data related to the process
- `Scheduler`: Base class for scheduling algorithms
- `FCFS`,`SJT`,`STRF`,`RR`,`PS`: Logic for scheduling
  
### Input
Input to the program can be provided in two ways:
  - Directly in the program
  - From CSV file
Both are taken care by `add_process` function in `Schedule` class. Polymorphism is leveraged to support different input formats
- Input format: process_id, arrival_time, burst_time, priority
- Example input file
    ``` csv
    1,20,200,0
    2,100,100,2
    3,140,40,1
    4,180,60,0
    5,220,100,1
    ```
### Running the algorithm
- Choose from any of the mentioned algorithm and define and object. `run` function runs the algorithm. Two optional parameters are required.
  - output type: *True* to enable file output
  - output file: file name of the output file  
- after the algorithms is completed `export_meta_data` function saves all the data related to the simulation in the specified file.

### Output
- The example file outputs of the simulation is shown
- Data file - *time_stamp(ms)*, *current thread&
    ```
    147,2
    157,2
    168,2
    178,2
    189,1
    200,1
    231,3
    305,2
    ```
- Simulation meta data
    ```
    id,arrival_time,burst_time,priority,completion_time,turn_around_time,waiting_time
    1,20,200,0,388,368, 168
    2,100,100,2,472,372, 272
    3,140,40,1,263,123, 83
    4,180,60,0,514,334, 274
    5,220,100,1,598,378, 278
    ```
### Visualization

- *plotter.py* Contains Classes and Functions for visualizationa and *analysis.ipynb* demonstrates the following
- `plot_gantt_chart`: plots the Gantt chart of the whole process displaying which thread was executed at each instant
- `plot_individual_gantt_chart`: plots the Gantt chart of every individual thread displaying the waiting time and runtime
- `evaluate_performance`: calculate parameters related to the simulation
  - Average waiting time
  - Peak waiting time
  - Average burst time
  - Average Response Time
  - Context Switch Overhead
- `compare_schedulers`: Prints comparision table to compare the performance of different scheduling algorithms and also plots a bar graph for each parameter.

## Scheduling Algorithms

#### 1. FCFS 
- threads are run in the order they arrive (arrival time)
- Non preemptive
- disadvantage: it can result in long waiting times if a long process arrrives before a shorter one (convoy effect) 
  
#### 2. Short Job FIrst (SJF) scheduling
- process with smallest waiting time is executed first
- it gives the minimum average waiting time overall
- but it could lead to **starvation** if shorter processes keep coming 
  
#### 3. Shortest remaining time first(STRF) scheduling
- same as sjf but preemptive
- advantages and disadvantages are the same


#### 4. Round Robin Scheduling
- The system rotates through all the processes allocating each of them a fixed time slice or quantum, regardless of their priority.
- after every quantum, if the process is completed then it is removed from the queue else it goes to the back of the queue.
- disadvantage : high overhead due to switching


#### 5. Priority Scheduling
- Each process is assigned a priority based on criteria such as memory requirment, time requirement, burst time, I/O, etc.
- highest priority task is executed first
- if tasks have same priority, then fcfs is followed
- can be preemptive of non-preemptive