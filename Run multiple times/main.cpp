#define _CRT_SECURE_NO_WARNINGS // We'll be careful - honest!
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_SIZE 1000

#define DATA_FILENAME "input.txt"
#define SEARCH_FILENAME	"find.txt"


#define GEN_PROG_NAME "generate_numbers.exe"
#define MAIN_PROG_NAME "hashing.exe"

#define checkPtr(pointer) if(pointer == NULL) { \
												fprintf(stderr,"NULL Pointer errror on line %d\n"                                        \
												                 "Suspected memory allocation error or file opening error.\n",__LINE__); \
												exit(EXIT_FAILURE); }


int main(int argc, char** argv)
{
    if(argc != 5)
	{
        fputs("incorrect no. of arguments.\n", stderr);
		fprintf(stderr,"correct usage: %s [progname] [interval] [maxNo] [mode] .\n", argv[0]);
        exit(EXIT_FAILURE);
	}

    int interval, maxNo, mode;

	if(sscanf(argv[2],"%d", &interval) != 1)
        exit(EXIT_FAILURE);

    if(sscanf(argv[3],"%d", &maxNo) != 1)
        exit(EXIT_FAILURE);

    if(sscanf(argv[4],"%d", &mode) != 1)
        exit(EXIT_FAILURE);

    if(maxNo%interval != 0)
	{
		fputs("FAM, your max no. is not a multiple of your interval! SORT IT OUT.\n", stderr);
        exit(EXIT_FAILURE);
	}


    int i;

    char buf[BUFFER_SIZE];

    time_t start;

    for(i=1; i<=maxNo; i==1 && interval !=1 ? i += (interval -1): i+=interval)
	{
        sprintf(buf,"%s %d %d %s", GEN_PROG_NAME, i, mode, DATA_FILENAME);
        puts(buf);
        system(buf);
        start = clock();
//        while(clock() < start + CLOCKS_PER_SEC);
        sprintf(buf,"%s %d %d %s", GEN_PROG_NAME, i, mode, SEARCH_FILENAME);
        puts(buf);
        system(buf);
        system(argv[1]);
	}


    return EXIT_SUCCESS;
}