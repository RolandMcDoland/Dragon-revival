#include <mpi.h>
#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>
#include <string.h>
#include <time.h>

// Seperate the professions between available processes
int * setup(size)
{
    // Array containing max tid of processes in each profession
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

    // Add previous elements to make the element max tid 
    prof_array[1] = prof_array[1] + prof_array[0];
    prof_array[2] = prof_array[2] + prof_array[1];
 
    return prof_array;
}

int maxClock(int a, int b)
{
	if(a>b) return a;
	else return b;
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
		
		int clock = 1;
		int *packet;
		packet = malloc(2*sizeof(int));
		
        int *profession_array;
        profession_array = malloc(3 * sizeof(int));

        /* Pass size - 1 as available processes since 
        root is exclusively devoted to generating contracts */
        profession_array = setup(size - 2);

        // Notify all processes about the profession assignment
        for (int i = 1; i < size - 1; i++)
        {
            MPI_Send(profession_array, 3, MPI_INT, i, 0, MPI_COMM_WORLD );
            printf("Sending profession array\n");
        }

        // A counter generating new contract number every time
        int contract_counter = 1;

        int seconds;
        srand(time(NULL));

        // Main loop- generate a contract at random
        while(1)
        {
            // Wait random amount of time
            seconds = rand() % 9 + 1;
            sleep(seconds);
			
			packet[0] = contract_counter;

            printf("Sending new contract (%d)\n", clock);
            // Notify all processes about the profession assignment
            for (int i = 1; i <size - 1; i++)
            {
				clock++;
				packet[1] = clock;
				MPI_Send(packet, 2, MPI_INT, i, 1, MPI_COMM_WORLD );
                //MPI_Send(&contract_counter, 1, MPI_INT, i, 1, MPI_COMM_WORLD );
            }

            contract_counter++;
        }
    }
    else if(tid == size - 1)
    {
		int clock = 0;
		int *packet;
		packet = malloc(2*sizeof(int));
		
        int seconds;
        srand(time(NULL));

        // Main loop- kill a dragon at random
        while(1)
        {
            // Wait random amount of time
            seconds = rand() % 9 + 1;
            sleep(seconds);
			
			packet[0] = seconds;

            // Notify all processes about the death of dragon
            for (int i = 1; i <size - 1; i++)
            {
				clock++;
				packet[1] = clock;
                //MPI_Send(&seconds, 1, MPI_INT, i, 9, MPI_COMM_WORLD );
				MPI_Send(packet, 2, MPI_INT, i, 9, MPI_COMM_WORLD );
            }
        }
    }
    else
    {
        int *profession_array;
        profession_array = malloc(3 * sizeof(int));

        // Array containing numbers of not yet done contracts 
        int active_contracts[100] = {};

        // Array containing tids of cooperating experts from other professions
        int associates[2] = {};

        // 0- head expert, 1- torso expert, 2- tail expert
        int profession;

        int min_tid, profession_count, received, accept_counter, is_accepted, restricted_contract;

        int contract_number = -1;
        int busy = 0;
        int working = 0;
        int waiting_for_team = 0;
        int max_contract_index = 0;
        int trying_get_resource = 0;
        int using_resource = 0;
        int desks = 1;
        int skeletons = 15;
        int revive_counter = 0;
        int associates_wait = 0;
        int waiting_for_dragon = 0;

        int *priority_queue;
        priority_queue = malloc((size - 1) * sizeof(int));
		
		int clock = 2;
		int *packet;
		packet = malloc(2*sizeof(int));

        // Receive the profession array to know every process profession
        MPI_Recv(profession_array, 3, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Check in array your own profession
        if(tid <= profession_array[0])
        {
            profession = 0;
            min_tid = 1;
            profession_count = profession_array[0];
        }
        else if (tid <= profession_array[1])
        {
            profession = 1;
            min_tid = profession_array[0] + 1;
            profession_count = profession_array[1] - profession_array[0];
        }
        else
        {
            profession = 2;
            min_tid = profession_array[1] + 1;
            profession_count = profession_array[2] - profession_array[1];
        }
        printf("Profession number: %d, tid: %d\n", profession, tid);

        // Initialise the priority queue 
        for(int i = min_tid ;i <= profession_array[profession];i++)
        {
            priority_queue[i] = i - min_tid;
        }

        // Only receive for testing purposes for now
        while(1)
        {
			MPI_Recv(packet, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			received = packet[0];
			clock = maxClock(clock, packet[1]);
			clock++;

            // Depending on the tag process appropriatly
            switch(status.MPI_TAG)
            {
                // When it's a new contract
                case 1:
                    if(received == restricted_contract)
                    {
                        break;
                    }

                    if(contract_number == -1 && !busy)
                    {
                        contract_number = received;
                    }

                    // Put the new contract in the first free space in active contracts array
                    for(int i = 0; i < 100; i++)
                    {
                        if(active_contracts[i] == 0)
                        {
                            active_contracts[i] = received;
                            if(max_contract_index < i)
                            {
                                max_contract_index = i;
                            }
                            break;
                        }
                    }

                    // If the process is not busy send a request to everyone of your profession
                    if(!busy)
                    {
                        busy = 1;
                        accept_counter = 0;
                        associates_wait = 0;

                        for(int i = min_tid;i <= profession_array[profession];i++)
                        {
                            if(i != tid)
                            {
								clock++;
								packet[0]=contract_number;
								packet[1] = clock;
								MPI_Send(packet, 2, MPI_INT, i, 2, MPI_COMM_WORLD );
                            }
                        }
                    }

                    break;

                // When it's a new contract request
                case 2:
                    // If the process isn't requesting the same contract send an accept
                    if(contract_number != received)
                    {
                        is_accepted = 1;
                        restricted_contract = received;
                    }
                    else
                    {
                        // Check in priority queue which process is higher
                        if(priority_queue[status.MPI_SOURCE] < priority_queue[tid] && !working)
                        {
                            is_accepted = 1;
                        }
                        else
                        {
                            is_accepted = 0;
                        }
                    }
					
					clock++;
					packet[0] = is_accepted;
					packet[1] = clock;
					MPI_Send(packet, 2, MPI_INT, status.MPI_SOURCE, 3, MPI_COMM_WORLD );

                    break;

                // When it's an accept message
                case 3:
                    // If process gets an accept increase the counter
                    if(received && contract_number != -1 && busy)
                    {
                        accept_counter++;
                    }

                    // If process gets accepts from all other professionals
                    if(profession_count - 1 == accept_counter)
                    {
                        printf("%d: Taking care of contract %d (%d)\n", tid, contract_number, clock);

                        // Send an information about doing the contract to all processes
                        for(int i = 1;i < size - 1;i++)
                        {
                            if(i != tid)
                            {
								clock++;
								packet[0] = contract_number;
								packet[1] = clock;
								MPI_Send(packet, 2, MPI_INT, i, 4, MPI_COMM_WORLD );
                            }
                        }
                        
                        // Put the process at the end of priority queue
                        for(int i = min_tid ;i <= profession_array[profession];i++)
                        {
                            if(priority_queue[tid] < priority_queue[i])
                            {
                                priority_queue[i] -= 1;
                            }
                        }
                        priority_queue[tid] = profession_count - 1;

                        // Remove contract from active contracts waiting for completion
                        for(int i = 0;i <= max_contract_index;i++)
                        {
                            if(active_contracts[i] == contract_number)
                            {
                                active_contracts[i] = 0;
                                break;
                            }
                        }

                        waiting_for_team = 1;
                        working = 1;

                        if(associates[0] != 0 && associates[1] != 0)
                        {
                            // When process knows both associates let them know the team is ready
                            for(int i = 0;i < 2;i++)
                            {
								clock++;
								packet[0] = contract_number;
								packet[1] = clock;
								MPI_Send(packet, 2, MPI_INT, associates[i], 5, MPI_COMM_WORLD );
                            }
							clock++;
							packet[0] = contract_number;
							packet[1] = clock;
							MPI_Send(packet, 2, MPI_INT, tid, 5, MPI_COMM_WORLD );
                        }
                    }

                    break;

                // When it's a message about a process taking up a contract
                case 4:
                    // If the process is in the same profession
                    if(status.MPI_SOURCE >= min_tid && status.MPI_SOURCE <= profession_array[profession])
                    { 
                        // Remove contract from active contracts waiting for completion
                        for(int i = 0;i <= max_contract_index;i++)
                        {
                            if(active_contracts[i] == received)
                            {
                                active_contracts[i] = 0;
                            }
                        }

                        /* If some process takes contract stop requesting it 
                            and get first undone contract */
                        if(received == contract_number && busy)
                        {
                            if(!working)
                            {
                                // Remove potential associates
                                associates[0] = 0;
                                associates[1] = 0;
                            }

                            accept_counter = 0;

                            for(int i = 0;i <= max_contract_index;i++)
                            {
                                if(active_contracts[i] != 0)
                                {
                                    contract_number = active_contracts[i];
                                    break;
                                }
                            }
                            
                            // If there were no contracts waiting 
                            if(received == contract_number)
                            {
                                contract_number = -1;
                            }
                            busy = 0;
                        }

                        // Put the process at the end of priority queue
                        for(int i = min_tid ;i <= profession_array[profession];i++)
                        {
                            if(priority_queue[status.MPI_SOURCE] < priority_queue[i])
                            {
                                priority_queue[i] -= 1;
                            }
                        }
                        priority_queue[status.MPI_SOURCE] = profession_count - 1;
                    }
                    else
                    {
                        // If the expert of a different profession takes up the same contract
                        if(received == contract_number)
                        {
                            if(associates[0] == 0)
                            {
                                associates[0] = status.MPI_SOURCE;
                            }
                            else
                            {
                                associates[1] = status.MPI_SOURCE;
                                
                                if(waiting_for_team)
                                {
                                    // When process knows both associates let them know the team is ready
                                    for(int i = 0;i < 2;i++)
                                    {
										clock++;
										packet[0] = contract_number;
										packet[1] = clock;
										MPI_Send(packet, 2, MPI_INT, associates[i], 5, MPI_COMM_WORLD );
                                    }
									clock++;
									packet[0] = contract_number;
									packet[1] = clock;
									MPI_Send(packet, 2, MPI_INT, tid, 5, MPI_COMM_WORLD );
                                }
                            }
                        }
                    }
                    

                    break;

                // When it's a message about the whole team of professionals being ready
                case 5:
                    associates_wait++;

                    if(associates_wait == 3)
                    {
                        associates_wait = 0;
                        waiting_for_team = 0;

                        // Send request for desk and skeleton
                        if(profession != 2)
                        {
                            accept_counter = 0;
                            trying_get_resource = 1;

                            for(int i = min_tid;i <= profession_array[profession];i++)
                            {
                                if(i != tid)
                                {
									clock++;
									packet[0] = contract_number;
									packet[1] = clock;
									MPI_Send(packet, 2, MPI_INT, i, 6, MPI_COMM_WORLD );
                                }
                            }
                        }
                    }

                    break;
                // When it's a request for resource
                case 6:
                    // If the process isn't requesting the resource send an accept
                    if(!trying_get_resource && !using_resource)
                    {
                        is_accepted = 1;
                    }
                    else
                    {
                        // Check which process has an earlier contract
                        if(received < contract_number && !using_resource)
                        {
                            is_accepted = 1;
                        }
                        else
                        {
                            is_accepted = 0;
                        }
                    }
					clock++;
					packet[0] = is_accepted;
					packet[1] = clock;
					MPI_Send(packet, 2, MPI_INT, status.MPI_SOURCE, 7, MPI_COMM_WORLD );
                    break;

                // When it's a response for resource request
                case 7:
                    if(trying_get_resource)
                    {
                        // If process gets an accept increase the counter
                        if(received)
                        {
                            accept_counter++;
                        }

                        // If process gets enough accepts for their resource start using it
                        // Head professional write the report on the desk
                        if(profession == 0)
                        {
                            if(accept_counter >= profession_count - desks)
                            {
                                trying_get_resource = 0;
                                using_resource = 1;
                                printf("%d: Making report (%d)\n", tid, clock);
                            }
                        }
                        // Torso professional get the skeleton
                        if(profession == 1)
                        {
                            if(accept_counter >= profession_count - skeletons)
                            {
                                trying_get_resource = 0;
                                using_resource = 1;
                                printf("%d: Getting skeleton (%d)\n", tid, clock);
                            }
                            else if(accept_counter == profession_count - 1)
                            {
                                waiting_for_dragon = 1;
                            }
                        }
                    }

                    // If the resource is used
                    if(using_resource)
                    {
                        // When process is done with resource let the associates know
                        for(int i = 0;i < 2;i++)
                        {
							clock++;
							packet[0] = contract_number;
							packet[1] = clock;
							MPI_Send(packet, 2, MPI_INT, associates[i], 8, MPI_COMM_WORLD );
                        }
						clock++;
						packet[0] = contract_number;
						packet[1] = clock;
						MPI_Send(packet, 2, MPI_INT, tid, 8, MPI_COMM_WORLD );

                        skeletons--;

                        // Notify all torso experts about decresed skeletons
                        for(int i = min_tid;i <= profession_array[profession];i++)
                        {
                            if(i != tid)
                            {
								clock++;
								packet[0] = is_accepted;
								packet[1] = clock;									
                                MPI_Send(packet, 2, MPI_INT, i, 10, MPI_COMM_WORLD );
                            }
                        }

                        is_accepted = 1;
                        // Send accepts to everyone waiting for resource 
                        for(int i = min_tid;i <= profession_array[profession];i++)
                        {
                            if(i != tid)
                            {
								clock++;
								packet[0] = is_accepted;
								packet[1] = clock;
								MPI_Send(packet, 2, MPI_INT, i, 7, MPI_COMM_WORLD );
                            }
                        }

                        using_resource = 0;
                    }

                    break;

                // When it's a message about being ready to revive
                case 8:
                    revive_counter++;

                    /* When you get two messages (both professionals are done with resources)
                        revive your part of the dragon */
                    if(revive_counter == 2)
                    {
                        if(profession == 0)
                        {
                            printf("%d: Reviving head (%d)\n", tid, clock);
                        }
                        if(profession == 1)
                        {
                            printf("%d: Reviving torso (%d)\n", tid, clock);
                        }
                        if(profession == 2)
                        {
                            printf("%d: Reviving tail (%d)\n", tid, clock);
                        }

                        // Reset all variables
                        revive_counter = 0;
                        accept_counter = 0;
                        working = 0;
                        associates[0] = 0;
                        associates[1] = 0;

                        for(int i = 0;i <= max_contract_index;i++)
                        {
                            if(active_contracts[i] != 0)
                            {
                                contract_number = active_contracts[i];
                                break;
                            }                            
                        }
                            
                        // If there were no contracts waiting 
                        if(received == contract_number)
                        {
                            contract_number = -1;
                        }
                        busy = 0;
                    }
                    break;

                case 9:
                    skeletons++;

                    if(waiting_for_dragon)
                    {
						clock++;
						packet[0] = 1;
						packet[1] = clock;
						MPI_Send(packet, 2, MPI_INT, tid, 7, MPI_COMM_WORLD );

                        waiting_for_dragon = 0;
                    }

                    break;

                case 10:
                    skeletons--;

                    break;  

            }
        }
    }

    MPI_Finalize();
}