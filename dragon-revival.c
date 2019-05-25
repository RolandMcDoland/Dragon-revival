#include <mpi.h>
#include <stdlib.h>
#include <stdio.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

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
    MPI_Status status;

    int *profession_array;

    MPI_Init(&argc, &argv);

    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &tid );

    if(tid == 0)
    {
        /* Pass size - 1 as available processes since 
        root is exclusively devoted to generating contracts */
        profession_array = setup(size - 1);

        // Notify all processes about the profession assignment
        for (int i = 1; i < size; i++)
        {
            MPI_Ssend(&profession_array, 1, MPI_INT, i, 0, MPI_COMM_WORLD );
        }

        // A counter generating new contract number every time
        int contract_counter = 0;

        int seconds;
        srand(time(NULL));

        // Main loop- generate a contract at random
        while(1)
        {
            // Wait random amount of time
            seconds = rand() % 9 + 1;
            sleep(seconds);

            // Notify all processes about the profession assignment
            for (int i = 1; i < size; i++)
            {
                MPI_Ssend(&contract_counter, 1, MPI_INT, i, 0, MPI_COMM_WORLD );
            }

            contract_counter++;
        }
    }
    else
    {
        // Only receive for testing purposes for now
        while(1)
        {
            MPI_Recv(&profession_array, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            printf(" Otrzymalem od %d\n", status.MPI_SOURCE);
        }
    }

    MPI_Finalize();
}