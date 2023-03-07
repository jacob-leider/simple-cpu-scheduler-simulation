#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reporter.h"

#define PRIORITY_HIGH   3
#define PRIORITY_MED    2
#define PRIORITY_LOW    1

#define RUNNING         3
#define READY           2
#define WAITING         1
#define TERMINATED      -1

// print a line at a given point
void print_status_line(long time_elapsed,
                       int nr_processes,
                       proc_t **processes,
                       proc_t *live_proc) {

    char *iostring = malloc(16 * sizeof(char));
    int c = 0;

    for (int i = 0; i < nr_processes; i++) {
        if (processes[i]->state == WAITING) {
            c += sprintf(&iostring[c], "%d ", processes[i]->id);
        }
    }

    if (!c) {
        strcpy(iostring, "xx");
    }

    if (live_proc) printf("│  %4ld %9d %9s     │\n", time_elapsed, live_proc->id, iostring);
    else printf("│  %4ld %9s %9s     │\n", time_elapsed, "xx", iostring);

    free(iostring);
}

// report stats at the end of a simulation: throughput, number of context switches,
// average wait time for each priority class,...
void report(long cpu_in_use, long cpu_idle, int context_switches,
                  int nr_processes,
                  proc_t **processes) {

    int turn_time = 0;

    int nr_high_procs = 0,
        nr_med_procs = 0,
        nr_low_procs = 0;

    int high_wait_time = 0,
        low_wait_time = 0,
        med_wait_time = 0,
        overall_wait_time = 0;

    double avg_high_wait_time,
           avg_med_wait_time,
           avg_low_wait_time,
           avg_overall_wait_time;


    for (int i = 0; i < nr_processes; i++) {
        proc_t *proc = processes[i];
        switch (proc->priority) {
            case PRIORITY_HIGH:
                high_wait_time += proc->wait_time;
                nr_high_procs++;
                break;
            case PRIORITY_MED:
                med_wait_time += proc->wait_time;
                nr_med_procs++;
                break;
            case PRIORITY_LOW:
                low_wait_time += proc->wait_time;
                nr_low_procs++;
                break;
        }
        overall_wait_time += proc->wait_time;
        turn_time += processes[i]->end_time;
    }

    avg_high_wait_time = ((double)high_wait_time) / nr_high_procs;
    avg_med_wait_time = ((double)med_wait_time) / nr_med_procs;
    avg_low_wait_time = ((double)low_wait_time) / nr_low_procs;
    avg_overall_wait_time = ((double)overall_wait_time) / nr_processes;

    printf("   CPU Busy Time: %ld\n", cpu_in_use);
    printf("   CPU Idle Time: %ld\n", cpu_idle);
    printf("   Avg. Throughput: %f\n", ((double )(cpu_in_use * 100) / (cpu_idle + cpu_in_use)));
    printf("   Avg. Wait times:\n");
    printf("    |-HIGH    : %f\n", avg_high_wait_time);
    printf("    |-MED     : %f\n", avg_med_wait_time);
    printf("    |-LOW     : %f\n", avg_low_wait_time);
    printf("    |-OVERALL : %f\n", avg_overall_wait_time);
    printf("   Context Switches: %d\n", context_switches);


}

// print a processes ID, priority and state
void print_process_info(proc_t *proc) {
    printf("ID: %d\nPRIO: %d\nSTATE: %d\n", proc->id, proc->priority, proc->state);
}