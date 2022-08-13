#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>

//shared memory values
const int MEMORY_SIZE = 4096;
const int BUFFER_SIZE = 256;
	
//generate an error message
void errormsg( char *msg ) {
   perror( msg );
   exit( 1 );
}

int p = 1;
//binary semaphore process synchronization mechanism
struct binary_semaphore {
    pthread_mutex_t mutex;
    pthread_cond_t cvar;
    bool v;
};
void mysem_post(struct binary_semaphore *p)
{
    pthread_mutex_lock(&p->mutex);
    if (p->v)
        abort(); // error
    p->v = true;
    pthread_cond_signal(&p->cvar);
    pthread_mutex_unlock(&p->mutex);
}
void mysem_wait(struct binary_semaphore *p)
{
    pthread_mutex_lock(&p->mutex);
    while (!p->v)
        pthread_cond_wait(&p->cvar, &p->mutex);
    p->v = false;
    pthread_mutex_unlock(&p->mutex);
}

int main(int argc, char *argv[])
{
	// iterate for loops	
	int i = 0; 
	
	//if statement for arguments
	if (argc != 3)
	{
		printf("ERROR: 3 arguments required to operate.\n1. Input the filename of this program\n2. Input the filename of the input file\n3. Input the value for d\n");
		exit(1);
	}

	//work with the file from argument
	FILE *f = fopen(argv[1], "r");

	//accept the value d from argument
	int d = 0;
	sscanf (argv[2],"%d",&d);
	
	//count the number of inputs in the input file by # of lines
	int line = 1; 
	char c;
	do
	{
	c = fgetc(f);
	if(c == '\n') line++;
	} while(c != EOF);
	
	//the array is the size of the number of lines of input file
	int arr[line];
	
	//reset file pointer to 0
	fseek(f, 0, SEEK_SET);

	//accept user input for v, the value to search occurences of
	int v;
	printf("\nPlease Enter an integer value for v\n");
	scanf("%d", &v);
	int n = v;
	printf("v is %d\n\n", v);

	//read from file and put into array
	int l1 = 0;
	for (i = 0; i < line; i++)
	{
		fscanf(f, "%d", &arr[i]);
		if (arr[i] == n) l1++;
	}
	
	//output to user
	printf("\nThe Selected Value for d is: %d\n", d);
	
	//error if d is too large
	if(d > line) errormsg("ERROR: d number of child processes is greater than the number of inputs from inputfile\n");	 

	//output numbers from array to check if they are correct
	printf("All inputs: ");
	for (i = 0; i < line; i++)
	{
		printf("%d ", arr[i]);
	}
	printf("\n");

	//int segment_id;
	char buffer[BUFFER_SIZE];

	// create a memory segment to be shared
	int segment_id = shmget(IPC_PRIVATE, MEMORY_SIZE, S_IRUSR | S_IWUSR);

	if ( segment_id < 0 ) errormsg( "ERROR in creating a shared memory segment\n");

	fprintf( stdout, "Segment id = %d\n", segment_id);

	// attach an area of local memory to the shared memory segment
	char *shared_memory = (char*) shmat( segment_id, NULL, 0 );

	//create d number of child processes using fork 
	//store pid_t into array for later reference
	pid_t x[d];
	pid_t pid;
	
	for (i=0;i<d;i++){
		pid = fork();
if (pid < 0) errormsg( "\nERROR in creating a child process\n");
		if (pid <= 0) 
			return 1;
		else{
		    printf("\nchild pid %d from parent pid %d\n",getpid(),getppid());
		x[i] = getpid();
		}	
	}
	
	//create a 2d array to use in shared memory
	//d arrays of (size/d)+1 
	int arr2d[d][(line/d)+1];
	
	//put array into shared memory
	int j = 0;
	*(shared_memory + i) = arr2d[i][j];
		
	//processes work in shared memory
	//iterates through 2d array in nested for loop
	j = 0;
	int ipcCounter = l1;
	for(i = 0; i < d; i++){
		//binary semaphore increment
		p--;
		if(pid == x[i]){
			for(j = 0; j < ((line/d)+1); j++)
			{
				if (arr2d[i][j] == v)
					ipcCounter++;
			}
		//binary semaphore decrement
		p++;
		}  
	} 

	//output occurences to user
	printf("\n\nThe total number of v in the input file is: %d \n\n\n",ipcCounter);
	
	//user types quit to terminate process
	char q[4];
	char w[] = "quit";
	printf("Enter quit to terminate process:\n");
	scanf(" %c",&q);
	if (strcmp(q,w) == 0) return 0;
	
return 0;

}
