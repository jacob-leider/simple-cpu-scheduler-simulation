//
// Created by Jacob Leider on 3/6/23.
//

#include <stdio.h>
#include <stdlib.h>
#include "traffic_generator.h"


unsigned int generate_cpu_burst() {
    // generate a random number between 0 and 530
    unsigned int r = arc4random() % 530;
    // assign burst time depending on r: note on distribution
    int burst;
    if (r < 150) {
        burst = 1;
    } else if (r < 270) {
        burst = 2;
    } else if (r < 360) {
        burst = 3;
    } else if (r < 420) {
        burst = 4;
    } else if (r < 450) {
        burst = 5;
    } else if (r < 475) {
        burst = 6;
    } else if (r < 495) {
        burst = 7;
    } else if (r < 510) {
        burst = 8;
    } else if (r < 520) {
        burst = 20;
    } else if (r < 525) {
        burst = 40;
    } else if (r < 528) {
        burst = 80;
    } else {                    //528 < r < 30
        burst = 160;
    }
    return burst;
}

unsigned int generate_io_burst(unsigned int cpu_burst) {
    // generate random number
    unsigned int r = arc4random() % 100;
    unsigned int io_burst;
    if (cpu_burst > 8) {
        io_burst = ((9 * r) / 10) + 10;
    } else {
        io_burst = (r / 10) + 10;
    }
    return io_burst;
}

unsigned int generate_reps(unsigned int cpu_burst) {
    unsigned int r = arc4random() % 100;
    unsigned int reps;
    if (cpu_burst > 8) {
        reps = (r / 20) + 1;
    } else {
        reps = (r / 2) + 50;
    }
    return reps;
}

unsigned int assign_priority(unsigned int cpu_burst) {
    unsigned int priority;
    unsigned int r = arc4random() % 10;
    if (cpu_burst > 8) {
        priority = 3 - (r * r) / 50;
    } else {
        priority = r % 3 + 1;
    }
    return priority;
}

int generate_traffic(unsigned int nr_processes) {
    FILE *fp;
    if (!(fp = fopen("traffic.txt", "w"))) {
        fprintf(stderr, "Failed to generate traffic (error creating file \"traffic.txt\")");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "// PID | CPU burst | IO burst | Repetitions | Priority\n");
    unsigned int cpu_burst, io_burst, reps, priority;
    for (int i = 0; i < nr_processes; i++) {
        cpu_burst = generate_cpu_burst();
        io_burst = generate_io_burst(cpu_burst);
        reps = generate_reps(cpu_burst);
        priority = assign_priority(cpu_burst);
        fprintf(fp, "%d %d %d %d %d\n", i, cpu_burst, io_burst, reps, priority);
    }
    fclose(fp);
    return 1;
}
