#include <mpi.h>
#include <stdlib.h>
#include <stdio.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

// Seperate the professions between available processes
int *setup(size)
{
    // Array containing number of processes in each profession
    static int prof_array[3];

    int div = size / 3;
    int div_mod = size % 3;

    /* Every profession has size/3 processes, in case the size isn't divisible by 3 
    one or two first professions get additional process*/
    for(int i = 0; i < 3; i++)
    {
        prof_array[i] = div;

        // If the remainder is greater than zero add process and decrease
        if(div_mod > 0)
        {
            prof_array[i]++;

            div_mod--;
        }
    }

    return prof_array;
}

int main(int argc, char **argv)
{
    int size, tid;

    MPI_Init(&argc, &argv);

    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &tid );

    if(tid == 0)
    {
        /* Pass size - 1 as available processes since 
        root is exclusively devoted to generating contracts */
        int *profession_array = setup(size - 1);
    }

    MPI_Finalize();
}