// C program for implementation of Simulation 
#include<stdio.h> 
#include<limits.h>
#include<stdlib.h>
#include "process.h"
#include "util.h"


// Function to find the waiting time for all  
// processes
void findWaitingTimeRR(ProcessType plist[], int n, int quantum) 
{ 
    // 1. Create rem_bt[] array
    int rem_bt[n];
    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
    }

    int current_time = 0; // 3. Initialize time

    // 4. Keep traversing processes
    while (1) {
        int all_processes_done = 1; // Flag to check if all processes are finished

        // Traverse all processes
        for (int i = 0; i < n; i++) {
            
            // If process has remaining time
            if (rem_bt[i] > 0) {
                all_processes_done = 0; // Not all are done

                if (rem_bt[i] > quantum) {
                    // (i) t = t + quantum
                    current_time += quantum;
                    // (ii) bt_rem[i] -= quantum;
                    rem_bt[i] -= quantum;
                } else { // Last cycle for this process
                    // (i) t = t + bt_rem[i];
                    current_time += rem_bt[i];
                    
                    // (ii) plist[i].wt = t - plist[i].bt
                    // (Assuming art=0 as per lab guide for RR)
                    plist[i].wt = current_time - plist[i].bt;
                    
                    // (iii) bt_rem[i] = 0; // This process is over
                    rem_bt[i] = 0;
                }
            }
        }

        // If we looped through and all processes are done, break
        if (all_processes_done == 1) {
            break;
        }
    }
} 

// Function to find the waiting time for all  
// processes (Pre-emptive SJF / SRTF)
void findWaitingTimeSJF(ProcessType plist[], int n)
{ 
    int rem_bt[n]; // Array to store remaining burst time
    for (int i = 0; i < n; i++) {
        rem_bt[i] = plist[i].bt;
    }

    int completed_processes = 0;
    int current_time = 0;
    int shortest_job = 0;
    int min_remaining_time = INT_MAX; // From <limits.h>
    int found_job = 0;

    // 1. Traverse until all process gets completely executed.
    while (completed_processes < n) {
        
        // Find process with minimum remaining time at current time lap
        found_job = 0;
        min_remaining_time = INT_MAX;
        
        for (int i = 0; i < n; i++) {
            // Check if process has arrived, has time left, and is the new shortest
            if (plist[i].art <= current_time && rem_bt[i] > 0 && rem_bt[i] < min_remaining_time) {
                min_remaining_time = rem_bt[i];
                shortest_job = i;
                found_job = 1;
            }
        }

        // If no job is ready (CPU idle), increment time and continue
        if (found_job == 0) {
            current_time++;
            continue;
        }

        // Reduce its time by 1
        rem_bt[shortest_job]--;

        // Check if its remaining time becomes 0
        if (rem_bt[shortest_job] == 0) {
            // Increment the counter of process completion
            completed_processes++;
            
            // Completion time of current process = current_time + 1;
            int completion_time = current_time + 1;

            // Calculate waiting time for each completed process.
            // wt[i] = Completion time - arrival_time - burst_time
            plist[shortest_job].wt = completion_time - plist[shortest_job].art - plist[shortest_job].bt;

            // Ensure waiting time is not negative
            if (plist[shortest_job].wt < 0) {
                plist[shortest_job].wt = 0;
            }
        }
        
        // Increment time lap by one
        current_time++;
    }
} 

// Function to find the waiting time for all  
// processes (FCFS - Corrected to handle arrival times)
void findWaitingTime(ProcessType plist[], int n)
{ 
    // We need to track the time the CPU is free
    int cpu_free_time = 0;

    // Waiting time for the first process is 0
    plist[0].wt = 0;
    
    // Completion time for the first process
    cpu_free_time = plist[0].art + plist[0].bt;

    // calculating waiting time for remaining processes
    for (int  i = 1; i < n ; i++ ) 
    { 
        // When will this process start?
        // It's either when it arrives (plist[i].art) or
        // when the CPU is free (cpu_free_time), whichever is later.
        int start_time = (cpu_free_time > plist[i].art) ? cpu_free_time : plist[i].art;
        
        // Waiting time = Start Time - Arrival Time
        plist[i].wt = start_time - plist[i].art;

        // Update when the CPU will be free next
        cpu_free_time = start_time + plist[i].bt;
    } 
} 
  
// Function to calculate turn around time 
void findTurnAroundTime( ProcessType plist[], int n)
{ 
    // calculating turnaround time by adding bt[i] + wt[i] 
    for (int  i = 0; i < n ; i++) 
        plist[i].tat = plist[i].bt + plist[i].wt; 
} 
  
// Function to sort the Process acc. to priority
// (Comparator for qsort)
int my_comparer(const void *this, const void *that)
{ 
    // 1. Cast this and that into (ProcessType *)
    ProcessType *p1 = (ProcessType *)this;
    ProcessType *p2 = (ProcessType *)that;
  
    // 2. return difference
    // This sorts in ascending order (e.g., priority 1, 2, 3...)
    // Assumes lower number = higher priority
    return p1->pri - p2->pri;
} 

//Function to calculate average time 
void findavgTimeFCFS( ProcessType plist[], int n) 
{ 
    //Function to find waiting time of all processes 
    findWaitingTime(plist, n); 
  
    //Function to find turn around time for all processes 
    findTurnAroundTime(plist, n); 
  
    //Display processes along with all details 
    printf("\n*********\nFCFS\n");
}

//Function to calculate average time 
void findavgTimeSJF( ProcessType plist[], int n) 
{ 
    //Function to find waiting time of all processes 
    findWaitingTimeSJF(plist, n); 
  
    //Function to find turn around time for all processes 
    findTurnAroundTime(plist, n); 
  
    //Display processes along with all details 
    printf("\n*********\nSJF\n");
}

//Function to calculate average time 
void findavgTimeRR( ProcessType plist[], int n, int quantum) 
{ 
    //Function to find waiting time of all processes 
    findWaitingTimeRR(plist, n, quantum); 
  
    //Function to find turn around time for all processes 
    findTurnAroundTime(plist, n); 
  
    //Display processes along with all details 
    printf("\n*********\nRR Quantum = %d\n", quantum);
}

//Function to calculate average time 
void findavgTimePriority( ProcessType plist[], int n) 
{ 
    // 1- Sort the processes according to the priority.
    qsort(plist, n, sizeof(ProcessType), my_comparer);
  
    // 2- Now simply apply FCFS algorithm (the corrected one)
    findWaitingTime(plist, n);
    
    // 3- Find turn around time
    findTurnAroundTime(plist, n);
  
    //Display processes along with all details 
    printf("\n*********\nPriority\n");
}

void printMetrics(ProcessType plist[], int n)
{
    int total_wt = 0, total_tat = 0; 
    float awt, att;
    
    // Added Arrival Time and Priority to the printout for clarity
    printf("\tProcesses\tArrival\tBurst\tPriority\tWaiting\tTurn around\n"); 
  
    // Calculate total waiting time and total turn  
    // around time 
    for (int  i=0; i<n; i++) 
    { 
        total_wt = total_wt + plist[i].wt; 
        total_tat = total_tat + plist[i].tat; 
        printf("\t%d\t\t%d\t%d\t%d\t\t%d\t\t%d\n", 
               plist[i].pid, plist[i].art, plist[i].bt, plist[i].pri, plist[i].wt, plist[i].tat); 
    } 
  
    awt = ((float)total_wt / (float)n);
    att = ((float)total_tat / (float)n);
    
    printf("\nAverage waiting time = %.2f", awt); 
    printf("\nAverage turn around time = %.2f\n", att); 
} 

ProcessType * initProc(char *filename, int *n) 
{
    FILE *input_file = fopen(filename, "r");
    if (!input_file) {
        fprintf(stderr, "Error: Invalid filepath\n");
        fflush(stdout);
        exit(0);
    }

    ProcessType *plist = parse_file(input_file, n);
  
    fclose(input_file);
  
    return plist;
}
  
// Driver code 
int main(int argc, char *argv[]) 
{ 
    int n; 
    int quantum = 2; // Default quantum for RR

    ProcessType *proc_list;
  
    if (argc < 2) {
       fprintf(stderr, "Usage: ./schedsim <input-file-path>\n");
       fflush(stdout);
       return 1;
     }
    
  // FCFS
    n = 0;
    // **** FIX WAS HERE ****
    proc_list = initProc(argv[1], &n);
  
    findavgTimeFCFS(proc_list, n);
    
    printMetrics(proc_list, n);
    
    free(proc_list); // Free memory
  
  // SJF
    n = 0;
    proc_list = initProc(argv[1], &n);
   
    findavgTimeSJF(proc_list, n); 
   
    printMetrics(proc_list, n);
    
    free(proc_list); // Free memory
  
  // Priority
    n = 0; 
    proc_list = initProc(argv[1], &n);
    
    findavgTimePriority(proc_list, n); 
    
    printMetrics(proc_list, n);
    
    free(proc_list); // Free memory
    
  // RR
    n = 0;
    proc_list = initProc(argv[1], &n);
    
    findavgTimeRR(proc_list, n, quantum); 
    
    printMetrics(proc_list, n);
    
    free(proc_list); // Free memory
    
    return 0; 
}