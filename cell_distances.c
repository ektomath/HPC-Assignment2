#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>



void calcWithinBlockCellDistances(float** matrix, int rows, int distanceVector[3465]) {
#pragma omp parallel
    {
#pragma omp for reduction(+:distanceVector[:3465]) // runs out for loop in parallel but not inner
        for (int fromCell = 0; fromCell < rows - 1; fromCell++) {
            for (int toCell = fromCell + 1; toCell < rows; toCell++) {
                float diff[3];
                diff[0] = matrix[fromCell][0] - matrix[toCell][0];
                diff[1] = matrix[fromCell][1] - matrix[toCell][1];
                diff[2] = matrix[fromCell][2] - matrix[toCell][2];
                distanceVector[(short) (sqrtf( (diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2])) * 100 + 0.5)] += 1;
            }
        }
    }
}

void calcBetweenBlockCellDistances(float** matrixA, float** matrixB, int rowsA, int rowsB, int distanceVector[3465]) {
    #pragma omp parallel
    {
    #pragma omp for reduction(+:distanceVector[:3465]) // runs out for loop in parallel but not inner
        for (int fromCell = 0; fromCell < rowsA; fromCell++) {
            for (int toCell = 0; toCell < rowsB; toCell++) {
                float diff[3];
                diff[0] = matrixA[fromCell][0] - matrixB[toCell][0];
                diff[1] = matrixA[fromCell][1] - matrixB[toCell][1];
                diff[2] = matrixA[fromCell][2] - matrixB[toCell][2];
                distanceVector[(short) (sqrtf( (diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2])) * 100 + 0.5)] += 1;
            }
        }
    }
}

int min(int a, int b)
{
    if(a>b)
        return b;
    else
        return a;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s -t<integer>\n", argv[0]);
        return 1;
    }
    int threads = atoi(argv[1] + 2);
    omp_set_num_threads(threads);


    // initializing distance vector
    int distanceVector[3466];
    #pragma omp for
    for (int i = 0; i <= 3465; i++) {
        distanceVector[i] = 0;
    }


    //reading file and calculating size and rows of file
    FILE *file = fopen("cells", "r");
    if (file == NULL) {
        perror("Error opening file to read");
        return 1;
    }
    fseek(file, 0, SEEK_END); // moves filepointer to end to calculate number of rows in file
    int bytesToRead = ftell(file);
    int rowsInFile = bytesToRead / 24;
    fseek(file, 0, SEEK_SET); // resets file pointer


    /* SETS SIZE OF BLOCKS AND LINES IN BLOCK, (maybe worth optimizing for number of threads?)  */
    int maxLines = 100000; // (4000000 / 24) roughly what we get if we divide 4mb with size of 3 floats times 2 blocks
    maxLines = maxLines / omp_get_num_threads(); // divide by number of threads so  each thread can work on a separate block
    int rowsInBlock = min(rowsInFile, maxLines); // no need for more lines in block than there is lines in the file
    int nrBlocks = rowsInBlock / rowsInFile; // what if last block has less rows? TODO, fix it



    // MAIN LOOP
//#pragma omp for
    for (int block_A = 0; block_A < nrBlocks; block_A++) {
        fseek(file, 24*block_A*rowsInBlock, SEEK_SET);


        float **matrix_A = (float **)malloc(rowsInBlock * sizeof(float *));
        #pragma omp for
        for (int i = 0; i < rowsInBlock; i++) {
            matrix_A[i] = (float *)malloc(3 * sizeof(float));
        }

        // reads file and fills block A with values
        float Ax, Ay ,Az;
        #pragma omp for
        for (int rowA = 0; rowA < rowsInBlock; rowA++) {
            fscanf(file, "%f %f %f", &Ax, &Ay, &Az);
            matrix_A[rowA][0] = Ax;
            matrix_A[rowA][1] = Ay;
            matrix_A[rowA][2] = Az;
        }
        calcWithinBlockCellDistances(matrix_A, rowsInBlock, distanceVector);


        // Keep block A and iterate through all other blocks calculating between blocks cell distances
        for (int block_B = block_A + 1; block_B < nrBlocks; block_B++) {
            float **matrix_B = (float **)malloc(rowsInBlock * sizeof(float *));
            #pragma omp for
            for (int i = 0; i < rowsInBlock; i++) {
                matrix_B[i] = (float *)malloc(3 * sizeof(float));
            }

            // reads file and fills block B with values
            float Bx, By ,Bz;
            #pragma omp for
            for (int rowB = 0; rowB < rowsInBlock; rowB++) {
                fscanf(file, "%f %f %f", &Bx, &By, &Bz);
                matrix_B[rowB][0] = Bx;
                matrix_B[rowB][1] = By;
                matrix_B[rowB][2] = Bz;
            }
            calcBetweenBlockCellDistances(matrix_A, matrix_B, rowsInBlock, rowsInBlock, distanceVector);


            // free memory
            for (int i = 0; i < rowsInBlock; i++) {
                free(matrix_B[i]);
            }
            free(matrix_B);
        }


        // free memory
        for (int i = 0; i < rowsInBlock; i++) {
            free(matrix_A[i]);
        }
        free(matrix_A);
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

    return 0;
}