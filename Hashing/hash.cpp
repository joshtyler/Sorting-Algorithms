/*
   hash.cpp
   Impementation of storage and retreival using a hash table
   Created as coursework for Computers and Programming II, April 2014
   Created by Joshua Tyler (URN:6213642)
*/

/* Symbolic constants prescribed by the specification */
#define DATA_FILENAME	"input.txt"
#define SEARCH_FILENAME	"find.txt"
#define HASH_TABLE_SIZE	10000

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
   1 is the verbosity expected by the specification (collision and retrieval messages only)
   2 is extra debugging messages used to solve programming problems */
#define DEBUG_LEVEL 1

/* Some level 2 debugging commands print the full input, output and hash table.
   This defines the maximum hash table size to do this for. */
#define MAX_TO_PRINT 20

/* Symbolic constants used to conditionally compile debugging code into the program */
#ifndef NDEBUG
#define LEVEL_2_SMALL	HASH_TABLE_SIZE <= MAX_TO_PRINT && DEBUG_LEVEL == 2
#define LEVEL_2			DEBUG_LEVEL == 2
#define LEVEL_1			DEBUG_LEVEL == 1
#else
#define LEVEL_2_SMALL	0
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

/* Defines the status of a hash table entry */
typedef enum hashEntryStatus
{
	empty = 0, // Empty = 0 so that we can initialise a hash table with just a calloc() call.
	occupied = 1,
	deleted = 2
} hashEntryStatus;

/* One element of a hashtable */
typedef struct hashEntry
{
	int data;
	hashEntryStatus status;
} hashEntry;

/* The hash table header */
typedef struct hashTable
{
	hashEntry *array;
	unsigned int size;
	unsigned int (*hashFunction)(int, unsigned int); // A pointer to the hash function used by this table
} hashTable;

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

/* The same function is used to perform all functions on the table.
   This defines the legal functions. */
typedef enum hashProcess
{
	search,
	save
} hashProcess;

/* A structure to hold statistics about a data set that was performed with reference to the hash table */
typedef struct runtimeStats
{
	double duration;
	unsigned int attempted;
	unsigned int processed;
} runtimeStats;

/* Function prototypes - full descriptions will be found with the definitions */

/* Functions relating to the hash table - prefix: "htbl" (Hash TaBLe) */
hashTable htblCreate(unsigned int size);
void htblFree(hashTable *table);
runtimeStats htblProcList(hashTable tableHeader, dataListElement *currentdataListElement, hashProcess operation, unsigned int noReps, debugList *debugMessages);
unsigned int htblProcNum(hashTable tableHeader, int numToProc, hashProcess operation, unsigned int noReps, debugList *debugMessages);
void htblPrint(hashTable table, FILE *stream);
unsigned int htblDivHash(int num, unsigned int size);

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
void printHtblFooter(FILE *stream, double storeTime, double searchTime, double percentFull);
void printAddToFile(char *fileName, unsigned int noStored, unsigned int noSearched, unsigned int noFound, double timeToStore, double timeToSearch);

/* Miscellaneous functions */
unsigned int setNoReps(unsigned int noForSize1, unsigned int dataSize);


int main(void)
{

	/* Allocate memory for table */
	hashTable table;
	table = htblCreate(HASH_TABLE_SIZE);
	table.hashFunction = &htblDivHash;

	#if LEVEL_2_SMALL
	puts("Empty table:");
	htblPrint(table, stdout);
	putchar('\n');
	#endif

	/* Load the numbers to be stored in the table to a data linked list */
	unsigned int noToStore;
	dataListElement *storeHead;
	noToStore = datlCreateFromFile(DATA_FILENAME, &storeHead);
	
	dbgLevel2Msg("Loaded %d items to store.\n", noToStore);
	#if LEVEL_2_SMALL
	puts("Input:");
	datlPrint(storeHead, stdout);
	putchar('\n');
	#endif

	/* Transfer the inputted numbers from the linked list to the hash table */
	runtimeStats storeStats;
	debugList saveMessages = {NULL, NULL};
	storeStats = htblProcList(table, storeHead, save, setNoReps(NO_REPS_FOR_SIZE_1, noToStore), &saveMessages);
	datlFree(storeHead);

	dbgLevel2Msg("Tried to store: %d items.\nStored: %d items.\nDuration %lf seconds.\n",
	             storeStats.attempted,storeStats.processed, storeStats.duration );
	#if LEVEL_2_SMALL
	puts("Populated table:");
	htblPrint(table, stdout);
	putchar('\n');
	#endif

	/* Load the numbers to be searched in the table to a data linked list */
	unsigned int noToSearch;
	dataListElement *searchHead;
	noToSearch = datlCreateFromFile(SEARCH_FILENAME, &searchHead);

	dbgLevel2Msg("Loaded %d items to search.\n", noToSearch);
	#if LEVEL_2_SMALL
	puts("To Search:");
	datlPrint(searchHead, stdout);
	putchar('\n');
	#endif

	/* Search the numbers from the linked list in the hash table */
	runtimeStats searchStats;
	debugList searchMessages = {NULL, NULL};
	searchStats = htblProcList(table, searchHead, search, setNoReps(NO_REPS_FOR_SIZE_1, noToSearch), &searchMessages);
	datlFree(searchHead);

	dbgLevel2Msg("Searched: %d items.\nFound: %d items.\nDuration %lf seconds.\n",
	             searchStats.attempted,searchStats.processed, searchStats.duration );

	/* We're now finished with the table, so can free it */
	htblFree(&table);

	/* Print the three sections of the output report */
	printHeader(stdout, "Hashing", DATA_FILENAME, SEARCH_FILENAME, storeStats.processed, searchStats.attempted, searchStats.processed);

	printBody(stdout, saveMessages, searchMessages);
	deblFree(&saveMessages);
	deblFree(&searchMessages);

	printHtblFooter(stdout, storeStats.duration, searchStats.duration, 100.00*((double)storeStats.processed/(double)HASH_TABLE_SIZE));

	/* Append the stats to a text file */
	#ifdef OUTPUT_TO_FILE
	printAddToFile(OUTPUT_FILENAME, storeStats.processed, searchStats.attempted, searchStats.processed, storeStats.duration, searchStats.duration);
	#endif

	return EXIT_SUCCESS;
}

/*
	Purpose:			Creates a hash table and initialises all elements to empty
	Parameters:			size - the size to create the table
	Return value:		The header to the hash table
	Function calls:		Direct function calls - assert(), calloc().
						Via checkPtr macro - fprintf(), exit()
	Asserts:			Size cannot be zero.
	Revision history:	1.0 - 11/04/2014 created by Joshua Tyler
*/
hashTable htblCreate(unsigned int size)
{
	assert(size > 0);

	hashTable table;
	table.array = (hashEntry *) calloc(size, sizeof(hashEntry)); // We're using calloc, so all elements will have status = 0 = empty
	checkPtr(table.array);

	table.size = size;

	return table;
}

/*
	Purpose:			Frees the memory allocated to a hash table
	Parameters:			table - A pointer to the table header to free
	Return value:		None
	Function calls:		assert(), free()
	Asserts:			table cannot be NULL
	Revision history:	1.0 - 11/04/2014 created by Joshua Tylerr
						2.0 - 15/04/2014 JT updated function to accept the table by reference and zero the contents.
*/
void htblFree(hashTable *table)
{
	assert(table != NULL);

	free(table->array);
	table->array = NULL;
	table->size = 0;
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
	Purpose:			Process the data in a linked list with repect to the hash table
	Parameters:			tableHeader - the header of the hash table to enter the data into.
						currentdataListElement - the first element of the linked list to process the data from.
						operation - the operation to perform with the data
						noReps - the number of times to repeat each operation
						debugMessages - a pointer to the header of the list to save debug messages in.
	Return value:		A runtimeStats structure containing information about how the data was processed
	Function calls:		assert(), clock(), htblProcNum()
	Asserts:			tableHeader.array cannot be NULL.
						tableHeader.size must be greater than 0.
						debugMessages cannot be NULL
	Revision history:	1.0 - 11/04/2014 created by Joshua Tyler
						2.0 - 11/04/2014 JT updated to support searching as well as storing
						3.0 - 11/04/2014 JT updated to return information about how the data was processed
						4.0 - 14/04/2014 JT updated to repeat the adding to list process a set no. of times (for timing)
*/
runtimeStats htblProcList(hashTable tableHeader, dataListElement *currentdataListElement, hashProcess operation, unsigned int noReps, debugList *debugMessages)
{
	assert(tableHeader.array != NULL);
	assert(tableHeader.size > 0);
	assert(debugMessages != NULL);

	runtimeStats returnStats = {0,0,0};
	time_t start, end;

	unsigned int repCtr;
	
	start = clock();
	while(currentdataListElement != NULL)
	{
		returnStats.attempted++;
		for(repCtr = noReps; repCtr > 0; repCtr--)
		{
			if( htblProcNum(tableHeader, currentdataListElement->data, operation, repCtr == 1, debugMessages) == SUCCESS)
			{
				/* This is true if the element was found or stored successfully */
				if(repCtr == 1)
					returnStats.processed++;
			}
		}
		currentdataListElement = currentdataListElement ->next;
	}
	end = clock();

	returnStats.duration = ( (double)(end-start)/(double)CLOCKS_PER_SEC )/(double)noReps;

	return returnStats;

}

/*
	Purpose:			Process an individual number (search for it or store it) with respect to the hash table
	Parameters:			tableHeader - the header of the hash table to insert the number in.
						numToProc - the number to process
	Return value:		SUCCESS if the number is stored successfully / number is found
						FAIL if the hash table is full / number is not found
	Function calls:		Direct function calls - assert(), tableHeader.hashFunction(),
						Via dbgLevel1ListMsg and dbgLevel2ListMsg macros - deblAddMsg()
	Asserts:			tableHeader.array cannot be NULL.
						tableHeader.size must be greater than 0.
						tableHeader.hashFunction cannot be NULL
						debugMessages cannot be NULL
						index must be less than the array size.
						operation must be save or search
	Revision history:	1.0 - 11/04/2014 created by Joshua Tyler
						2.0 - 14/04/2014 JT modified the purpose of the function so it could both store and find a number
						3.0 - 14/04/2014 JT modified to output debugging messages if debug is enabled
						4.0 - 14/04/2014 JT modified to save debug messages to a linked list so they can be run later.
						5.0 - 14/04/2014 JT modified to use linear probing (rather than quadratic) in accordance with the new specifications
						6.0 - 14/04/2014 JT modified to allow multiple repitions
*/
unsigned int htblProcNum(hashTable tableHeader, int numToProc, hashProcess operation, unsigned int lastRep, debugList *debugMessages)
{
	assert(tableHeader.array != NULL);
	assert(tableHeader.size > 0);
	assert(tableHeader.hashFunction != NULL);
	assert(debugMessages != NULL);

	unsigned int hash, index;
	hash = index = tableHeader.hashFunction(numToProc, tableHeader.size);

	unsigned int offset = 1;

	while(1)
	{
		assert(index < tableHeader.size);

		switch(operation)
		{
			case save:
				if(tableHeader.array[index].status == occupied)
				{
					dbgLevel1ListMsg(debugMessages, numToProc, index, collision, lastRep);
				} else {
					if(lastRep)
					{
						tableHeader.array[index].data = numToProc;
						tableHeader.array[index].status = occupied;
					}
					dbgLevel2ListMsg(debugMessages, numToProc, index, saved, lastRep);
					return 1;
				}
				break;

			case search:
				if(tableHeader.array[index].status == occupied)
				{
					if(tableHeader.array[index].data == numToProc)
					{
						dbgLevel1ListMsg(debugMessages, numToProc, index, found, lastRep);
						return 1;
					}
				} else if(tableHeader.array[index].status == empty) {
                    dbgLevel1ListMsg(debugMessages, numToProc, index, notFound, lastRep);
                    return 0;
				}
				break;

			default:
				assert(1);
		}

		index = (hash + offset)%tableHeader.size; // Linear probing

		/* This will be true when the table is full / we have searched the entire table */
		if(offset == tableHeader.size)
		{
			if(operation == save)
			{
				dbgLevel2ListMsg(debugMessages, numToProc, index, tableFull, lastRep);
			}
			else
			{
				dbgLevel1ListMsg(debugMessages, numToProc, index, notFound, lastRep);
			}
			return 0;
		}
		offset++;
	}

	return 0; // Code will never get here
}

/*
	Purpose:			Find the hash of a given number using the division method.
	Parameters:			num - the number to find the hash of
						size - the size of the table
	Return value:		The index of the hash.
	Function calls:		assert()
	Asserts:			size must be greater than zero.
						return value must be less than the array size
	Revision history:	1.0 - 11/04/2014 created by Joshua Tylerr
*/
unsigned int htblDivHash(int num, unsigned int size)
{
	assert(size > 0);

	unsigned int hash;
	hash = num  % size;

	assert(hash < size);

	return hash;
}


/*
	Purpose:			Print out the content of a hash table (for debugging)
	Parameters:			table - the table to print
						stream - the stream to print it to
	Return value:		None
	Function calls:		assert(), fputs(), fprintf(), fputc()
	Asserts:			stream cannot be NULL
						table.array cannot be NULL
	Revision history:	1.0 - 11/04/2014 created by Joshua Tyler
*/
void htblPrint(hashTable table, FILE *stream)
{
	assert(stream != NULL);
	assert(table.array != NULL);

	unsigned int index = 0;
	while(index < table.size)
	{
		switch(table.array[index].status)
		{
		case empty:
			fputs("- ", stream);
			break;

		case deleted:
			fputs("X ", stream);
			break;

		case occupied:
			fprintf(stream,"%d ", table.array[index].data);
			break;

		default:
			assert(1);
		}
		index++;
	}
	fputc('\n',stream);
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
	Purpose:			Add a message to a debug linked list
	Parameters:			list - the list header of the list to add it to
						value, location, and event - parameters to save into the list
						lastRep - if TRUE this function is being called on the last repitition of the store/search action
	Return value:		None
	Function calls:		Direct function calls - assert(), malloc()
						Via checkPtr macro - fprintf(), exit()
	Asserts:			list cannot be NULL
	Revision history:	1.0 - 14/04/2014 created by Joshua Tyler
						2.0 - 14/04/14 JT modified to suppport repeating
*/
void deblAddMsg(debugList *list, int value, unsigned int location, debugEvent event, unsigned int lastRep)
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
	elementToAdd->location = location;
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
	Revision history:	1.0 - 14/04/2014 created by Joshua Tyler
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
			fprintf(stream,"Value %d found in table at location %d.\n", current->value, current->location);
			break;

		case notFound:
			fprintf(stream,"Value %d not found in table.\n", current->value);
			break;

		case collision:
			fprintf(stream,"Collision occurred saving item with value %d at table location %d.\n", current->value, current->location);
			break;

		case saved:
			fprintf(stream,"Item with value %d saved at table location %d.\n", current->value, current->location);
			break;

		case tableFull:
			fprintf(stream,"Item with value %d not saved as table is full.\n", current->value);
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


/*
	Purpose:			Print the main body of results
	Parameters:			stream - the stream to output the results to
						saveList and searchList - the lists containing the messages from saving and searching
	Return value:		None
	Function calls:		assert(), fputs(), deblPrint(), printSpacers(), fputc()
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - 14/04/2014 created by Joshua Tyler
*/
void printBody(FILE *stream, debugList saveList, debugList searchList)
{
	assert(stream != NULL);

	/* If both lists are empty there is nothing to print */
	if( (saveList.first == NULL) && (searchList.first == NULL) )
		return;

	fputs("Debug Output:\n\n", stream);

	if(saveList.first != NULL)
	{
		fputs("\tStorage details:\n\n", stream);
		deblPrint(saveList, 1, stream);
	}

	if(searchList.first != NULL)
	{
		fputs("\tRetrieval details:\n\n", stream);
		deblPrint(searchList, 1, stream);
	}

	printSpacers(stream, '-', NO_SPACERS);
	fputc('\n',stream);
}

/*
	Purpose:			Print the results footer for the hash table
	Parameters:			storeTime and searchTime- the time in seconds taken to store and search the data
						percentFull - the percentage of the hash table that's filled.
						saveList and searchList - the lists containing the messages from saving and searching
	Return value:		None
	Function calls:		assert(), fputs(), fprintf(), fputc()
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - 14/04/2014 created by Joshua Tylerr
*/
void printHtblFooter(FILE *stream, double storeTime, double searchTime, double percentFull)
{
	assert(stream != NULL);

	fputs("Execution times:\n\n", stream);

	fprintf(stream,"\tTime taken to store data: %lf s\n", storeTime);
	fprintf(stream,"\tTime taken to retreive data: %lf s\n", searchTime);

	fputc('\n',stream);

	fprintf(stream,"Hash table %2.0lf%% full.\n", percentFull);

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


/*
	Purpose:			
	Parameters:			None
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - 14/04/2014 created by Joshua Tyler
*/