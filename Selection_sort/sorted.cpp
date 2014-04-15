/*
   sorted.cpp
   Impementation of storage using selection sort into an array, and retrieval using binary search.
   Created as coursework for Computers and Programming II, April 2014
   Created by Joshua Tyler (URN:6213642)
*/

/* Symbolic constants prescribed by the specification */
#define DATA_FILENAME	"input.txt"
#define SEARCH_FILENAME	"find.txt"

/* Commenting out the following line ENABLES debug mode. Debug mode should be DISABLED for optimal performance */
//#define NDEBUG

/* Remove warnings about using insecure open and read functions (used carefully to maintain ANSI complience) */
#define _CRT_SECURE_NO_WARNINGS 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

/* The number of times which a store/search operation should be repeated on a data set of size 1 to ensure accurate results. */
#define NO_REPS_FOR_SIZE_1 100000

/* The number of spacer characters to print between sections of the printed report */
#define NO_SPACERS 50

/* Constants used as the return values of functions */
#define SUCCESS 1
#define FAIL 0

/* Options defining whether to output the statistics to a text file */

/* Comment out this line to disable outputting to a file */
//#define OUTPUT_TO_FILE

#define OUTPUT_FILENAME "outputStats.txt"

/* Constants relating to the debug mode */

/* Fefines how verbose the debug messages are.
   0 is off (asserts are still compiled unless the #define NDEBUG line is uncommented.)
   1 is the verbosity expected by the specification (comparisons, swaps and retrieval messages only)
   2 is extra debugging messages used to solve programming problems */
#define DEBUG_LEVEL 1

/* Symbolic constants used to conditionally compile debugging code into the program */
#ifndef NDEBUG
#define LEVEL_2			DEBUG_LEVEL == 2
#define LEVEL_1			DEBUG_LEVEL == 1
#else
#define LEVEL_2			0
#define LEVEL_1			0
#endif

/* Macros to repace debug commands with the relevant function calls if the debug conditions are met, or nothing if they're not.*/
#if LEVEL_2
#define dbgLevel1Msg(message, ...)								printf(message"\n",##__VA_ARGS__)
#define dbgLevel1ListMsg(list, value, location, event, lastRep)	deblAddMsg(list, value, location, event, lastRep)
#define dbgLevel2Msg(message, ...)								printf(message"\n",##__VA_ARGS__)
#define dbgLevel2ListMsg(list, value, location, event, lastRep)	deblAddMsg(list, value, location, event, lastRep)
#elif LEVEL_1
#define dbgLevel1Msg(message, ...)								printf(message"\n",##__VA_ARGS__)
#define dbgLevel1ListMsg(list, value, location, event, lastRep)	deblAddMsg(list, value, location, event, lastRep)
#define dbgLevel2Msg(message, ...)
#define dbgLevel2ListMsg(list, value, location, event, lastRep)
#else
#define dbgLevel1Msg(message, ...)
#define dbgLevel1ListMsg(list, value, location, event, lastRep)
#define dbgLevel2Msg(message, ...)
#define dbgLevel2ListMsg(list, value, location, event, lastRep)
#endif

/* Pass this macro a pointer and if it is null it will terminate the program giving the line number it was called from.
   It is used to check the pointers returned by malloc, calloc, fopen etc. */
#define checkPtr(pointer) if(pointer == NULL) { \
												fprintf(stderr,"NULL Pointer errror on line %d\n"                                        \
												                 "Suspected memory allocation error or file opening error.\n",__LINE__); \
												exit(EXIT_FAILURE); }
/* Custom data types */

/* Array Header */
typedef struct array
{
	int *data;
	unsigned int size;
} array;

/* A linked list used to store the data from an input file before it is processed */
typedef struct dataListElement dataListElement;
struct dataListElement
{
	int data;
	dataListElement *next;
};

/* Defines the event that has occured when a debug message is added to the list */
typedef enum debugEvent
{
	found,
	notFound,
	collision,
	saved,
	tableFull
} debugEvent;

/* A linked list used to store debug messages so that they can be output together once the operation is finished */
typedef struct debugListElement debugListElement;
struct debugListElement
{
	int value;
	unsigned int location;
	debugEvent event;
	debugListElement *next;
};

/* A header for the debug linked list.
   We need to add messages to the END of the list, this structure avoids traversing the list each time. */
typedef struct debugList
{
	debugListElement *first;
	debugListElement *last;
} debugList;

/* A structure to hold statistics about a data set that was performed with reference to the hash table */
typedef struct runtimeStats
{
	double duration;
	unsigned int attempted;
	unsigned int processed;
} runtimeStats;

/* Function prototypes - full descriptions will be found with the definitions */

/* Array functions - prefix: "ary" (ARraY) */
array aryCreate(unsigned int size);
void aryFree(array *header);
runtimeStats arySelSort(array list, unsigned int noReps, debugList *debugMessages);
void aryFromList(array destination, dataListElement *source);
void aryPrint(array list, FILE *stream);

/* Data linked list functions - prefix: "datl" (DATa linked List) */
unsigned int datlCreateFromFile(char* fileName, dataListElement** head);
void datlPrint(dataListElement* current, FILE *stream);
void datlFree(dataListElement *currentElement);

/* Debug linked list functions - prefix: "debl" (DEBug linked List)  */
void deblFree(debugList *list);
void deblAddMsg(debugList *list, int value, unsigned int location, debugEvent event, unsigned int lastRep);
void deblPrint(debugList list, unsigned int indentLevel, FILE *stream);

/* Generic printing functions (not reliant upon a custom data type) - prefix "print" */
void printHeader(FILE *stream, char *method, char *dataFilename, char *searchFilename, unsigned int stored, unsigned int searched, unsigned int found);
void printBody(FILE *stream, debugList saveList, debugList searchList);
void printSpacers(FILE *stream, char character, unsigned int number);
void printFooter(FILE *stream, double storeTime, double searchTime);
void printAddToFile(char *fileName, unsigned int noStored, unsigned int noSearched, unsigned int noFound, double timeToStore, double timeToSearch);

/* Miscellaneous functions */
unsigned int setNoReps(unsigned int noForSize1, unsigned int dataSize);


int main(void)
{
	/* Load the numbers to be stored in the table to a data linked list */
	unsigned int noToStore;
	dataListElement *unsortedList;
	noToStore = datlCreateFromFile(DATA_FILENAME, &unsortedList);
	
	dbgLevel2Msg("Loaded %d items to store.\n", noToStore);

	array list;
	list = aryCreate(noToStore);
	aryPrint(list, stdout);

	aryFromList(list, unsortedList);
	aryPrint(list, stdout);

	runtimeStats storeStats;
	debugList storeMessages = {NULL, NULL};
	storeStats = arySelSort(list, setNoReps(NO_REPS_FOR_SIZE_1, noToStore), &storeMessages);
	aryPrint(list, stdout);
	
	printf("%e\n", storeStats.duration);
	
//	datlFree(unsortedList);






	aryFree(&list);

	return(EXIT_SUCCESS);
}

/*
	Purpose:			Read the integers (stored one per line) from a text file to a data linked list structure
	Parameters:			filename - The filename of the file to open.
						head - The address of a pointer to be set to point to the first element
	Return value:		The number of integers stored
	Function calls:		Direct function calls - assert(), fopen(), fscanf(), malloc(), fclose()
						Via checkPtr macro - fprintf(), exit()
	Asserts:			fileName cannot point to nothing
						head cannot be NULL
	Revision history:	1.0 - 11/04/2014 created by Joshua Tyler
*/
unsigned int datlCreateFromFile(char* fileName, dataListElement** head)
{
	assert(fileName != NULL);
	assert(head != NULL);

	FILE *inputFile = fopen(fileName,"r");
	checkPtr(inputFile);

	unsigned int count = 0;
	int currentNum;
	dataListElement *currentItem = NULL;
	dataListElement *temp;
	*head = NULL;
	while( fscanf(inputFile,"%d", &currentNum) == 1)
	{
		count++;
		temp = (dataListElement *) malloc(sizeof(dataListElement));
		checkPtr(temp);

		/* If this is the first item in the list, set the head pointer */
		if(currentItem == NULL)
		{
			*head = currentItem = temp;
		} else {
			currentItem->next = temp;
			currentItem = temp;
		}

		currentItem->data = currentNum;
		currentItem->next = NULL;
	}

	fclose(inputFile);
	return count;
}




/*
	Purpose:			Creates an array and initialises all elements to empty
	Parameters:			size - the number of elements to have in the array
	Return value:		The header of the array
	Function calls:		Direct function calls - assert(), calloc().
						Via checkPtr macro - fprintf(), exit()
	Asserts:			Size cannot be zero.
	Revision history:	1.0 - 15/04/2014 adapted from htblCreate() by Joshua Tyler
*/
array aryCreate(unsigned int size)
{
	assert(size > 0);

	array header;
	header.data = (int *) calloc(size, sizeof(int)); // We're using calloc, so all elements will be 0
	checkPtr(header.data);

	header.size = size;

	return header;
}

/*
	Purpose:			Frees the memory allocated to an array
	Parameters:			header - A pointer to the array header to free
	Return value:		None
	Function calls:		assert(), free()
	Asserts:			header cannot be NULL
	Revision history:	1.0 - 15/04/2014 adapted from htblFree() by Joshua Tylerr
*/
void aryFree(array *header)
{
	assert(header != NULL);

	free(header->data);
	header->data = NULL;
	header->size = 0;
}


runtimeStats arySelSort(array list, unsigned int noReps, debugList *debugMessages)
{
	unsigned int sorted, lowest, position, repCtr;

	int temp;

	runtimeStats returnStats;

	time_t start, end;
	
	start=clock();
	for(sorted = 0; sorted < list.size; sorted++)
	{
		for(repCtr = noReps; repCtr > 0; repCtr--)
		{
			for(position = lowest = sorted; position < list.size; position++)
			{
				if(list.data[position] < list.data[lowest])
					lowest = position;
			}
			if(noReps == 1)
			{
				temp = list.data[sorted];
				list.data[sorted] = list.data[lowest];
				list.data[lowest] = temp;
			}
		}
	}
	end = clock();

	returnStats.duration = ( (double)(end-start)/(double)CLOCKS_PER_SEC )/(double)noReps;

	return returnStats;
}

void aryFromList(array destination, dataListElement *source)
{
	unsigned int i;

	dataListElement *current;
	current = source;

	for(i=0; current != NULL; i++)
	{
		assert(i < destination.size);

		destination.data[i] = current->data;
		current = current->next;
	}
}

void aryPrint(array list, FILE *stream)
{
	unsigned int i;
	for(i=0;i<list.size;i++)
		fprintf(stream,"%d, ", list.data[i]);

	fputc('\n',stream);

}

/*
	Purpose:			Return the number of repeats that should be performed to get reasonable results
	Parameters:			noForSize1 - The number of repeats which would be performed for a dataset of size 1
						dataSize -  The size of the dataset
	Return value:		The number of repeats which should be performed
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - 14/04/2014 created by Joshua Tyler
*/
unsigned int setNoReps(unsigned int noForSize1, unsigned int dataSize)
{
	/* If the data size is greater than or equal to the number of repeats for size 1, just repeat once */
	if(dataSize >= noForSize1)
		return 1;

	/* Otherwise, division will allow the number of repeats to decrease linearly as dataSize increases */
	return noForSize1 / dataSize;
}