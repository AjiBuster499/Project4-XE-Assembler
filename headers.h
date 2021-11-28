#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef HEADERS
#define HEADERS

struct symbol {
	int address;
	int sourceLine;
	char name[7];
    bool usesBase;
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
void addSymbol(struct symbol* tab[], unsigned int* addr, int src, char* sName);
struct symbol* symbolReturn(struct symbol* tab[], char *sname);

#endif
