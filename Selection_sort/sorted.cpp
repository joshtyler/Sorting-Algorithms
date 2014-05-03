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
#define NDEBUG

/* Remove warnings about using insecure open and read functions (used carefully to maintain ANSI complience) */
#define _CRT_SECURE_NO_WARNINGS 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

/* The number of times which a store/search operation should be repeated on a data set of size 1 to ensure accurate results. */
#define NO_REPS_FOR_SIZE_1 500000

/* The number of spacer characters to print between sections of the printed report */
#define NO_SPACERS 50

/* Constants used as the return values of functions */
#define SUCCESS 1
#define FAIL 0

/* Options defining whether to output the statistics to a text file */

/* Comment out this line to disable outputting to a file */
#define OUTPUT_TO_FILE

#define OUTPUT_FILENAME "selSortOutputStats.txt"

/* Constants relating to the debug mode */

/* Macros to repace debug commands with the relevant function calls if the debug conditions are met, or nothing if they're not.*/
#ifdef NDEBUG
#define dbgListMsg(list, value, event, lastRep)
#define dbgCtrInc(finalRep, counter)
#else
#define dbgListMsg(list, value, event, lastRep)		deblAddMsg(list, value, event, lastRep)
#define dbgCtrInc(finalRep, counter)				if(finalRep) counter++  
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
} debugEvent;

/* A linked list used to store debug messages so that they can be output together once the operation is finished */
typedef struct debugListElement debugListElement;
struct debugListElement
{
	int value;
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
	unsigned int comparisons;
	unsigned int swaps;
} runtimeStats;

/* Function prototypes - full descriptions will be found with the definitions */

/* Array functions - prefix: "ary" (ARraY) */
array aryCreate(unsigned int size);
void aryFree(array *header);
runtimeStats arySelSort(array list, unsigned int noReps);
void aryFromList(array destination, dataListElement *source);
void aryPrint(array list, FILE *stream);
runtimeStats aryBinSearchList(array dataArray, dataListElement *currentSearchListElement, unsigned int noReps, debugList *messages);
unsigned int aryBinSearchNum(array dataArray, int searchData);

/* Data linked list functions - prefix: "datl" (DATa linked List) */
unsigned int datlCreateFromFile(char* fileName, dataListElement** head);
void datlPrint(dataListElement* current, FILE *stream);
void datlFree(dataListElement *currentElement);

/* Debug linked list functions - prefix: "debl" (DEBug linked List)  */
void deblFree(debugList *list);
void deblAddMsg(debugList *list, int value, debugEvent event, unsigned int lastRep);
void deblPrint(debugList list, unsigned int indentLevel, FILE *stream);

/* Generic printing functions (not reliant upon a custom data type) - prefix "print" */
void printHeader(FILE *stream, char *method, char *dataFilename, char *searchFilename, unsigned int stored, unsigned int searched, unsigned int found);
void printAryBody(FILE *stream, unsigned int comparisons, unsigned int swaps, debugList searchList);
void printSpacers(FILE *stream, char character, unsigned int number);
void printFooter(FILE *stream, double storeTime, double searchTime);
void printAddToFile(char *fileName, unsigned int noStored, unsigned int noSearched, unsigned int noFound, double timeToStore, double timeToSearch);

/* Miscellaneous functions */
unsigned int setNoReps(unsigned int noForSize1, unsigned int dataSize);


int main(void)
{
	/* Load the numbers to be stored in the array to a data linked list */
	unsigned int noToStore;
	dataListElement *unsortedList;
	noToStore = datlCreateFromFile(DATA_FILENAME, &unsortedList);

	/* Allocate an array big enough to store all the items */
	array mainArray;
	mainArray = aryCreate(noToStore);

	/* Copy the linked list input to the array */
	aryFromList(mainArray, unsortedList);

	/* We're now finished with the input linked list */
	datlFree(unsortedList);

	/* Sort the array with selection sort */
	runtimeStats storeStats;
	storeStats = arySelSort(mainArray, setNoReps(NO_REPS_FOR_SIZE_1, noToStore));
	
	/* Load the numbers to search into a linked list */
	unsigned int noToSearch;
	dataListElement *searchList;
	noToSearch = datlCreateFromFile(SEARCH_FILENAME, &searchList);

	/* Search for the items in the array using binary search */
	runtimeStats searchStats;
	debugList searchMessages = {NULL,NULL};
	searchStats = aryBinSearchList(mainArray, searchList,setNoReps(NO_REPS_FOR_SIZE_1, noToSearch), &searchMessages);
//	searchStats = aryBinSearchList(mainArray, searchList,NO_REPS_FOR_SIZE_1, &searchMessages); // Binary search is really fast, so just use the no. of reps for size 1 for all!

	/* We're now finished with the search list and array */
	datlFree(searchList);
	aryFree(&mainArray);

	/* Print the reults */
	printHeader(stdout, "Binary Search", DATA_FILENAME, SEARCH_FILENAME, storeStats.processed, searchStats.attempted, searchStats.processed);
	printAryBody(stdout, storeStats.comparisons, storeStats.swaps, searchMessages);
	deblFree(&searchMessages);
	printFooter(stdout, storeStats.duration, searchStats.duration);


	/* Append the stats to a text file */
	#ifdef OUTPUT_TO_FILE
	printAddToFile(OUTPUT_FILENAME, storeStats.processed, searchStats.attempted, searchStats.processed, storeStats.duration, searchStats.duration);
	#endif

	return(EXIT_SUCCESS);
}

/* Array Functions */
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

/*
	Purpose:			Perform selection sort on an array of data to sort it into ascending order
	Parameters:			list - the array to sort
						noReps - the number of times to repeat sorting each item (for accurate timing)
	Return value:		A runtimeStats structure containing statistics about the sorting
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - 15/04/2014 created by Joshua Tyler
						2.6 - 16/04/2014 JT updated to add conditionally compiling swap and comparison stats.
*/
runtimeStats arySelSort(array list, unsigned int noReps)
{
	unsigned int sorted, lowest, position, repCtr;

	int temp;

	runtimeStats returnStats = {0.0, 0, 0, 0, 0};

	time_t start, end;
	
	start=clock();
	for(sorted = 0; sorted < list.size - 1; sorted++)
	{
		for(repCtr = noReps; repCtr > 0; repCtr--)
		{
			lowest = sorted;
			for(position = lowest + 1; position < list.size; position++)
			{
				dbgCtrInc(repCtr == 1, returnStats.comparisons);

				if(list.data[position] < list.data[lowest])
					lowest = position;
			}
			if(repCtr == 1)
				if(lowest != sorted)
				{
					dbgCtrInc(repCtr == 1, returnStats.swaps);
					temp = list.data[sorted];
					list.data[sorted] = list.data[lowest];
					list.data[lowest] = temp;
				}
		}
	}
	end = clock();

	returnStats.duration = ( (double)(end-start)/(double)CLOCKS_PER_SEC )/(double)noReps;

	returnStats.attempted = returnStats.processed = list.size;

	return returnStats;
}

/*
	Purpose:			Transfer data from a linked list to an array
	Parameters:			None
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - 15/04/2014 created by Joshua Tyler
*/
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

/*
	Purpose:			Print the contents of an array
	Parameters:			None
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - 15/04/2014 created by Joshua Tyler
*/
void aryPrint(array list, FILE *stream)
{
	unsigned int i;
	for(i=0;i<list.size;i++)
		fprintf(stream,"%d, ", list.data[i]);

	fputc('\n',stream);

}

/*
	Purpose:			Perform binary search on a data array using values from a linked list
	Parameters:			None
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - 16/04/2014 created by Joshua Tyler
*/
runtimeStats aryBinSearchList(array dataArray, dataListElement *currentSearchListElement, unsigned int noReps, debugList *messages)
{
	assert(dataArray.data != NULL);
	assert(dataArray.size > 0);
	assert(messages != NULL);

	runtimeStats returnStats = {0,0,0};
	time_t start, end;

	unsigned int repCtr;
	
	start = clock();
	while(currentSearchListElement != NULL)
	{
		returnStats.attempted++;
		for(repCtr = noReps; repCtr > 0; repCtr--)
		{
			if( aryBinSearchNum(dataArray, currentSearchListElement->data) == SUCCESS)
			{
				/* This is true if the element was found or stored successfully */
				dbgListMsg(messages, currentSearchListElement->data, found, repCtr == 1);
				if(repCtr == 1)
					returnStats.processed++;
			} else {
				dbgListMsg(messages, currentSearchListElement->data, notFound, repCtr == 1);
			}
		}
		currentSearchListElement = currentSearchListElement ->next;
	}
	end = clock();

	returnStats.duration = ( (double)(end-start)/(double)CLOCKS_PER_SEC )/(double)noReps;

	return returnStats;

}

/*
	Purpose:			Perform binary search on an array with a specific value
	Parameters:			None
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - 16/04/2014 created by Joshua Tyler
*/
unsigned int aryBinSearchNum(array dataArray, int searchData)
{
	assert(dataArray.data != NULL);
	assert(dataArray.size > 0);

	int left, right, mid;

	left = 0;
	right = dataArray.size - 1;

	while(left <= right)
	{
		mid = (left+right)/2;

		if(dataArray.data[mid] > searchData)
		{
			right = mid-1;
		} else if(dataArray.data[mid] < searchData)
		{
			left = mid + 1;
		} else
		{
			return SUCCESS;
		}

	}
	return FAIL;
}

/* Data linked list functions */
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
	Purpose:			Print the contents of a linked list to stdout (for debugging)
	Parameters:			current -  a pointer to the first element in the linked list
						stream - the output stream to write the contents to
	Return value:		None
	Function calls:		assert(), fprintf(), fputc()
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - 11/04/2014 created by Joshua Tyler
*/
void datlPrint(dataListElement* current, FILE *stream)
{
	assert(stream != NULL);
	
	while(current != NULL)
	{
		fprintf(stream,"%d, ", current->data);
		current = current->next;
	}

	fputc('\n',stream);

	return;
}

/*
	Purpose:			Free the memory used by a data linked list
	Parameters:			currentElement - A pointer to the first element to be removed
	Return value:		None
	Function calls:		free()
	Asserts:			None
	Revision history:	1.0 - 11/04/2014 created by Joshua Tyler
						2.0 - 14/04/2014 JT modified to remove bugs
*/
void datlFree(dataListElement *currentElement)
{

	dataListElement *temp;

	while(currentElement !=NULL)
	{
		temp = currentElement->next;
		free(currentElement);
		currentElement = temp;
	}

}

/* Debug linked list functions */
/*
	Purpose:			Free the memory used by a debug linked list
	Parameters:			list - a pointer to the list whose memory we want to free
	Return value:		None
	Function calls:		None
	Asserts:			list cannot be NULL
	Revision history:	1.0 - 11/04/2014 created by Joshua Tyler
						2.0 - 14/04/14 JT modified to add assert.
*/
void deblFree(debugList *list)
{
	assert(list != NULL);

	debugListElement *nodeToFree = list->first;

	debugListElement *temp;

	while(nodeToFree != NULL)
	{
		temp = nodeToFree->next;
		free(nodeToFree);
		nodeToFree = temp;
	}

	list->first = list->last = NULL;

}

/*
	Purpose:			Add a message to a debug linked list
	Parameters:			list - the list header of the list to add it to
						value and event - parameters to save into the list
						lastRep - if TRUE this function is being called on the last repitition of the store/search action
	Return value:		None
	Function calls:		Direct function calls - assert(), malloc()
						Via checkPtr macro - fprintf(), exit()
	Asserts:			list cannot be NULL
	Revision history:	1.0 - 16/04/2014 JT adapted from deblAddMsg() in hashed.cpp
*/
void deblAddMsg(debugList *list, int value,  debugEvent event, unsigned int lastRep)
{
	assert(list != NULL);

	/* If this is not the last repitition, return to avoid repeat messages */
	if(!lastRep)
		return;

	debugListElement *elementToAdd;

	/* Allocate space for an element */
	elementToAdd = (debugListElement *)malloc(sizeof(debugListElement));
	checkPtr(elementToAdd);

	/* Set values of new element */
	elementToAdd->value = value;
	elementToAdd->event = event;
	elementToAdd->next = NULL;

	/* Add element to the end of the list */
	if(list->first == NULL)
	{
		list->first = elementToAdd;
		list->last = elementToAdd;
	} else {
		list->last->next = elementToAdd;
		list->last = elementToAdd;
	}

}

/*
	Purpose:			Prints the messages stored in a debugList
	Parameters:			list - the debugList to print
						indentLevel - the number of tabs to print before each message
						stream - the output stream to print to
	Return value:		None
	Function calls:		assert(), printSpacers(), fprintf(), fputc()
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - 16/04/2014 adapted form deblPrint() used in hash.cpp by Joshua Tyler
*/
void deblPrint(debugList list, unsigned int indentLevel, FILE *stream)
{
	assert(stream != NULL);
	
	debugListElement *current = list.first;

	while(current != NULL)
	{
		printSpacers(stream, '\t', indentLevel);
		switch (current->event)
		{
		case found:
			fprintf(stream,"Value %d found in array.\n", current->value);
			break;

		case notFound:
			fprintf(stream,"Value %d not found in array.\n", current->value);
			break;

		default:
			assert(1);
			break;
		}
		current = current->next;
	}

	fputc('\n',stream);

	return;
}

/* Generic printing functions */
/*
	Purpose:			Print a header file to introduce the programs output
	Parameters:			stream - the stream to write the data to
						method - the name of the method being used to store the data
						dataFilename - the name of the text file to read the data to be stored from
						searchFilename - the name of the text file to read the data to be searched from
						stored - the number of integers stored
						searched - the number of integers searched
						found - the number of integers found
	Return value:		None
	Function calls:		assert(), printSpacers(), fputc(), fprintf()
	Asserts:			stream cannot be NULL
						method cannot be NULL
						dataFilename cannot be NULL
						searchFilename cannot be NULL
	Revision history:	1.0 - 11/04/2014 created by Joshua Tyler
						2.0 - 14/04/14 JT modified to change format and display more information
*/
void printHeader(FILE *stream, char *method, char *dataFilename, char *searchFilename, unsigned int stored, unsigned int searched, unsigned int found)
{
	assert(stream != NULL);
	assert(method != NULL);
	assert(dataFilename != NULL);
	assert(searchFilename != NULL);

	fputs("Data storage and retrieval: A comparison of hashing and directed search of sorted data.\n", stream);

	printSpacers(stream, '=', NO_SPACERS);
	fputc('\n',stream);

	fprintf(stream, "Storage method: %s\n", method);

	fputc('\n',stream);

	fprintf(stream, "Input data loaded from file: %s\n", dataFilename);
	fprintf(stream, "Retreival data loaded from file: %s\n", searchFilename);

	fputc('\n',stream);

	fprintf(stream, "Number of items stored: %d\n", stored);
	fprintf(stream, "Number of items searched: %d\n", searched);
	fprintf(stream, "Number of items found: %d\n", found);

	printSpacers(stream, '-', NO_SPACERS);
	fputc('\n',stream);
	
}

/*
	Purpose:			Print the main body of results
	Parameters:			stream - the stream to output the results to
						comparisons - the number of comparisons made (0 if debug output is off)
						swaps - the number of swaps made (0 if debug output is off)
						searchList - the list containing the messages from searching
	Return value:		None
	Function calls:		assert(), fputs(), deblPrint(), printSpacers(), fputc()
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - 14/04/2014 adapted from printHtblBody() by Joshua Tyler
*/
void printAryBody(FILE *stream, unsigned int comparisons, unsigned int swaps, debugList searchList)
{
	assert(stream != NULL);

	/* If no comparisons were made then debug mode is off, so we don't need to so anything */
	if( comparisons == 0 )
		return;

	fputs("Debug Output:\n\n", stream);

	fputs("\tStorage details:\n\n", stream);
	fprintf(stream, "\tNumber of swaps: %d.\n", swaps);
	fprintf(stream, "\tNumber of comparisons: %d.\n", comparisons);

	if(searchList.first != NULL)
	{
		fputs("\tRetrieval details:\n\n", stream);
		deblPrint(searchList, 1, stream);
	}

	printSpacers(stream, '-', NO_SPACERS);
	fputc('\n',stream);
}

/*
	Purpose:			Prints a row of spacer characters
	Parameters:			stream - the stream to print the character to
						character - the character to print
						number - the number of spacers to print
	Return value:		None
	Function calls:		assert(), fputc()
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - 11/04/2014 created by Joshua Tyler
						2.0 - 14/04/14 JT removed new line termination for greater flexibility
*/
void printSpacers(FILE *stream, char character, unsigned int number)
{
	assert(stream != NULL);
	unsigned int i;
	for(i=0; i<number;i++)
		fputc(character,stream);
}

/*
	Purpose:			Print a results footer
	Parameters:			storeTime and searchTime- the time in seconds taken to store and search the data
						saveList and searchList - the lists containing the messages from saving and searching
	Return value:		None
	Function calls:		assert(), fputs(), fprintf(), fputc()
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - 16/04/2014 adapted from printHtblFooter() by Joshua Tyler
*/
void printFooter(FILE *stream, double storeTime, double searchTime)
{
	assert(stream != NULL);

	fputs("Execution times:\n\n", stream);

	fprintf(stream,"\tTime taken to store data: %lf s\n", storeTime);
	fprintf(stream,"\tTime taken to retreive data: %lf s\n", searchTime);

}

/*
	Purpose:			Write statistics to an output text file in CSV format
	Parameters:			fileName - the name of the output file to append to
						noStored, noSearched, noFound, timeToStore, timeToSearch - parameters to output
	Return value:		None
	Function calls:		Direct function calls - assert(), fopen(), fprintf(), fclose()
						Via checkPtr macro - fprintf(), exit()
	Asserts:			fileName cannot be NULL
	Revision history:	1.0 - 14/04/2014 created by Joshua Tyler
						2.0 - 15/04/2014 JT added the filename as a parameter
*/
void printAddToFile(char *fileName, unsigned int noStored, unsigned int noSearched, unsigned int noFound, double timeToStore, double timeToSearch)
{
	assert(fileName != NULL);

	/* Open the file */
	FILE *outputFile;
	outputFile = fopen(fileName,"a");
	checkPtr(outputFile);

	/* Output the data */
	fprintf(outputFile,"%d,%d,%d,%e,%e\n", noStored, noSearched, noFound, timeToStore, timeToSearch);

	/* Close the file */
	fclose(outputFile);
}

/* Miscellaneous functions */
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
//	dataSize = dataSize * dataSize; // Square datasize so that it goes down quadratically.

	if(dataSize == 0)
		return noForSize1;

	/* If the data size is greater than or equal to the number of repeats for size 1, just repeat once */
	if(dataSize >= noForSize1)
		return 1;

	/* Otherwise, division will allow the number of repeats to decrease quadratically as dataSize increases */
	return noForSize1 / dataSize;
}