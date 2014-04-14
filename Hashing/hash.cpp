//#define NDEBUG // Uncommenting disables debug mode
#define _CRT_SECURE_NO_WARNINGS // We'll be careful - honest!

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define DATA_FILENAME	"input_2000.txt"
#define SEARCH_FILENAME	"find_2000.txt"
#define HASH_TABLE_SIZE	4000

#define NO_SPACERS 50

#define DEBUG_LEVEL 0 // 0 is off, 1 is collision and retrieval messages only, 2 is all debugging messsages
#define MAX_TO_PRINT 20

#define LEVEL_2_SMALL  HASH_TABLE_SIZE <= MAX_TO_PRINT && !defined(NDEBUG) && DEBUG_LEVEL == 2
#define LEVEL_2  !defined(NDEBUG) && DEBUG_LEVEL == 2
#define LEVEL_1  !defined(NDEBUG) && DEBUG_LEVEL == 1

#define debug(stream,message,...) fprintf(stream,message"\n",##__VA_ARGS__)

#if LEVEL_2
#define dbgLevel1Err(message, ...) debug(stderr,message,##__VA_ARGS__)
#define dbgLevel1Msg(message, ...) debug(stdout,message,##__VA_ARGS__)
#define dbgLevel1ListMsg(list, value, location, event ) messageToDebugList(list, value, location, event)
#define dbgLevel2Err(message, ...) debug(stderr,message,##__VA_ARGS__)
#define dbgLevel2Msg(message, ...) debug(stdout,message,##__VA_ARGS__)
#define dbgLevel2ListMsg(list, value, location, event ) messageToDebugList(list, value, location, event)
#elif LEVEL_1
#define dbgLevel1Err(message, ...) debug(stderr,message,##__VA_ARGS__)
#define dbgLevel1Msg(message, ...) debug(stdout,message,##__VA_ARGS__)
#define dbgLevel1ListMsg(list, value, location, event ) messageToDebugList(list, value, location, event)
#define dbgLevel2Err(message, ...)
#define dbgLevel2Msg(message, ...)
#define dbgLevel2ListMsg(list, value, location, event )
#else
#define dbgLevel1Err(message, ...)
#define dbgLevel1Msg(message, ...)
#define dbgLevel1ListMsg(list, value, location, event )
#define dbgLevel2Err(message, ...)
#define dbgLevel2Msg(message, ...)
#define dbgLevel2ListMsg(list, value, location, event )
#endif

#define checkPtr(pointer) if(pointer == NULL) { \
												fprintf(stderr,"NULL Pointer errror on line %d\n"                                        \
												                 "Suspected memory allocation error or file opening error.\n",__LINE__); \
												exit(EXIT_FAILURE); }

enum hashEntryStatus
{
	empty = 0,
	occupied = 1,
	deleted = 2
};
typedef enum hashEntryStatus hashEntryStatus;

struct hashEntry
{
	int data;
	hashEntryStatus status;
};
typedef struct hashEntry hashEntry;

struct hashTable
{
	hashEntry *array;
	unsigned int size;
	unsigned int (*hashFunction)(int, unsigned int);
};
typedef struct hashTable hashTable;

struct dataListElement
{
	int data;
	struct dataListElement *next;
};
typedef struct dataListElement dataListElement;

enum debugEvent
{
	found,
	notFound,
	collision,
	saved,
	tableFull
};
typedef enum debugEvent debugEvent;

struct debugListElement
{
	int value;
	unsigned int location;
	debugEvent event;
	struct debugListElement *next;
};
typedef struct dataListElement dataListElement;

struct debugList
{
	debugListElement *first;
	debugListElement *last;
};
typedef struct debugList debugList;


enum hashProcess
{
	search,
	save
};
typedef enum hashProcess hashProcess;

struct runtimeStats
{
	double duration;
	unsigned int attempted;
	unsigned int processed;
};
typedef struct runtimeStats runtimeStats;

hashTable createTable(unsigned int size);
void freeTable(hashTable table);

dataListElement * intsFromFile(char* fileName);
unsigned int intsFromFile(char* fileName, dataListElement** head);

void printList(dataListElement* current, FILE *stream);
runtimeStats hashList(hashTable tableHeader, dataListElement *currentdataListElement, hashProcess operation, debugList *debugMessages);
unsigned int hashProcessNumber(hashTable tableHeader, int numToProc, hashProcess operation, debugList *debugMessages);
unsigned int divHash(int num, unsigned int size);
void printTable(hashTable table, FILE *stream);
void freeDataList(dataListElement *currentElement);
void freeDebugList(debugList *list);

void printHeader(FILE *stream, char *method, char *dataFilename, char *searchFilename, unsigned int stored, unsigned int searched, unsigned int found);
void printBody(FILE *stream, debugList saveList, debugList searchList);
void printSpacers(FILE *stream, char character, unsigned int number);
void hashPrintFooter(FILE *stream, double storeTime, double searchTime, double percentFull);

void messageToDebugList(debugList *list, int value, unsigned int location, debugEvent event);
void printDebugList(debugList list, unsigned int indentLevel, FILE *stream);

int main(void)
{
	/* Allocate memory for table */
	hashTable table;
	table = createTable(HASH_TABLE_SIZE);
	table.hashFunction = &divHash;

	#if LEVEL_2_SMALL
	puts("Empty table:");
	printTable(table, stdout);
	putchar('\n');
	#endif

	unsigned int noToStore;
	/* Load the data to be stored in the table */
	dataListElement *storeHead;
	noToStore = intsFromFile(DATA_FILENAME, &storeHead);
	dbgLevel2Msg("Loaded %d items to store.\n", noToStore);

	#if LEVEL_2_SMALL
	puts("Input:");
	printList(storeHead, stdout);
	putchar('\n');
	#endif

	runtimeStats storeStats;
	debugList saveMessages = {NULL, NULL};
	storeStats = hashList(table, storeHead, save, &saveMessages);
	dbgLevel2Msg("Tried to store: %d items.\nStored: %d items.\nDuration %lf seconds.\n", storeStats.attempted,storeStats.processed, storeStats.duration );
	freeDataList(storeHead);

	#if LEVEL_2_SMALL
	puts("Populated table:");
	printTable(table, stdout);
	putchar('\n');
	#endif

	unsigned int noToSearch;
	dataListElement *searchHead;
	noToSearch = intsFromFile(SEARCH_FILENAME, &searchHead);
	dbgLevel2Msg("Loaded %d items to search.\n", noToSearch);

	#if LEVEL_2_SMALL
	puts("To Search:");
	printList(searchHead, stdout);
	putchar('\n');
	#endif

	runtimeStats searchStats;
	debugList searchMessages = {NULL, NULL};
	searchStats = hashList(table, searchHead, search, &searchMessages);
	dbgLevel2Msg("Searched: %d items.\nFound: %d items.\nDuration %lf seconds.\n", searchStats.attempted,searchStats.processed, searchStats.duration );
	freeDataList(searchHead);

	printHeader(stdout, "Hashing", DATA_FILENAME, SEARCH_FILENAME, storeStats.processed, searchStats.attempted, searchStats.processed);

	printBody(stdout, saveMessages, searchMessages);

	freeDebugList(&saveMessages);
	freeDebugList(&searchMessages);

	hashPrintFooter(stdout, storeStats.duration, searchStats.duration, 100.00*((double)storeStats.processed/(double)HASH_TABLE_SIZE));


	freeTable(table);

	return EXIT_SUCCESS;
}

/*
	Purpose:			Creates a hash table and initialises all elements to empty
	Parameters:			size, the size to create the table
	Return value:		The header to the hash table
	Function calls:		None
	Asserts:			Size cannot be zero.
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
*/
hashTable createTable(unsigned int size)
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
	Parameters:			table, the header to the table to free
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
*/
void freeTable(hashTable table)
{
	free(table.array);
}

/*
	Purpose:			Read the integers (stored one per line) from a text file to a linked list structure
	Parameters:			filename: The filename of the file to open.
						head: The pointer to be set to the first linked list element
	Return value:		The number of integers matched
	Function calls:		None
	Asserts:			fileName cannot point to nothing
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
*/
unsigned int intsFromFile(char* fileName, dataListElement** head)
{
	assert(fileName != NULL);

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
	Function calls:		None
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
*/
void printList(dataListElement* current, FILE *stream)
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
						currentdataListElement - the first element of the linked list to get the data from.
	Return value:		A runtimeStatistics structure containing information about how the data was processed
	Function calls:		None
	Asserts:			tableHeader.array cannot be NULL.
						tableHeader.size must be greater than 0.
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
						2.0 - Updated on 11/04/2014 by Joshua Tyler to support searching as well as storing
						3.0 - Updated on 11/04/2014 by Joshua Tyler to return information about how the data was processed
*/
runtimeStats hashList(hashTable tableHeader, dataListElement *currentdataListElement, hashProcess operation, debugList *debugMessages)
{
	assert(tableHeader.array != NULL);
	assert(tableHeader.size > 0);

	runtimeStats returnStats = {0,0,0};
	time_t start, end;
	
	start = clock();
	while(currentdataListElement != NULL)
	{
		returnStats.attempted++;
		if( hashProcessNumber(tableHeader, currentdataListElement->data, operation, debugMessages) == 1) // If found or stored
		{
			returnStats.processed++;
		}
		currentdataListElement = currentdataListElement ->next;
	}
	end = clock();

	returnStats.duration = (double)(end-start)/(double)CLOCKS_PER_SEC;

	return returnStats;

}

/*
	Purpose:			Process a number (search for it or store it) with respect o the hash table
	Parameters:			tableHeader - the header of the hash table to insert the number in.
						numToProc - the number to process
	Return value:		1 if the number is stored successfully / number is found
						0 if the hash table is full / number is not found
	Function calls:		None
	Asserts:			tableHeader.array cannot be NULL.
						tableHeader.size must be greater than 0.
						index must be less than the array size.
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
						2.0 - Modified the purpose of the function so it could both store and find a number
						3.0 - Modified to output debugging messages if debug is enabled
						4.0 - 14/04/14 Modified to save debug messages to a linked list so they can be run later.
						5.0 - 14/04/14 Modified to use linear probing in accordance with the new specifications
*/
unsigned int hashProcessNumber(hashTable tableHeader, int numToProc, hashProcess operation, debugList *debugMessages)
{
	assert(tableHeader.array != NULL);
	assert(tableHeader.size > 0);

	unsigned int hash, index;

	hash = index = tableHeader.hashFunction(numToProc, tableHeader.size);

	unsigned int offset = 1;

	while(1)
	{
//		printf("Number: %d, Index: %d\n", numToProc, index);
		assert(index < tableHeader.size);
		switch(operation)
		{
			case save:
				if(tableHeader.array[index].status == occupied)
				{
					dbgLevel1ListMsg(debugMessages, numToProc, index, collision);
//					dbgLevel1Msg("\tCollision occured storing value %d at hash table location %d.", numToProc, index);
				} else {
					tableHeader.array[index].data = numToProc;
					tableHeader.array[index].status = occupied;
//					dbgLevel2Msg("\tData with value %d stored successfully at hash table location %d.", numToProc, index);
					dbgLevel2ListMsg(debugMessages, numToProc, index, saved);
					return 1;
				}
				break;

			case search:
				if(tableHeader.array[index].status == occupied) 
					if(tableHeader.array[index].data == numToProc)
					{
//						dbgLevel1Msg("\tData value %d found at hash table location %d.", numToProc, index);
						dbgLevel1ListMsg(debugMessages, numToProc, index, found);
						return 1;
					}
				break;

			default:
				assert(1);
		}

		index = (hash + offset)%tableHeader.size; // Linear probing
		if(offset == tableHeader.size)
		{
			if(operation == save)
			{
//				dbgLevel2Msg("\tData value %d not stored in hash table. Hash table is full", numToProc);
				dbgLevel2ListMsg(debugMessages, numToProc, index, tableFull);
			}
			else
			{
//				dbgLevel1Msg("\tData value %d not found in hash table.", numToProc);
				dbgLevel1ListMsg(debugMessages, numToProc, index, notFound);
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
	Function calls:		None
	Asserts:			size must be greater than zero.
						return value must be less than the array size
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
*/
unsigned int divHash(int num, unsigned int size)
{
	assert(size > 0);
	unsigned int hash;
	hash = num  % size;
	assert(hash < size);
	return hash;
}


/*
	Purpose:			Print out the content of a hash table
	Parameters:			table - the table to print
						stream - the stream to print it to
	Return value:		None
	Function calls:		None
	Asserts:			stream cannot be NULL
						table.array cannot be NULL
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
*/
void printTable(hashTable table, FILE *stream)
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
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
*/
void freeDataList(dataListElement *currentElement)
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
	Parameters:			currentElement - A pointer to the first element to be removed
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - Initially created on 14/04/2014 by Joshua Tyler
*/;
void freeDebugList(debugList *list)
{

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
	Purpose:			To print a header file to introduce the programs output
	Parameters:			stream - the stream to write the data to
						method - the name of the method being used to store the data
						dataFilename - the name of the text file to read the data to be stored from
						searchFilename - the name of the text file to read the data to be searched from
						noToStore - the number of integers read from the data file
						noToSearch - the number fo inegers read from the search file
	Return value:		None
	Function calls:		None
	Asserts:			stream cannot be NULL
						method cannot be NULL
						dataFilename cannot be NULL
						searchFilename cannot be NULL
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
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
	Function calls:		None
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
						2.0 - Removed new line termination for greater flexibility
*/
void printSpacers(FILE *stream, char character, unsigned int number)
{
	assert(stream != NULL);
	unsigned int i;
	for(i=0; i<number;i++)
		fputc(character,stream);
}

/*
	Purpose:			Add a message to the debug list
	Parameters:			list - the list header of the list to add it to
						value, location, and event - parameters to save into the list
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - Initially created on 14/04/2014 by Joshua Tyler
*/
void messageToDebugList(debugList *list, int value, unsigned int location, debugEvent event)
{
	debugListElement *elementToAdd;

	elementToAdd = (debugListElement *)malloc(sizeof(debugListElement));
	checkPtr(elementToAdd);

	elementToAdd->value = value;
	elementToAdd->location = location;
	elementToAdd->event = event;
	
	elementToAdd->next = NULL;

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
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - Initially created on 14/04/2014 by Joshua Tyler
*/
void printDebugList(debugList list, unsigned int indentLevel, FILE *stream)
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
	Function calls:		None
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - Initially created on 14/04/2014 by Joshua Tyler
*/
void printBody(FILE *stream, debugList saveList, debugList searchList)
{
	assert(stream != NULL);

	if( (saveList.first == NULL) && (searchList.first == NULL) )
		return;

	fputs("Debug Output:\n\n", stream);

	if(saveList.first != NULL)
	{
		fputs("\tStorage details:\n\n", stream);
		printDebugList(saveList, 1, stream);
	}

	if(searchList.first != NULL)
	{
		fputs("\tRetrieval details:\n\n", stream);
		printDebugList(searchList, 1, stream);
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
	Function calls:		None
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - Initially created on 14/04/2014 by Joshua Tyler
*/
void hashPrintFooter(FILE *stream, double storeTime, double searchTime, double percentFull)
{
	fputs("Execution times:\n\n", stream);

	fprintf(stream,"\tTime taken to store data: %lf s\n", storeTime);
	fprintf(stream,"\tTime taken to retreive data: %lf s\n", searchTime);

	fputc('\n',stream);

	fprintf(stream,"Hash table %2.0lf%% full.\n", percentFull);

}

/*
	Purpose:			
	Parameters:			None
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - Initially created on 14/04/2014 by Joshua Tyler
*/