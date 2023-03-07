# simple-cpu-scheduler-simulation

## Usage

To build the program from the command line on a UNIX-like system, link and compile the files schedulersim.c, reporter.c, traffic_generator.c as follows:

_cc schedulersim.c reporter.c traffic_generator.c_

This program takes two arguments: An algorithm name (either FCFS for First-Come-First-Serve, or RR for Round Robin), and a positive integer. The latter represents the workload that the program will simulate. For example,

_./a.out RR 25_

would simulate the scheduling and execution of 25 processes using a Round Robin scheduling algorithm.
