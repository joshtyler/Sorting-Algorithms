//#define NDEBUG // Uncommenting disables debug mode
#define _CRT_SECURE_NO_WARNINGS // We'll be careful - honest!

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define DATA_FILENAME	"input.txt"
#define SEARCH_FILENAME	"find.txt"
#define HASH_TABLE_SIZE	100

#define NO_SPACERS 50

#define DEBUG_LEVEL 1 // 0 is off, 1 is collision and retrieval messages only, 2 is all debugging messsages
#define MAX_TO_PRINT 10

#define LEVEL_2_SMALL  HASH_TABLE_SIZE <= MAX_TO_PRINT && !defined(NDEBUG) && DEBUG_LEVEL == 2
#define LEVEL_2  !defined(NDEBUG) && DEBUG_LEVEL == 2
#define LEVEL_1  !defined(NDEBUG) && DEBUG_LEVEL == 1

#define debug(stream,message,...) fprintf(stream,message"\n",##__VA_ARGS__)

#if LEVEL_2
#define dbgLevel1Err(message, ...) debug(stderr,message,##__VA_ARGS__)
#define dbgLevel1Msg(message, ...) debug(stdout,message,##__VA_ARGS__)
#define dbgLevel2Err(message, ...) debug(stderr,message,##__VA_ARGS__)
#define dbgLevel2Msg(message, ...) debug(stdout,message,##__VA_ARGS__)
#elif LEVEL_1
#define dbgLevel1Err(message, ...) debug(stderr,message,##__VA_ARGS__)
#define dbgLevel1Msg(message, ...) debug(stdout,message,##__VA_ARGS__)
#define dbgLevel2Err(message, ...)
#define dbgLevel2Msg(message, ...)
#else
#define dbgLevel1Err(message, ...)
#define dbgLevel1Msg(message, ...)
#define dbgLevel2Err(message, ...)
#define dbgLevel2Msg(message, ...)
#endif

#define checkPtr(pointer) if(pointer == NULL) { \
												fprintf(stderr,"NULL Pointer errror on line %d\n                                     \
												                 Suspected memory allocation error or file opening error",__LINE__); \
												exit(EXIT_FAILURE); }


enum hashEntryStatus
{
	empty = 0,
	occupied = 1,
	deleted = 2
};

struct hashEntry
{
	int data;
	enum hashEntryStatus status;
};
typedef struct hashEntry hashEntry;

struct hashHeader
{
	hashEntry *array;
	unsigned int size;
	unsigned int (*hashFunction)(int, unsigned int);
};
typedef struct hashHeader hashHeader;

struct listElement
{
	int data;
	struct listElement *next;
};
typedef struct listElement listElement;

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

hashHeader createTable(unsigned int size);
void freeTable(hashHeader table);
listElement * intsFromFile(char* fileName);
unsigned int intsFromFile(char* fileName, listElement** head);
void printList(listElement* current, FILE *stream);
runtimeStats hashList(hashHeader tableHeader, listElement *currentListElement, hashProcess operation);
unsigned int hashProcessNumber(hashHeader tableHeader, int numToProc, hashProcess operation);
unsigned int divHash(int num, unsigned int size);
void printTable(hashHeader table, FILE *stream);
void freeList(listElement *);
void printHeader(FILE *stream, char *method, char *dataFilename, char *searchFilename, unsigned int noToStore, unsigned int noToSearch);
void printSpacers(FILE *stream, char character, unsigned int number);

int main(void)
{

	hashHeader table;
	table = createTable(HASH_TABLE_SIZE);
	table.hashFunction = &divHash;

	#if LEVEL_2_SMALL
	puts("Empty table:");
	printTable(table, stdout);
	#endif

	unsigned int noToStore, noToSearch;

	listElement *storeHead;
	noToStore = intsFromFile(DATA_FILENAME, &storeHead);

	#if LEVEL_2_SMALL
	puts("Input:");
	printList(storeHead, stdout);
	#endif

	listElement *searchHead;
	noToSearch = intsFromFile(SEARCH_FILENAME, &searchHead);

	#if LEVEL_2_SMALL
	puts("To Search:");
	printList(searchHead, stdout);
	#endif

	printHeader(stdout, "Hashing", DATA_FILENAME, SEARCH_FILENAME, noToStore, noToSearch);

	dbgLevel1Msg("Debug Output:\n\n\tStorage details:");

	runtimeStats storeStats;
	storeStats = hashList(table, storeHead, save);
	dbgLevel2Msg("Tried to store: %d items.\nStored: %d items.\nDuration %lf seconds.\n", storeStats.attempted,storeStats.processed, storeStats.duration );

	#if LEVEL_2_SMALL
	puts("Populated table:");
	printTable(table, stdout);
	#endif

	dbgLevel1Msg("\n\tRetrieval details:");

	runtimeStats searchStats;
	searchStats = hashList(table, searchHead, search);
	dbgLevel2Msg("Searched: %d items.\nFound: %d items.\nDuration %lf seconds.\n", searchStats.attempted,searchStats.processed, searchStats.duration );

	printSpacers(stdout, '-', NO_SPACERS);


	freeTable(table);
	freeList(storeHead);
	freeList(searchHead);

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
hashHeader createTable(unsigned int size)
{
	assert(size > 0);

	hashHeader table;
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
void freeTable(hashHeader table)
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
unsigned int intsFromFile(char* fileName, listElement** head)
{
	assert(fileName != NULL);

	FILE *inputFile = fopen(fileName,"r");
	checkPtr(inputFile);

	unsigned int count = 0;
	int currentNum;
	listElement *currentItem = NULL;
	listElement *temp;
	*head = NULL;
	while( fscanf(inputFile,"%d", &currentNum) == 1)
	{
		count++;
		temp = (listElement *) malloc(sizeof(listElement));
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
void printList(listElement* current, FILE *stream)
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
						currentListElement - the first element of the linked list to get the data from.
	Return value:		A runtimeStatistics structure containing information about how the data was processed
	Function calls:		None
	Asserts:			tableHeader.array cannot be NULL.
						tableHeader.size must be greater than 0.
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
						2.0 - Updated on 11/04/2014 by Joshua Tyler to support searching as well as storing
						3.0 - Updated on 11/04/2014 by Joshua Tyler to return information about how the data was processed
*/
runtimeStats hashList(hashHeader tableHeader, listElement *currentListElement, hashProcess operation)
{
	assert(tableHeader.array != NULL);
	assert(tableHeader.size > 0);

	runtimeStats returnStats = {0,0,0};
	time_t start, end;
	
	start = clock();
	while(currentListElement != NULL)
	{
		returnStats.attempted++;
		if( hashProcessNumber(tableHeader, currentListElement->data, operation) == 1) // If found or stored
		{
			returnStats.processed++;
		}
		currentListElement = currentListElement ->next;
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
*/
unsigned int hashProcessNumber(hashHeader tableHeader, int numToProc, hashProcess operation)
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
					dbgLevel1Msg("\tCollision occured storing value %d at hash table location %d.", numToProc, index);
				} else {
					tableHeader.array[index].data = numToProc;
					tableHeader.array[index].status = occupied;
					dbgLevel2Msg("\tData with value %d stored successfully at hash table location %d.", numToProc, index);
					return 1;
				}
				break;

			case search:
				if(tableHeader.array[index].status == occupied) 
					if(tableHeader.array[index].data == numToProc)
					{
						dbgLevel1Msg("\tData value %d found at hash table location %d.", numToProc, index);
						return 1;
					}
				break;

			default:
				assert(1);
		}

		index = (hash + offset*offset)%tableHeader.size; // Quadratic probing
		if(offset == tableHeader.size)
		{
			if(operation == save)
				dbgLevel1Msg("\tData value %d not stored in hash table. Hash table is full", numToProc);
			else
				dbgLevel1Msg("\tData value %d not found in hash table.", numToProc);
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
void printTable(hashHeader table, FILE *stream)
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
	Purpose:			Free the memory used by a linked list
	Parameters:			currentElement - A pointer to the first element to be removed
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
*/
void freeList(listElement *currentElement)
{
	listElement *prevElement = currentElement;

	do
	{
		currentElement = prevElement->next;
		free(prevElement);
		prevElement = currentElement;
	}while(currentElement != NULL);

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
void printHeader(FILE *stream, char *method, char *dataFilename, char *searchFilename, unsigned int noToStore, unsigned int noToSearch)
{
	assert(stream != NULL);
	assert(method != NULL);
	assert(dataFilename != NULL);
	assert(searchFilename != NULL);

	fputs("Data storage and retrieval: A comparison of hasing and directed search of sorted data.\n", stream);

	printSpacers(stream, '=', NO_SPACERS);

	fprintf(stream, "Storage method: %s\n", method);

	fputc('\n',stream);

	fprintf(stream, "Number of items to be stored: %d\n", noToStore);
	fprintf(stream, "Number of items to be searched: %d\n", noToSearch);

	printSpacers(stream, '-', NO_SPACERS);
	
}

/*
	Purpose:			Prints a row of spacer characters terminated by a new line
	Parameters:			stream - the stream to print the character to
						character - the character to print
						number - the number of spacers to print
	Return value:		None
	Function calls:		None
	Asserts:			stream cannot be NULL
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
*/
void printSpacers(FILE *stream, char character, unsigned int number)
{
	assert(stream != NULL);
	unsigned int i;
	for(i=0; i<number;i++)
		fputc(character,stream);
	fputc('\n',stream);
}


/*
	Purpose:			
	Parameters:			None
	Return value:		None
	Function calls:		None
	Asserts:			None
	Revision history:	1.0 - Initially created on 11/04/2014 by Joshua Tyler
*/