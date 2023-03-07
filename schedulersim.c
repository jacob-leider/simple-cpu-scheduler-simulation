#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "reporter.h"
#include "traffic_generator.h"

#define NR_SCHED_ALGS   2

#define QUANTUM         5

#define PRIORITY_HIGH   3
#define PRIORITY_MED    2
#define PRIORITY_LOW    1

#define RUNNING         3
#define READY           2
#define WAITING         1
#define TERMINATED      -1


// a general node used as links to implement a runqueue
typedef struct node {
    int priority;
    proc_t *process;
    struct node *next;
} node_t;



static long time_elapsed = 0;
static int context_switches = 0; // ONLY iterated when a call to context_switch() is successful
static long cpu_in_use = 0, cpu_idle = 0;
static proc_t **processes;
static int nr_processes;
static int finished_processes = 0;
static proc_t *live_proc;  // points to the process running at any given time
static char *sched_alg;
static int sched_alg_index;
const char* sched_algs[] = {
        "FCFS",
        "RR"
};

// pointers to the head & tail of each priority level's associated queue
static node_t *high_head, *med_head, *low_head,
        *high_tail, *med_tail, *low_tail;


// check if a string is contained in the 'sched_algs' list
// if contained, sets the global 'sched_alg_index'
int valid_sched_alg(const char *s) {
    for (int i = 0; i < NR_SCHED_ALGS; i++) {
        if (strcmp(sched_algs[i], s) == 0) {
            sched_alg_index = i;
            return 1;
        }
    }
    return 0;
}


// push to a specified queue
void push_to_runqueue(proc_t *proc, node_t **rq_head, node_t **rq_tail) {
    // temporary link storing input process
    node_t *proc_node = malloc(sizeof(node_t));
    proc_node->process = proc;
    proc_node->next = NULL;
    proc_node->priority = proc->priority;

    // if queue is empty, this link becomes the head and
    // the tail. Otherwise, this link becomes the tail
    if (*rq_head == NULL) {
        *rq_head = *rq_tail = proc_node;
    } else {
        (*rq_tail)->next = proc_node;
        *rq_tail = proc_node;
    }
}

// push to the queue matching the priority of a given process
void push(proc_t *proc) {
    // the priority of a process determines the queue to which
    // it will be added
    node_t **head, **tail;
    switch (proc->priority) {
        case PRIORITY_HIGH:
            head = &high_head;
            tail = &high_tail;
            break;
        case PRIORITY_MED:
            head = &med_head;
            tail = &med_tail;
            break;
        case PRIORITY_LOW:
            head = &low_head;
            tail = &low_tail;
            break;
    }
    push_to_runqueue(proc, head, tail);
}


// poll from the highest priority nonempty queue
proc_t *poll() {
    // if high level queue isn't empty, pop from queue of HIGH priority
    if (high_head != NULL) {
        node_t *curr = high_head;
        if (curr != NULL) {
            proc_t *popped = curr->process;
            high_head = (high_head)->next;
            free(curr);
            return popped;
        }
    }
    // otherwise, if med level queue isn't empty, pop from queue of MED priority
    if (med_head != NULL) {
        node_t *curr = med_head;
        if (curr != NULL) {
            proc_t *popped = curr->process;
            med_head = (med_head)->next;
            free(curr);
            return popped;
        }
    }
    // otherwise, if low level queue isn't empty, pop from queue of LOW priority
    if (low_head != NULL) {
        node_t *curr = low_head;
        if (curr != NULL) {
            proc_t *popped = curr->process;
            low_head = (low_head)->next;
            free(curr);
            return popped;
        }
    }
    return NULL; // Otherwise, all processes are in waiting or terminated states: return NULL
}

// poll from a specified queue
proc_t *poll_from_runqueue(node_t **rq_head) {
    // if high level queue isn't empty, pop from queue of HIGH priority
    if (*rq_head != NULL) {
        node_t *curr = *rq_head;
        if (curr != NULL) {
            proc_t *popped = curr->process;
            *rq_head = (high_head)->next;
            free(curr);
            return popped;
        }
    }
    // return NULL if queue is empty
    return NULL;
}


// load processes from traffic.txt into the appropriate runqueues
void load_processes(FILE *f) {
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, f) != -1) {
        if (line[0] != '/' && line[1] != '/') {                     // if line is not a comment
            // create a process with fields from traffic.txt
            proc_t *proc = malloc(sizeof(proc_t));
            sscanf(line, "%d %d %d %d %d",
                   &proc->id,
                   &proc->cpu_burst,
                   &proc->io_burst,
                   &proc->reps,
                   &proc->priority);
            // set state to 'ready-to-run' and add to appropriate queue
            proc->state = READY;
            switch (sched_alg_index) {
                case 0:
                    push_to_runqueue(proc, &high_head, &high_tail);
                    break;
                case 1:
                    push(proc);
                    break;
                default:
                    break;
            }
            nr_processes++;
        }
    }

    // array of processes - add each process
    processes = malloc(nr_processes * sizeof(proc_t));

    node_t *curr = *&high_head;
    int i = 0;
    while (curr) {
        processes[i++] = curr->process;
        curr = curr->next;
    }
    curr = *&med_head;
    while (curr) {
        processes[i++] = curr->process;
        curr = curr->next;
    }
    curr = *&low_head;
    while (curr) {
        processes[i++] = curr->process;
        curr = curr->next;
    }
}

// called when a process terminates or is booted from the CPU
proc_t *context_switch() {
    proc_t *next_proc;
    switch (sched_alg_index) {
        case 0:
            if ((next_proc = poll_from_runqueue(&high_head))) { // if any jobs in queue, this should be true
                // RUN this process
                next_proc->state = RUNNING;

                if (!next_proc->start_time) {
                    next_proc->start_time = time_elapsed;
                }

                if (next_proc->burst_countdown <= 0) {
                    next_proc->burst_countdown = next_proc->cpu_burst;
                    next_proc->reps--;
                }
            }
            break;
        case 1:
            if ((next_proc = poll())) { // if any jobs in queue, this should be true
                // RUN this process
                next_proc->state = RUNNING;

                if (!next_proc->start_time) {
                    next_proc->start_time = time_elapsed;
                }

                if (next_proc->burst_countdown <= 0) {
                    next_proc->burst_countdown = next_proc->cpu_burst;
                    next_proc->reps--;
                }

                next_proc->quantum_countdown = QUANTUM;
            }
            break;
        case 2:
            //break;
        default:
            exit(EXIT_FAILURE);
    }
    return next_proc;
}


// First-Come-First-Serve simulation
void run_FCFS() {
    printf("RUNNING FCFS...\n");
    while (1) {
        if (live_proc) {
            live_proc->burst_countdown -= 1;

            // move live process to io wait queue if burst countdown is up
            // and run the next available process
            if (live_proc->burst_countdown <= 0) {
                live_proc->reps -= 1;
                if (live_proc->reps <= 0) {
                    // if process complete, send to terminated state and increase
                    // number of finished processes
                    live_proc->state = TERMINATED;
                    finished_processes += 1;
                } else {
                    // if process incomplete, send to waiting state and reset io countdown
                    live_proc->state = WAITING;
                    live_proc->io_countdown = live_proc->io_burst;
                }
                live_proc = NULL;
            }
            cpu_in_use++;
        } else {    // if no process is running, run the highest priority in-order process
            if (!(live_proc = context_switch())) {
                cpu_idle++;
            } else {
                context_switches++;
            }
        }
        // check if any processes have finished io countdown
        for (int i = 0; i < nr_processes; i++) {
            proc_t *proc = processes[i];
            if (proc->state == WAITING) {
                if ((proc->io_countdown -= 1) <= 0 ) {
                    if (proc->reps <= 0) {
                        proc->state = TERMINATED;
                        proc->end_time = time_elapsed;
                        finished_processes++;
                    } else {
                        proc->state = READY;
                        proc->wait_time += 1;
                        push_to_runqueue(proc, &high_head, &high_tail);
                    }
                }
            } else if (proc->state == READY) {
                proc->wait_time += 1;
            }
        }
        // break if all processes are finished
        if (finished_processes >= nr_processes) {
            break;
        }
        // increment total runtime
        time_elapsed++;
    }
    report(cpu_in_use, cpu_idle, context_switches, nr_processes, processes);
    // free all the mem
    for (int i = 0; i < nr_processes; i++) free(processes[i]);
}

// Round-Robin simulation
void run_RR() {
    printf("RUNNING RR...\n");
    while (1) {
        if (live_proc) {
            live_proc->burst_countdown -= 1;
            live_proc->quantum_countdown -= 1;

            // move live process to io wait queue if burst countdown is up
            // and run the next available process. If the quantum countdown
            // is up, push the live process back into the run-queue
            if (live_proc->burst_countdown <= 0) {
                live_proc->reps -= 1;
                if (live_proc->reps <= 0) {
                    // if process complete, send to terminated state and increase
                    // number of finished processes
                    live_proc->state = TERMINATED;
                    finished_processes += 1;
                } else {
                    // if process incomplete, send to waiting state and reset io countdown
                    live_proc->state = WAITING;
                    live_proc->io_countdown = live_proc->io_burst;
                }
                live_proc = NULL;
            } else if (live_proc->quantum_countdown <= 0) {
                // if the live process's timeslice is up, send it back into the runqueue
                live_proc->state = READY;
                push(live_proc);
                live_proc = NULL;
            }
            cpu_in_use++;
        } else {
            // if no process is running, run the highest priority in-order process
            // if none are available, increase idle time
            if (!(live_proc = context_switch())) {
                cpu_idle++;
            } else {
                context_switches++;
            }
        }
        // check if any processes have finished io countdown
        for (int i = 0; i < nr_processes; i++) {
            proc_t *proc = processes[i];
            if (proc->state == WAITING) {
                if ((proc->io_countdown -= 1) <= 0 ) {
                    if (proc->reps <= 0) {
                        proc->state = TERMINATED;
                        proc->end_time = time_elapsed;
                        finished_processes++;
                    } else {
                        proc->state = READY;
                        proc->wait_time += 1;
                        push(proc);
                    }
                }
            } else if (proc->state == READY) {
                proc->wait_time += 1;
            }
        }
        // break if all processes are finished
        if (finished_processes >= nr_processes) {
            break;
        }
        // increment total runtime
        time_elapsed++;
    }
    report(cpu_in_use, cpu_idle, context_switches, nr_processes, processes);
    // free all the mem
    for (int i = 0; i < nr_processes; i++) free(processes[i]);
}

// Shortest-Job-First Simulation
void run_SJF() {

}


int main(int argc, char *argv[]) {

    // validate command line args
    if (argc != 3) {
        fprintf(stderr, "Usage: $ ./<executable> <algorithm> <number of processes>");
        exit(EXIT_FAILURE);
    }
    sched_alg = argv[1];
    if (!valid_sched_alg(sched_alg)) {
        fprintf(stderr, "Invalid scheduling algorithm. Try any of the following:"
                        "\n\t\"FCFS\" (First Come First Serve), "
                        "\n\t\"RR\" (Round Robin), "
                        "\n\t\"SJF\" (Shortest Job First)\n");
        exit(EXIT_FAILURE);
    }

    // generate sample traffic for the scheduler
    generate_traffic(atoi(argv[2]));

    // read traffic and load processes
    FILE *fp;
    fp = fopen("traffic.txt", "r");
    load_processes(fp);
    fclose(fp);

    // run according to the selected scheduling algorithm
    switch (sched_alg_index) {
        case 0:
            run_FCFS();
            break;
        case 1:
            run_RR();
            break;
        case 2:
            // run_CFS();
            break;
        default:
            exit(EXIT_FAILURE);
    }

    return 0;
}