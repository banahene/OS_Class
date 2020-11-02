#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
     int *base_ref;
     int row_count;
     int column_count;
} Matrix;

/*
    This function will accept a pointer to a Matrix and return the value at given [row][column]
    TO DO       Manage NULL pointer and possible illegal arguments for 'row' and 'column'
*/
int get(Matrix *matrix, int row, int column){
    int index = row * matrix->column_count + column;
    return *(matrix->base_ref + index);
}


// DECLARE METHODS
Matrix* readMatrix(FILE* sourceFile);
Matrix* multiplyMatrices(Matrix *matrix1,  Matrix *matrix2);
int writeMatrixToFile(Matrix *matrix, char* fileName);
Matrix* parseCharArrayAsMatrix(char *array);


/*
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

    // Multiply matrices
    Matrix *solution = multiplyMatrices(matrix1, matrix2);

    // MAKE SURE SOLUTION IS NOT AN ERROR
    if (solution == NULL)
        return 3;           // third error code for failure


    writeMatrixToFile(solution, "matrixC.txt");

    // FREE ALL ALLOCATED MEMORY (matrix1, matrix2, solution)               TO DO           (2D_ARRAYs may still leak. Refactor)
    free(matrix1->base_ref); free(matrix2->base_ref); free(solution->base_ref); free(matrix1); free(matrix2); free(solution);

    return 0;           // Program executed successfully
}



/*
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


/*
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



/*
    Given two matrices, this method simply multiplies them
*/
Matrix* multiplyMatrices(Matrix *matrix1,  Matrix *matrix2){

    if (matrix1 == NULL || matrix2 == NULL || (matrix1->column_count != matrix2->row_count) ){
        printf("Invalid matrix multiplication! Dimensions likely don't support operation");
        return NULL;
    }

    // Create array pointers for the two matrices
    int (*solution)[matrix1->row_count] =
                    malloc(matrix1->row_count * matrix2->column_count * sizeof(int));

    // Do multiplication
    int sum = 0;
    for (int i = 0; i < matrix1->row_count; i++){
        for (int j = 0; j < matrix2->column_count; j++){
            for (int k = 0; k < matrix2->row_count; k++){
                sum += get(matrix1, i, k) * get(matrix2, k, j);
            }

            *(*(solution + i) + j) = sum;       //solution[i][j] = sum
            sum = 0;            // reset dot product sum
        }
    }

    Matrix *return_matrix = malloc(sizeof(Matrix));
    return_matrix->row_count = matrix1->row_count;
    return_matrix->column_count = matrix2->column_count;
    return_matrix->base_ref = *solution;
    return return_matrix;
}


/*
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