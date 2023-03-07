#ifndef SCHEDULER_REPORTER_H
#define SCHEDULER_REPORTER_H

typedef struct process {
    int id;
    int priority;
    int state;

    int start_time;
    int end_time;
    int wait_time;

    int burst_time;         // total time spend on CPU
    int arrival_time;       // time at which process is created
    int response_time;      // interval between process creation and first run on CPU
    int waiting_time;       // total time spent in ready state
    int turnaround_time;    // interval between process creation and termination

    int burst_countdown;
    int io_countdown;
    int quantum_countdown;  // depends on scheduling algorithm
    int cpu_burst;          // range from ? to ?, distribution based on ?
    int io_burst;           // range from ? to ?
    int reps;               // should be random
} proc_t;

void print_status_line(long time_elapsed,
                       int nr_processes,
                       proc_t **processes,
                       proc_t *live_proc);

void report(long cpu_in_use, long cpu_idle, int context_switches,
                  int nr_processes,
                  proc_t **processes);

void print_process_info(proc_t *proc);

#endif //SCHEDULER_REPORTER_H
