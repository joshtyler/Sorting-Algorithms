#define _CRT_SECURE_NO_WARNINGS // We'll be careful - honest!
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_SIZE 1000

#define DATA_FILENAME "input.txt"
#define SEARCH_FILENAME	"find.txt"
#define MODE 1

#define GEN_PROG_NAME "generate_numbers.exe"
#define MAIN_PROG_NAME "hashing.exe"

#define checkPtr(pointer) if(pointer == NULL) { \
												fprintf(stderr,"NULL Pointer errror on line %d\n"                                        \
												                 "Suspected memory allocation error or file opening error.\n",__LINE__); \
												exit(EXIT_FAILURE); }


int main(int argc, char** argv)
{
    if(argc != 3)
	{
        fputs("incorrect no. of arguments.\n", stderr);
        exit(EXIT_FAILURE);
	}

    int interval, maxNo;

	if(sscanf(argv[1],"%d", &interval) != 1)
        exit(EXIT_FAILURE);

    if(sscanf(argv[2],"%d", &maxNo) != 1)
        exit(EXIT_FAILURE);

    if(maxNo%interval != 0)
        exit(EXIT_FAILURE);

    int i;

    char buf[1000];

    time_t start;

    for(i=1; i<=maxNo; i==1 && interval !=1 ? i += (interval -1): i+=interval)
	{
        sprintf(buf,"%s %d %d %s", GEN_PROG_NAME, i, MODE, DATA_FILENAME);
        puts(buf);
        system(buf);
        start = clock();
//        while(clock() < start + CLOCKS_PER_SEC);
        sprintf(buf,"%s %d %d %s", GEN_PROG_NAME, i, MODE, SEARCH_FILENAME);
        puts(buf);
        system(buf);
        system(MAIN_PROG_NAME);
	}


    return EXIT_SUCCESS;
}