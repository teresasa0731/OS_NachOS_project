# OS_NachOS_project
## Part 1: trace code
## Part 2. Implementation
### 2.1 Implement a multilevel feedback queue scheduler with aging mechanism as described below:
#### (a) There are 3 levels of queues: L1, L2 and L3. L1 is the highest level queue, and L3 is the lowest level queue.
#### (b) All processes must have a valid scheduling priority between 0 to 149. Higher value means higher priority. So 149 is the highest priority, and 0 is the lowest priority.
#### (c) A process with priority between 0 - 49 is in L3 queue, priority between 50 - 99 is in L2 queue, and priority between 100 - 149 is in L1 queue.
#### (d) L1 queue uses preemptive SRTN (shortest remaining time first) scheduling algorithm. If current thread has the lowest remaining burst time, it should not be preempted by the threads in the ready queue. The burst time (job execution time) is provided by user when execute the test case.
#### (e) L2 queue uses a FCFS (First-Come First-Served) scheduling algorithm which means lower thread ID has higher priority.
#### (f) L3 queue uses a round-robin scheduling algorithm with time quantum 200 ticks (you should select a thread to run once 200 ticks elapsed). If two threads enter the L3 queue with the same priority, either one of them can execute first.
#### (g) An aging mechanism must be implemented, so that the priority of a process is increased by 10 after waiting for more than 400 ticks (Note: The operations of preemption and priority updating can be delayed until the next timer alarm interval).
### 2.2 Add a command line argument -epb for nachos to initialize priority of process. E.g., the
#### command below will launch 2 processes: hw2_test1 with initial priority 40 and burst time 5000,and hw2_test2 with initial priority 80 and burst time 4000. $ userprog/nachos -epb test/hw2_test1 40 5000 -epb test/hw2_test2 80 4000
### 2.3 Add a debugging flag z and use the DEBUG('z', expr) macro (defined in debug.h) to print following messages. Replace {...} to the corresponding value.
