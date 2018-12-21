

#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h> 
#include <sys/mman.h>
#include <stdint.h>
#include <iomanip>

typedef struct {
	uint8_t pid;
	uint8_t page;
} MemoryAccess;

typedef struct {
	int frame;
	int pageHits;
	int pageMisses;
} PageTableEntry;

typedef struct{
	uint8_t pid;
	uint8_t page;
	bool vacant;
} FrameTableEntry;

//Initializing memeory access to 0
int memacess = 0;

//Initializing pagetablesize
int PAGETABLESIZE =  256;

//intialzing nprocesses
int NPROCESSES =  256;

//Initializing to read in data file
char file[20];

int main(int argc, char** argv)
{

	printf("Jakub \n");
	
	//If arguement is one, then display exit message
	if(argc == 1)
	{

		printf("Need more than 1 arguement to run program(Needs a picture file). Exiting...\n");

		exit(0);
	}

	//If arguement is 2 then strcopy the file into argv1 of 20 characters
	if(argc >= 2){

		strncpy(file,argv[1], 100);
		printf("Loading in File...\n");
	}

	if(argc >= 3){

		memacess = atoi(argv[2]);
		printf("MemoryAccess number updated\n");

	}
	if(argc >= 4){

		PAGETABLESIZE = atoi(argv[3]);

		if(PAGETABLESIZE == 0){
			PAGETABLESIZE = 256;

		}
		printf("PageTable size updated\n");


	}

	//Attmepting to open the file
	int filesuccess = open(file, O_RDWR);

	//If File doesn't open then it returns -1
	if(filesuccess == -1)
	{
		printf("File did not open successfully. Exiting...\n");
		exit(0);
	}

	/* fstat the file*/
	struct stat statsOfFile;

	if(fstat(filesuccess, &statsOfFile) < 0)
	{
		printf("fstat failed. Exiting...\n");

		exit(0);
	}


	/* mmap the file */
	int fileSize = statsOfFile.st_size;


	MemoryAccess *memtable;

	memtable = (MemoryAccess*)mmap(0, fileSize, PROT_READ, MAP_SHARED, filesuccess, 0);

	/* Check if we want to read the whole file */
	if(memacess == 0)
	{

		memacess = (int)(fileSize/ sizeof(MemoryAccess));

	}


	//Printing out different info
	printf("Input file: %s\n", file);

	//memory access stats
	printf("# of times Memory Accessed: %d\n",memacess);

	//FrameTimeTable size
	printf("Size Frametable: %d\n", PAGETABLESIZE);


	//Initializing pagetable
	PageTableEntry pageTable[256][256];

	
	//Going through the 2D array
	for(int a = 0; a < 256; a++)
	{

		for(int b = 0; b < 256; b++)
		{
			//Setting to -1
			pageTable[a][b].frame = -1;

			//Setting to 0
			pageTable[a][b].pageHits = 0;

			//setting to 0
			pageTable[a][b].pageMisses = 0;

		}
	}



	//Setting up the frametable
	FrameTableEntry theframetable[PAGETABLESIZE];

	//Setting all the vacant to true inside the pagetable
	for(int i = 0; i < PAGETABLESIZE; i++)
	{
		theframetable[i].vacant = true;
	}

	//Intializing pagehit to 0
	int pageH = 0;

	//intializing misses to 0
	int pageM = 0;

	//initialzing currentframe to 0
	int currentFrame = 0;


	//Looping throught memoryaccess
	for(int i = 0; i < memacess; i++)
	{
		//Setting Currentpid to memtable pid
		int currentPid = memtable[i].pid;

		//Setting Currentpage to memtable page
		int currentPage = memtable[i].page;

		//If its not -1 then it is a hit
		if(pageTable[currentPid][currentPage].frame != -1)
		{
			//We want to increment the hits and pagetable hits
			pageTable[currentPid][currentPage].pageHits++;
			pageH++;
		}

		//it is -1 therefore execute
		else
		{
			//We want to increment the misses and pagetable misses
			pageTable[currentPid][currentPage].pageMisses++;
			pageM++;
			
			//If vacant is true then insert here
			if(theframetable[currentFrame].vacant == true)
			{
				//Set it to false meaning its taken
				theframetable[currentFrame].vacant = false;

				//Set the page to current Page
				theframetable[currentFrame].page = currentPage;

				//Set the pid to the current pid
				theframetable[currentFrame].pid = currentPid;
				
				//pagetable at that frame is now set to current frame
				pageTable[currentPid][currentPage].frame = currentFrame;
			}
			else//Frame is already taken up
			{
				
				
				int framePage = theframetable[currentFrame].page;
				int framePid = theframetable[currentFrame].pid;

				//Clear the frame from the page table
				pageTable[framePid][framePage].frame = -1;
				//Setting page to page in frametable
				theframetable[currentFrame].page = currentPage;

				//Setting pid to pid in frametable
				theframetable[currentFrame].pid = currentPid;
				
				//Updating pagetable frame
				pageTable[currentPid][currentPage].frame = currentFrame;
			}

			//increment frame to have a new frame to put the page, modding to avoid seg faults
			currentFrame = (currentFrame + 1) % PAGETABLESIZE;
		}
	}

	printf("\nFIFO display stats\n");
	printf("# Hits: %d\n", pageH);
	printf("# of misses: %d\n", pageM);
	printf("# of mem accesses: %d \n", memacess);


	//Reseting for infinite memory
	for(int a = 0; a < 256; a++)
	{
		for(int b = 0; b < 256; b++)
		{
			//initiating this to 0 and -1
			pageTable[a][b].pageMisses = 0;
			pageTable[a][b].pageHits = 0;
			pageTable[a][b].frame = -1;
		}
	}

	//intiailzing variables
	pageH = 0;
	pageM = 0;
	for(int c = 0; c < memacess; c++)
	{
		int currPid = memtable[c].pid;
		int currPage = memtable[c].page;

		//if doesnt equal -1 its a hit
		if(pageTable[currPid][currPage].frame != -1)//HIT
		{
			//increament pagetable hits
			pageTable[currPid][currPage].pageHits++;

			//increment pagehits
			pageH++;
		}
		//It is equal to -1 so miss
		else
		{
			//set pagetable frame to 1
			pageTable[currPid][currPage].frame = 1;

			//increment misses in pagetable
			pageTable[currPid][currPage].pageMisses++;
			
			//increment miss
			pageM++;
		}
	}

	printf("\nInfinite Memory display stats\n");
	printf("# of Hits: %d\n", pageH);
	printf("# of misses: %d\n", pageM);
	printf("# of mem accesses: %d\n\n", memacess);
}
