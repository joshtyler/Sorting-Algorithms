#define _CRT_SECURE_NO_WARNINGS // We'll be careful - honest!

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define checkPtr(pointer) if(pointer == NULL) { \
												fprintf(stderr,"NULL Pointer errror on line %d\n"                                        \
												                 "Suspected memory allocation error or file opening error.\n",__LINE__); \
												exit(EXIT_FAILURE); }

int compAsc(const void * a, const void * b);
int compDsc(const void * a, const void * b);

int main(int argc, char** argv)
{
    if(argc != 4)
	{
        fputs("incorrect no. of arguments.\n", stderr);
        exit(EXIT_FAILURE);
	}

    int number, mode;

	if(sscanf(argv[1],"%d", &number) != 1)
        exit(EXIT_FAILURE);

    if(sscanf(argv[2],"%d", &mode) != 1)
        exit(EXIT_FAILURE);

    int *array;

    array = (int *) malloc(number*sizeof(int));

    srand ((time(NULL) + number)*argv[3][0]);

    int i,j;
    for(i=0; i < number; i++)
	{
	    array[i] = rand();
//        printf("%d, ",array[i]);
	}


    switch(mode)
	{
	    case 1: // Don't sort
            break;

		case 2: // Sort Ascending
            qsort((void *)array,(size_t)number,sizeof(int),compAsc);
            break;

		case 3: //Sort Descending
            qsort((void *)array,(size_t)number,sizeof(int),compDsc);
            break;

		default:
            exit(EXIT_FAILURE);
	}

    FILE *writeFile;

    writeFile = fopen(argv[3],"w");
    checkPtr(writeFile);

    for(i=0; i < number; i++)
	    fprintf(writeFile, "%d\n", array[i]);

    fclose(writeFile);

    free(array);

    return EXIT_SUCCESS;
}


int compAsc(const void * a, const void * b)
{
    if( *(int*)a < *(int*)b )
        return -1;

    if( *(int*)a > *(int*)b )
        return 1;

    return 0;
}

int compDsc(const void * a, const void * b)
{
    if( *(int *)a < *(int *)b )
        return -1;

    if( *(int *)a > *(int *)b )
        return 1;

    return 0;
}