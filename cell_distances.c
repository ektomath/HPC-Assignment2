#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s -t<integer>\n", argv[0]);
        return 1;
    }
    int threads = atoi(argv[1] + 2);
    omp_set_num_threads(threads);

    /* PART 1 READING THE FILE AND SAVING THE DATA */
    FILE *file = fopen("cells", "r");
    if (file == NULL) {
        perror("Error opening file to read");
        return 1;
    }

    fseek(file, 0, SEEK_END); // moves filepointer to end to calculate number of rows in file
    //printf("pointer position %ld\n", ftell(file));
    int bytesToRead = ftell(file);
    int rows = bytesToRead / 24;
    fseek(file, 0, SEEK_SET); // resets file pointer
    //printf("nr rows %ld\n", rows);

    float **matrix = (float **)malloc(rows * sizeof(float *));
#pragma omp for
    for (int i = 0; i < rows; i++) {
        matrix[i] = (float *)malloc(3 * sizeof(float));
    }

    float x, y ,z;
#pragma omp for
    for (int row = 0; row < rows; row++) {
        fscanf(file, "%f %f %f", &x, &y, &z);
        matrix[row][0] = x;
        matrix[row][1] = y;
        matrix[row][2] = z;
    }
    fclose(file);

    /*
    printf("Matrix elements:\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%.3f ", matrix[i][j]);
        }
        printf("\n"); // Move to the next row
    }*/

    //PART 2 COMPUTING THE DISTANCE
    int distanceVector[3466];
#pragma omp for
    for (int i = 0; i <= 3465; i++) { // initializes vector to 0
        distanceVector[i] = 0;
    }

    #pragma omp parallel
    {
    #pragma omp for reduction(+:distanceVector[:3465]) // runs out for loop in parallel but not inner
    for (int fromCell = 0; fromCell < rows - 1; fromCell++) {
        //printf("fromRow: %d\n", fromCell);
        for (int toCell = fromCell + 1; toCell < rows; toCell++) {
            //printf("toCell: %d\n", toCell);
            float diff[3];
            //int sumOfSquares;
            diff[0] = matrix[fromCell][0] - matrix[toCell][0];
            diff[1] = matrix[fromCell][1] - matrix[toCell][1];
            diff[2] = matrix[fromCell][2] - matrix[toCell][2];
            /*int sumOfSquares = (diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);

            if (((short) (sqrtf( sumOfSquares) /10 + 0.5)) == 0) {
                printf("fromcell: %d, tocell: %d\n", fromCell, toCell);
                printf("sumofsquares %d\n", sumOfSquares);
                printf("sqrt_sumofsquares %d\n", (short) (sqrtf( sumOfSquares) / 10));
                printf("fromCell coordinates: %d, %d, %d\n", matrix[fromCell][0], matrix[fromCell][1], matrix[fromCell][2]);
                printf("toCell coordinates: %d, %d, %d\n", matrix[toCell][0], matrix[toCell][1], matrix[toCell][2]);
                printf("diff[0] %d, diff[1] %d, diff[2] %d,\n", diff[0], diff[1], diff[2]);
            }*/
            //printf("index in distVector: %d\n", (int) roundf(sqrtf((float) sumOfSquares) / 10.0));
            //distanceVector[(short) (sqrtf((float) sumOfSquares) / 10.0)] += 1;
            distanceVector[(short) (sqrtf( (diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2])) * 100 + 0.5)] += 1;
        }
    }
    }


    // print results
    for (int i = 0; i < 1000; i++) {
        if (distanceVector[i] != 0) {
            printf("0%.2f %d\n", i / 100.0, distanceVector[i]);
        }
    }
    for (int i = 1000; i <= 3465; i++) {
        if (distanceVector[i] != 0) {
            printf("%.2f %d\n", i / 100.0, distanceVector[i]);
        }
    }

    // free memory
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);

    return 0;
}