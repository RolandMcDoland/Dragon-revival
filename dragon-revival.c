#include <mpi.h>
#include <stdlib.h>
#include <stdio.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Seperate the professions between available processes
int * setup(size)
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

    MPI_Init(&argc, &argv);

    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &tid );

    if(tid == 0)
    {
        int *profession_array;
        profession_array = malloc(3 * sizeof(int));

        /* Pass size - 1 as available processes since 
        root is exclusively devoted to generating contracts */
        profession_array = setup(size - 1);

        // Notify all processes about the profession assignment
        for (int i = 1; i < size; i++)
        {
            MPI_Ssend(profession_array, 3, MPI_INT, i, 0, MPI_COMM_WORLD );
            printf("Sending profession array\n");
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
                MPI_Ssend(&contract_counter, 1, MPI_INT, i, 1, MPI_COMM_WORLD );
                printf("Sending new contract\n");
            }

            contract_counter++;
        }
    }
    else
    {
        int *profession_array;
        profession_array = malloc(3 * sizeof(int));

        // Array containing numbers of not yet done contracts 
        int active_contracts[100];

        // 0- head expert, 1- torso expert, 2- tail expert
        int profession;

        // Receive the profession array to know every process profession
        MPI_Recv(profession_array, 3, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Check in array your own profession
        if(tid <= profession_array[0])
        {
            profession = 0;
        }
        else if (tid <= profession_array[0] + profession_array[1])
        {
            profession = 1;
        }
        else
        {
            profession = 2;
        }
        printf("Profession number: %d, tid: %d\n", profession, tid);

        int *contract_number;

        // Only receive for testing purposes for now
        while(1)
        {
            MPI_Recv(&contract_number, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);        
            printf("New contract received by %d\n", tid);
        }
    }

    MPI_Finalize();
}