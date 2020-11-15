#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>



/*******************************************************************************************************************
                                                    Ashesi University
                                        Operating Systems 2020 Lab Assignment 5
                                            Owusu-Banahene K. Osei (18122021)
*******************************************************************************************************************/


typedef struct {
     int process_number;
     int arrival_time;
     int burst_time;

     int remaining_burst_time;

     int start_time;                // These two are for statistical purposes
     int end_time;                  //
} Process;


typedef struct ProcessLinkedListNode {
    Process *process;
    struct ProcessLinkedListNode *previous;
    struct ProcessLinkedListNode *next;
}   ProcessLinkedListNode;


typedef struct {
    ProcessLinkedListNode *start;
    ProcessLinkedListNode *end;
} ProcessList;


typedef struct QueuedProcess {
     Process *process;
     struct QueuedProcess *next_queued_process;
     int time_slice;
} QueuedProcess;


typedef struct {
    QueuedProcess *start;
    QueuedProcess *end;
} ProcessQueue;


/**


    FUNCTION DECLARAIONS
*/
ProcessQueue* schedule(FILE* sourceFile);
ProcessQueue* scheduleFirstComeFirstServed(FILE* sourceFile);
ProcessQueue* schedulePreemptiveShortestJobNext(FILE* sourceFile);

void averageWaitingTime(ProcessList *processes, ProcessQueue *process_queue);
void averageTurnAroundTime(ProcessList *processes, ProcessQueue *process_queue);


ProcessList* readProcessList(FILE* sourceFile);
ProcessList* parseCharArrayAsProcessList(char *array);

void printProcess(Process *process);
void printProcesses(ProcessList *processes);
void addProcess(Process *process, ProcessList *list);
void removeProcessFromList(ProcessLinkedListNode *process_node, ProcessList *processes);

void enqueueProcess(Process *process,int time_slice, int *cpu_timeline, ProcessQueue *process_queue);
void sortProcessListByArrival(ProcessList *processes);

void printScheduleStats(ProcessList *processes);
void printProcessQueue(ProcessQueue *process_queue);

ProcessLinkedListNode* selectArrivedProcessWithShortestRemainingBurst(ProcessList *processes, int cpu_timeline);




/**
    Program entry point
*/
int main(int argc, char *argv[]){
    printf("STARTED\n");
    // Check to make sure we have correct number of arguments
    if (argc != 2){
        printf("You entered the wrong number of argumentTs (%i). Include the filename of process list file on command line\n\n", argc);
        printf("use format <compiled c file> <file with process list>\n\n");
        return 1;           // Return 1 as error code
    }

    // Open both files
    FILE* input_file = fopen(argv[1], "r");
    // Make sure file opens successfully
    if (input_file == NULL){
        printf("file '%s' could not be opened", argv[1]);
        return 2;           // Error code for file not opening
    }

    ProcessQueue* process_queue = schedule(input_file);
    printProcessQueue(process_queue);


    printf("\n\n ----------- COMPLETED -----------------");

    return 0;
}



/**
    This function accepts an open file with the input processes and schedules them.

    It falls back to one of two schedulers. One uses the First Come First served principle. The other is Preemptive shortest job Next
*/
ProcessQueue* schedule(FILE* sourceFile){
    scheduleFirstComeFirstServed(sourceFile);
    //schedulePreemptiveShortestJobNext(sourceFile);
}


/**
    This is a scheduler that uses the First Come First Served approach
*/
ProcessQueue* scheduleFirstComeFirstServed(FILE* sourceFile){
    ProcessList *processes = readProcessList(sourceFile);    // Read the processes in the input file
    if (processes == NULL){
        printf("ERROR: Reading processes from specified text file failed");
        return NULL;
    }

    sortProcessListByArrival(processes);

    ProcessQueue *process_queue = malloc(sizeof(ProcessQueue));
    process_queue->start = NULL; process_queue->end = NULL;

    ProcessLinkedListNode *runner = processes->start;

    int cpu_timeline = 0;       // CPU timeline tracker

    while(runner != NULL){
        enqueueProcess(runner->process, runner->process->burst_time, &cpu_timeline, process_queue);
        runner = runner->next;          // Move to next Process
    }


    printScheduleStats(processes);

    return process_queue;
}



/**
    This is a scheduler that uses the Preemptive Shortext Job Next approach
*/
ProcessQueue* schedulePreemptiveShortestJobNext(FILE* sourceFile){
    ProcessList *processes = readProcessList(sourceFile);    // Read the processes in the input file
    if (processes == NULL){
        printf("ERROR: Reading processes from specified text file failed");
        return NULL;
    }

    ProcessQueue *process_queue = malloc(sizeof(ProcessQueue));
    process_queue->start = NULL; process_queue->end = NULL;

    int cpu_timeline = -1;          // seed at -1 so loop starts it at 0
    int process_timeslice_count = 0;

    ProcessLinkedListNode *current_process_node;
    ProcessLinkedListNode *last_process_node = NULL;
    while(processes->start != NULL){
        cpu_timeline += 1;
        //printProcesses(processes);


        //printf("\nAnother Iteration \t cpu_timeline: %i \t timeslice: %i", cpu_timeline, process_timeslice_count);
        current_process_node = selectArrivedProcessWithShortestRemainingBurst(processes, cpu_timeline);
        //printf("\t\tProcess %i selected\n", current_process_node->process->process_number);
        if (last_process_node == NULL)
            last_process_node = current_process_node;

        if (current_process_node == NULL){          // No process to execute at the moment
            process_timeslice_count = 0; last_process_node = NULL;
            continue;
        }

        if (last_process_node != current_process_node){
            int mock_cpu_timeline = cpu_timeline - process_timeslice_count;         // TO DO: Fix this inelegant workaround
            last_process_node->process->remaining_burst_time += process_timeslice_count;           // TO DO: Another (same below) inelegant workaround to avoid double burstTime deduction in 'enqueue..."
            enqueueProcess(last_process_node->process, process_timeslice_count, &mock_cpu_timeline, process_queue);
            process_timeslice_count = 0;        // reset timeslice count
        }

        // Reduce remaining time for current process by 1 and increment process_timeslice_count
        current_process_node->process->remaining_burst_time -= 1;
        process_timeslice_count += 1;
        last_process_node = current_process_node;

        if (current_process_node->process->remaining_burst_time <= 0){          // If process has completed execution
            last_process_node->process->remaining_burst_time += process_timeslice_count;           // TO DO: Another inelegant workaround to avoid double burstTime deduction in 'enqueue..."
            enqueueProcess(last_process_node->process, process_timeslice_count, &cpu_timeline, process_queue);
            process_timeslice_count = 0;        // reset timeslice count

            printf("\n\tAbout to remove Process %i", current_process_node->process->process_number);
            current_process_node->process->end_time = cpu_timeline;
            removeProcessFromList(current_process_node, processes);
            last_process_node = NULL;
        }

    }

    printScheduleStats(processes);
    return process_queue;
}


/**
    This function takes a list of processes and selects the next one to execute in the the Preemptive Shortest Job Next scheduling
*/
ProcessLinkedListNode* selectArrivedProcessWithShortestRemainingBurst(ProcessList *processes, int cpu_timeline){
    ProcessLinkedListNode* ready_node = NULL;

    ProcessLinkedListNode *runner = processes->start;

    while(runner != NULL){
        if (runner->process->arrival_time <= cpu_timeline &&    // Process has arrived as at this point in cpu timeline
                (ready_node == NULL || runner->process->remaining_burst_time < ready_node->process->remaining_burst_time)
            ){
            ready_node = runner;
        }
        runner = runner->next;
    }

    return ready_node;
}





/**
    Given a file pointer, this function will parse it and return a ProcessList (which is a linked list of processes)
*/
ProcessList* readProcessList(FILE* sourceFile){
    if (sourceFile == NULL)
        return NULL;

    // Create buffer to hopefully read all of file in one go
    const int bufferSize = 500;
    char *buffer = malloc(bufferSize * sizeof(char));

    if (buffer == NULL)
        return NULL;        // Make sure there was enough memory to malloc

    int len = fread(buffer, sizeof(char), bufferSize, sourceFile);    // Hopefully read all at a go
    buffer[len] = '\0';              // Add string terminator as sentinel

    ProcessList *processes = parseCharArrayAsProcessList(buffer);

    free(buffer);       // Free heap allocated memory
    return processes;
}



/**
    Given a "string" this method is able to parse it into a ProcessList
*/
ProcessList* parseCharArrayAsProcessList(char *array){
    char *runner = array;
    int spaceCount = 0;
    int lineCount = 0;

    // Skip first line with table headings
    while(*runner != '\n' && *runner != '\0')
        runner += 1;
    if (*runner == '\n')         // The above loop ends when '\n' is reached. Now skip that too
        runner += 1;
    array = runner;             // change starting position of string

    while(*runner != '\0' && *runner != EOF){       // Count spaces and newlines
        if (*runner == ' ')
            spaceCount++;
        else if(*runner == '\n')
            lineCount++;

        runner += 1;   // move pointer to next character
    }

    int rows = *array == '\0' ? 0 : lineCount + 1;              //check for empty file
    //int columns =rows == 0 ? 0 : (spaceCount / rows) + 1;
    int columns = 3;        // Assume input file always has exactly 3 columns

    if (rows == 0 || columns == 0)      // Check for empty file
        return NULL;

    ProcessList *processList = malloc(sizeof(ProcessList));
    // TO DO: Ensure malloc was successful to avoid error
    processList->end = NULL; processList->start = NULL;

    int (*matrix)[rows] = malloc(rows * columns * sizeof(int));       // heap 2D array


    //Split at spaces
    char delimiters[] = " \n";
    int toggle = 0;
    char *token = strtok(array, delimiters);  // Get first token
    Process *current_process_being_parsed = malloc(sizeof(Process));
    current_process_being_parsed->start_time = INT_MIN;

    char process_num_int_string[10];        // placeholder for string integer from process name like "P1" or "P10"

    while(token != NULL){

        switch(toggle){
            case 0:     // First value in row is process num... e.g. "P1"
                memset(process_num_int_string, '\0', sizeof(process_num_int_string));
                strncpy(process_num_int_string, token + 1, strlen(token));
                int process_num = atoi(process_num_int_string);

                // Set current process' number
                current_process_being_parsed->process_number = process_num;
                break;
            case 1:     // Second row value is Burst time as an int
                current_process_being_parsed->burst_time = atoi(token);
                current_process_being_parsed->remaining_burst_time = current_process_being_parsed->burst_time;
                break;
            case 2:     // Third row value is Arrival time as an int
                current_process_being_parsed->arrival_time = atoi(token);

                addProcess(current_process_being_parsed, processList);
                current_process_being_parsed = malloc(sizeof(Process));
                current_process_being_parsed->start_time = INT_MIN;

                break;
        }

        token = strtok(NULL, delimiters);        // get next
        toggle = (toggle + 1) % 3;    // toggle from 0 to 2... 0 for process number, 1 for burst time, 2 for arrival time
    }

    return processList;
}





//-------------------------------------------------- Diagnostic functions go here ----------------------------------------------
/**
    This is a diagnostic function that displays the data for a process
*/
void printProcess(Process *process){
    printf("\n\tProcess Number:%i \t ArrivalTime: %i \t BurstTime: %i \t RemainingBurstTime: %i \t CPU StartTime: %i \t CPU EndTime: %i",
                        process->process_number, process->arrival_time, process->burst_time, process->remaining_burst_time,
                                                                                            process->start_time, process->end_time);
}

/**
    This function takes a list of processes and displays them one at a time
*/
void printProcesses(ProcessList *processes){

    ProcessLinkedListNode *runner = processes->start;

    while(runner != NULL){
        printProcess(runner->process);
        runner = runner->next;
    }
}


/**
    This function takes a List of processes and displays stats like their Average Turnaround and Average Wait Times
*/
void printScheduleStats(ProcessList *processes){

    int totalWaitTime = 0;
    int totalTurnaroundTime = 0;
    float processCount = 0;

    ProcessLinkedListNode *current_node = processes->start;
    Process *current;

    int current_turnaround;
    int current_wait;
    while(current_node != NULL){
        if (current_node->process == NULL) continue;        // Skip list nodes with no process

        processCount += 1;      // increment process counter
        current = current_node->process;

        current_turnaround = current->end_time - current->arrival_time;
        current_wait = current_turnaround - current->burst_time;

        totalWaitTime += current_wait;
        totalTurnaroundTime += current_turnaround;

        current_node = current_node->next;
    }

    printf("------------------------ SCHEDULE STATS ---------------------------\n");
    printf("Average Wait Time: %f \t\t Average Turnaround Time: %f\n\n", totalWaitTime/processCount, totalTurnaroundTime/processCount);
}



/**
    This function displays a process queue as determined by a chosen process scheduler
*/
void printProcessQueue(ProcessQueue *process_queue){
    if (process_queue == NULL){
        printf("ERROR: Could not print 'NULL' process queue\n");
        return;
    }

    QueuedProcess *current_queued_process;

    current_queued_process = process_queue->start;

    printf("Process Execution Sequence. \tFormat: [Process <process_number> [<time_slice>]\n\n");
    printf("\n\tSTART ---- ");
    while(current_queued_process != NULL){
        if (current_queued_process->process == NULL) continue;

        printf("Process %i [%i] -->  ",
                    current_queued_process->process->process_number,
                    current_queued_process->time_slice);

        current_queued_process = current_queued_process->next_queued_process;
    }

    printf("--- END\n");

}


// ------------------------------------------ Helper Functions for Structs go here -------------------

/**
  This function adds a process to a specified list of processes (at the back of the list)
*/
void addProcess(Process *process, ProcessList *list){

    if (process == NULL || list == NULL) return;
    ProcessLinkedListNode *node = malloc(sizeof(ProcessLinkedListNode));
    node->previous = NULL;

    node->process = process;
    node->next = NULL;      // last process points to NULL;

    if(list->end != NULL){
        list->end->next = node;
        node->previous = list->end;
    }

    list->end = node;       // new process is always last in list

    if (list->start == NULL)
        list->start = node;
}



/**
    This functions adds a process to a queue... Used as a helper for the different schedulers
*/
void enqueueProcess(Process *process,int time_slice, int *cpu_timeline, ProcessQueue *process_queue){

    if (process->start_time < 0){       // Negative start time is used as sentinel for a process that hasn't been scheduled
        process->start_time = *cpu_timeline;
        //printf("\n\n Set Start time of Process %i to %i\n", process->process_number, *cpu_timeline);
    }

    int time_to_schedule = (process->remaining_burst_time < time_slice) ? process->remaining_burst_time : time_slice;       // Don't schedule more time than needed to complete process

    QueuedProcess *queued_process = malloc(sizeof(QueuedProcess));
    queued_process->process = process;
    queued_process->time_slice = time_to_schedule;

    // Update process to reflect new scheduled CPU Time
    process->remaining_burst_time -= time_to_schedule;

    *cpu_timeline += time_to_schedule;          // progress CPU timeline by scheduled amount
    if (process->remaining_burst_time <= 0){
        process->end_time = *cpu_timeline;         // Set end time of process if no burst time left to schedule
        //printf("\n\n Set End time of Process %i to %i\n", process->process_number, *cpu_timeline);

    }


    queued_process->next_queued_process = NULL;

    if (process_queue->end != NULL)
        process_queue->end->next_queued_process = queued_process;
    process_queue->end = queued_process;

    if (process_queue->start == NULL)
        process_queue->start = queued_process;
}



/**
    This function sorts a list of processes by their time of arrival
*/
void sortProcessListByArrival(ProcessList *processes){
    // TO DO: Implement Sorting by Arrival
}


/**
  This function removes a specified process from a list of processes
*/
void removeProcessFromList(ProcessLinkedListNode *process_node, ProcessList *processes){
    if (processes == NULL || process_node == NULL) return;

    if (process_node->previous != NULL)
        process_node->previous->next = process_node->next;
    if (process_node->next != NULL)
        process_node->next->previous = process_node->previous;

    if (processes->start == process_node)
        processes->start = process_node->next;
    if (processes->end == process_node)
        processes->end = process_node->previous;

    free(process_node);
}



