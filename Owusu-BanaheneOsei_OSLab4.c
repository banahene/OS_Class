#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>



/*******************************************************************************************************************
                                        Operating Systems 2020 Lab Assignment 4
                                            Owusu-Banahene K. Osei (18122021)
*******************************************************************************************************************/



/**
    My definition of a Matrix struct
*/
typedef struct {
     int *base_ref;
     int row_count;
     int column_count;
} Matrix;


/**
    Struct representing unit of threaded multiplication
*/
typedef struct {
    Matrix* mat1;
    Matrix* mat2;
    Matrix* solution;
    int row_start;
    int row_end;
} Threaded_Matrix_Multiplication_Representation;


/**
    This function will accept a pointer to a Matrix and return the value at given [row][column]
    TO DO       Manage NULL pointer and possible illegal arguments for 'row' and 'column'
*/
int get(Matrix *matrix, int row, int column){
    int index = row * matrix->column_count + column;
    return *(matrix->base_ref + index);
}


// DECLARE FUNCTIONS
Matrix* readMatrix(FILE* sourceFile);
//Matrix* multiplyMatrices(Matrix *matrix1,  Matrix *matrix2);
Matrix* multiplyMatrices(Matrix *matrix1,  Matrix *matrix2, Matrix *output_matrix);
int writeMatrixToFile(Matrix *matrix, char* fileName);
Matrix* parseCharArrayAsMatrix(char *array);
Matrix* sequencialMultiplyMatrices(Matrix *matrix1,  Matrix *matrix2, Matrix *output_matrix);
Matrix* parallelMultiplyMatrices(Matrix *matrix1,  Matrix *matrix2, Matrix *output_matrix);
void* partialMatrixMultiply(void* multiply_data);


/**
    Program entry point
*/
int main(int argc, char *argv[]){

    // Check to make sure we have correct number of arguments
    if (argc != 3){
        printf("You entered the wrong number of arguments. Include the filenames of both matrices on command line");
        return 1;           // Return 1 as error code
    }

    // Open both files
    FILE* file1 = fopen(argv[1], "r");
    FILE* file2 = fopen(argv[2], "r");

    if (file1 == NULL || file2 == NULL)
        return 2;           // Error code for files not opening

    Matrix *matrix1 = readMatrix(file1);    // Pass file to readMatrix() and dereference returned matrix pointer
    Matrix *matrix2 = readMatrix(file2);     // Pass file to readMatrix()

    if (matrix1 == NULL || matrix2 == NULL)
        return 3;                           // Error code indication matrix computation failure

    // MULTIPLY MATRICES
    Matrix* solution = malloc(sizeof(Matrix));                  // First allocate memory for matrix
    // Create array pointers for the two matrices
    solution->base_ref = malloc(matrix1->row_count * matrix2->column_count * sizeof(int));
    solution->row_count = matrix1->row_count;     solution->column_count = matrix2->column_count;

    int successful = (multiplyMatrices(matrix1, matrix2, solution) )!= NULL ? 0 : 1;      // O for success, 1 for failure

    // MAKE SURE SOLUTION IS NOT AN ERROR
    if (successful == 0)
        writeMatrixToFile(solution, "matrixC.txt");

    //return 3;           // third error code for failure




    // FREE ALL ALLOCATED MEMORY (matrix1, matrix2, solution)               TO DO           (2D_ARRAYs may still leak. Refactor)
    free(matrix1->base_ref); free(matrix2->base_ref); free(solution->base_ref); free(matrix1); free(matrix2); free(solution);

    // Close Opened Files
    fclose(file1); fclose(file2);

    return (successful == 0) ? 0 : 3;           // Program executed successfully
}



/**
    Given a file pointer, this method will parse it and return a matrix
*/
Matrix* readMatrix(FILE* sourceFile){
    if (sourceFile == NULL)
        return NULL;

    // Create buffer to hopefully read all of file in one go
    const int bufferSize = 500;
    char *buffer = malloc(bufferSize * sizeof(char));

    if (buffer == NULL)
        return NULL;        // Make sure there was enough memory to malloc

    int len = fread(buffer, sizeof(char), bufferSize, sourceFile);    // Hopefully read all at a go
    buffer[len] = '\0';              // Add string terminator as sentinel

    Matrix *matrix = parseCharArrayAsMatrix(buffer);

    free(buffer);       // Free heap allocated memory
    return matrix;
}


/**
    Given a "string" this method is able to parse it into a matrix
*/
Matrix* parseCharArrayAsMatrix(char *array){
    char *runner = array;
    int spaceCount = 0;
    int lineCount = 0;

    while(*runner != '\0' && *runner != EOF){       // Count spaces and newlines
        if (*runner == ' ')
            spaceCount++;
        else if(*runner == '\n')
            lineCount++;

        runner += 1;   // move pointer to next character
    }

    int rows = *array == '\0' ? 0 : lineCount + 1;              //check for empty file
    int columns =rows == 0 ? 0 : (spaceCount / rows) + 1;

    if (rows == 0 || columns == 0)      // Check for empty file
        return NULL;

    int (*matrix)[rows] = malloc(rows * columns * sizeof(int));       // heap 2D array


    //Split at spaces
    char delimiters[] = " \n";
    int count = 0;
    char *token = strtok(array, delimiters);  // Get first token
    while(token != NULL){
        *((*matrix) + count) = atoi(token);
        token = strtok(NULL, delimiters);        // get next
        count += 1;
    }

    Matrix *return_matrix = malloc(sizeof(Matrix));
    return_matrix->row_count = rows;
    return_matrix->column_count = columns;
    return_matrix->base_ref = *matrix;
    return return_matrix;
}



/**
    Given two matrices, this method simply multiplies them
*/
Matrix* multiplyMatrices(Matrix *matrix1,  Matrix *matrix2, Matrix *output_matrix){

    if (matrix1 == NULL || matrix2 == NULL || (matrix1->column_count != matrix2->row_count) ){
        printf("Invalid matrix multiplication! Dimensions likely don't support operation");
        return NULL;
    }

    if (matrix1->row_count <= 4)
        return sequencialMultiplyMatrices(matrix1, matrix2, output_matrix);
    else
        return parallelMultiplyMatrices(matrix1, matrix2, output_matrix);


    printf("Multiplied\n\n");

}




/**

	This function will execute a matrix multiplication using multiple threads of execution

*/
Matrix* parallelMultiplyMatrices(Matrix *matrix1,  Matrix *matrix2, Matrix *output_matrix){
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    pthread_t thread4;

    Threaded_Matrix_Multiplication_Representation* rep1 = malloc(sizeof(Threaded_Matrix_Multiplication_Representation));
    rep1->mat1 = matrix1; rep1->mat2 = matrix2; rep1->solution = output_matrix;
    rep1->row_start = 0; rep1->row_end = matrix1->row_count/4 - 1;
    printf("\tFrom %i to %i",  0, matrix1->row_count/4 - 1);


    Threaded_Matrix_Multiplication_Representation* rep2 = malloc(sizeof(Threaded_Matrix_Multiplication_Representation));
    rep2->mat1 = matrix1; rep2->mat2 = matrix2; rep2->solution = output_matrix;
    rep2->row_start = matrix1->row_count/4; rep2->row_end = matrix1->row_count/2 - 1;

    Threaded_Matrix_Multiplication_Representation* rep3 = malloc(sizeof(Threaded_Matrix_Multiplication_Representation));
    rep3->mat1 = matrix1; rep3->mat2 = matrix2; rep3->solution = output_matrix;
    rep3->row_start = matrix1->row_count/2; rep3->row_end = matrix1->row_count - matrix1->row_count/4 - 1;

    Threaded_Matrix_Multiplication_Representation* rep4 = malloc(sizeof(Threaded_Matrix_Multiplication_Representation));
    rep4->mat1 = matrix1; rep4->mat2 = matrix2; rep4->solution = output_matrix;
    rep4->row_start = matrix1->row_count - matrix1->row_count/4; rep4->row_end = matrix1->row_count - 1;

    // Spawn threads to do their jobs
    pthread_create(&thread1, NULL, partialMatrixMultiply, rep1);
    pthread_create(&thread2, NULL, partialMatrixMultiply, rep2);
    pthread_create(&thread2, NULL, partialMatrixMultiply, rep3);
    pthread_create(&thread4, NULL, partialMatrixMultiply, rep4);

	//Join threads to avoid function returning when the multiplication is not complete
    pthread_join(thread1, NULL); pthread_join(thread2, NULL); pthread_join(thread3, NULL); pthread_join(thread4, NULL);
    return output_matrix;
}



/**
    This is a partial multiplication of a matrix that can run in parallel
*/
void* partialMatrixMultiply(void* multiply_data){
    Threaded_Matrix_Multiplication_Representation* data = (Threaded_Matrix_Multiplication_Representation*) multiply_data;

    // Do multiplication
    int sum = 0;
    for (int i = data->row_start; i <= data->row_end; i++){
        for (int j = 0; j < data->mat2->column_count; j++){
            for (int k = 0; k < data->mat2->row_count; k++){
                sum += get(data->mat1, i, k) * get(data->mat2, k, j);
            }
            *((i * data->solution->column_count + j) + data->solution->base_ref) = sum;
            sum = 0;            // reset dot product sum
        }
    }
}



/**
	This function does a tradition matrix multiplication (i.e. uses a single thread of execution)
*/
Matrix* sequencialMultiplyMatrices(Matrix *matrix1,  Matrix *matrix2, Matrix *output_matrix){
    // Do multiplication
    int sum = 0;
    printf("Row count: %i\n\n", matrix1->row_count);
    for (int i = 0; i < matrix1->row_count; i++){
        for (int j = 0; j < matrix2->column_count; j++){
            for (int k = 0; k < matrix2->row_count; k++){
                sum += get(matrix1, i, k) * get(matrix2, k, j);
            }

            *((i * output_matrix->column_count + j) + output_matrix->base_ref) = sum;
            sum = 0;            // reset dot product sum
        }
    }

    return output_matrix;
}




/**
    Given a matrix and a filename, this method will write the matrix to that file
*/
int writeMatrixToFile(Matrix *matrix, char* fileName){
    FILE* file = fopen(fileName, "w");
    int row_count = matrix->row_count;
    int col_count = matrix->column_count;

    for (int index = 0; index < row_count * col_count; index++){
        if (index != 0 && index % col_count == 0)
            fputc('\n', file);      // move to next line
        fprintf(file, "%d ", *(matrix->base_ref + index));
    }

    return 0;
}
