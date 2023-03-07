//
// Created by Jacob Leider on 3/6/23.
//

#ifndef SCHEDULER_TRAFFIC_GENERATOR_H
#define SCHEDULER_TRAFFIC_GENERATOR_H

unsigned int generate_cpu_burst();
unsigned int generate_io_burst(unsigned int cpu_burst);
unsigned int generate_reps(unsigned int cpu_burst);
unsigned int assign_priority(unsigned int cpu_burst);
int generate_traffic(unsigned int nr_processes);

#endif //SCHEDULER_TRAFFIC_GENERATOR_H
