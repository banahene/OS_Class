#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

/**
    Structure for representing a list of numbers
*/
typedef struct {
     int *base_ref;
     int size;
} IntList;


/**
    Method Declarations
*/
void getNumbersListFromCommandLine(int argc, char **argv, int* array, IntList* nums);
void printListValues(int* start, int size);
void bubbleSortNumbers(int* start, int size);
void* getSortedListMaxValue(void* list);
void* getSortedListMinValue(void* list);
void* getSortedListMedianValue(void* list);
void* getListAverageValue(void* list);
void* getListStandardDeviation(void* list);





/**
  Main entry point to this program
*/
int main(int argc, char **argv){
    pthread_t avg_thread;
    pthread_t max_val_thread;
    pthread_t min_val_thread;
    pthread_t median_thread;
    pthread_t std_thread;

    if (argc < 2){
        printf("Enter the numbers needed on the command line");
        return 10;              // Exit code for improper command line arguments
    }

    //Allocate memory for integer list
    int (*numbers)[argc - 1] = malloc(sizeof(int) * (argc - 1));
    IntList* nums = malloc(sizeof(IntList));

    getNumbersListFromCommandLine(argc, argv, *numbers, nums);

    bubbleSortNumbers(nums->base_ref, nums->size);              // Sort all values


    // Spawn threads to do their jobs
    pthread_create(&max_val_thread, NULL, getSortedListMaxValue, nums);
    pthread_create(&min_val_thread, NULL, getSortedListMinValue, nums);
    pthread_create(&median_thread, NULL, getSortedListMedianValue, nums);
    //pthread_create(&avg_thread, NULL, getListAverageValue, nums);
    pthread_create(&std_thread, NULL, getListStandardDeviation, nums);

    // Stop main thread from exiting before other threads are done
    pthread_join(max_val_thread, NULL); pthread_join(min_val_thread, NULL);
    pthread_join(median_thread, NULL); //pthread_join(avg_thread, NULL);
    pthread_join(std_thread, NULL);

    //Free all allocated memory
    free(nums); free(numbers);
}


/**
    This function takes in argc and argv and returns an integer list of numbers
    that were passed via command line
*/
void getNumbersListFromCommandLine(int argc, char **argv, int* array, IntList* nums){
    for (int i = 1; i < argc; i++){
        //loop through argv and populate numbers array
        char* next_value = *(argv+ i);

        /**
        TO DO: Protect against illegal values
        if (!isdigit(next_value)){
            printf("'%s' is not a number", next_value);
            return;
        }
        */

        *(array + i - 1) = atoi(next_value);

        nums->base_ref = array;
        nums->size = argc - 1;
    }
}


/**
This function sorts a list of numbers in O(n^2) time using Bubble Sort

It accepts as arguments an integer pointer to the start of the list, and the
number of elements in the list
*/
void bubbleSortNumbers(int* start, int size){
    for (int end = size - 2; end > 0; end--){
       for (int i = 0; i <= end; i++){
            if (*(start + i) > *(start + i + 1)){       // if currentPosition > nextPosition
                // Swap values
                int temp = *(start + i + 1);        // temp store next
                *(start + i + 1) = *(start + i);
                *(start + i) = temp;
            }
        }
    }

}



/**
    Given an IntList, this function prints out the highest/max value.

    NOTE: this function assumes the IntList passed is sorted
*/
void* getSortedListMaxValue(void* list){
    IntList *num_list = (IntList *)list;
    int max_value = *(num_list->base_ref + (num_list->size - 1));

    printf("The maximum value is %i \n\n", max_value);
    return NULL;
}


/**
    Given an IntList, this function prints out the least/min value.

    NOTE: this function assumes the IntList passed is sorted
*/
void* getSortedListMinValue(void* list){
    IntList *num_list = (IntList *)list;
    int min_value = *num_list->base_ref;

    printf("The minimum value is %i \n\n", min_value);
    return  NULL;
}


/**
    Given an IntList, this function prints out the median value.

    NOTE: this function assumes the IntList passed is sorted
*/
void* getSortedListMedianValue(void* list){
    IntList *num_list = (IntList *)list;
    int mid = num_list->size / 2;
    int mid_value = *(num_list->base_ref + mid);

    printf("The median value is %i \n\n", mid_value);
    return NULL;
}



/**
Given a pointer to an IntList, this function will print out its average value
*/
void* getListAverageValue(void* list){
    IntList *num_list = (IntList *)list;
    long sum = 0;

    for (int i = 0; i < num_list->size; i++){
        sum += *(num_list->base_ref + i);
    }

    float* avg = malloc(sizeof(float));         // Memory Leak her [TO DO]
    *avg = sum / num_list->size;
    printf("The average value is %f \n\n", *avg);
    return (void *) avg;
}


/**
Given a pointer to an IntList, this function will print out its standard deviation

Note: This function is based off of the formula for find standard deviation of set of numbers
*/
void* getListStandardDeviation(void* list){
    IntList *num_list = (IntList *) list;

    float mean = ( *(float *) getListAverageValue(num_list) );

    float summation;

    for (int i = 0; i < num_list->size; i++){
        summation += pow(*(num_list->base_ref + i) - mean, 2);
    }
    float std = sqrt(summation / num_list->size);

    printf("The standard deviation is %f \n\n", std);
}



/**
    This function is largely diagnostic during coding. It helps print out the content of a
    list at any given time.
*/
void printListValues(int* start, int size){
    for (int i = 0; i < size; i++){
        printf("%i \t", *(start + i));
    }
    printf("\n");
}

