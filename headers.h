#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HEADERS
#define HEADERS

struct symbol {
	int address;
	int sourceLine;
	char name[7];
};

struct syminst{
    char iname[5];
    int opcode;
};

struct countertrack{
    int line;
    unsigned int counter;
};

int matchDirective(char* name);
int checkspecial(char* symbol);
int symbolExists(struct symbol* tab[], char *sname);
int IsAValidSymbol(char *TestSymbol, struct symbol* tab[]);

#endif
