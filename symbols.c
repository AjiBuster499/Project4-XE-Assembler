#include "headers.h"

int IsAValidSymbol(char *TestSymbol, struct symbol* tab[]) {
  int result = 1;
  if( strlen(TestSymbol) > 6 ) {
    return result = 2; // this indicates that the string is more than 6, and therefore invalid;
  } else if(matchDirective(TestSymbol) == 0) {
    return result = 3; //the result has the same name as the directive
  } else if(checkspecial(TestSymbol) == 0) {
    return result = 4; //the directive name contains a special character
  } else if(symbolExists(tab, TestSymbol)) {
    return result = 5;
  }

	return result;
}

int matchDirective(char* name) {
    if ((strcmp("START", name) == 0) || (strcmp("END", name) == 0) || (strcmp("BYTE", name) == 0) ||
        (strcmp("WORD", name) == 0) || (strcmp("RESB", name) == 0) || (strcmp("RESW", name) == 0) ||
        (strcmp("RESR", name) == 0) || (strcmp("EXPORTS", name) == 0)) {
      return 0;
    } else {
      return 1;
    }
}

void addSymbol(struct symbol* tab[], unsigned int* addr, int src, char* sName) {
    int index = 0;
    struct symbol *newSym;
    newSym = malloc(sizeof (struct symbol));
    newSym->address = *addr;
    newSym->sourceLine = src;
    newSym->usesBase = false;
    strcpy(newSym->name,sName);
    
    while(tab[index] != NULL) {
      index++; //when this finishes, we've found an empty slot
    }
    
    tab[index] = newSym;
    //we want to malloc a new symbol
}

int symbolExists(struct symbol* tab[], char *sname) {
  int result = 0;
  int index = 0;
  
  while (tab[index] != NULL) {
    if (strcmp(sname, tab[index]->name) == 0) {
      result = -1;
      break;
    }
    index++;
  }

  return result;
}

struct symbol* symbolReturn(struct symbol* tab[], char *sname) {
    struct symbol* result = NULL;
    strtok(sname, " ");
    char* sym;
    int index = 0;
    while(tab[index] != NULL) {
      sym = tab[index]->name;
      if(strcmp(sname, sym) == 0) {
        result = tab[index];
        break;
      }
      index++;
    }
    return result;
}
int symbolIndex(struct symbol* tab[], char *sname) {
    struct symbol* result = NULL;
    strtok(sname, " ");
    char* sym;
    int index = 0;
    while(tab[index] != NULL) {
        sym = tab[index]->name;
        if(strcmp(sname, sym) == 0) {
            result = tab[index];
            break;
        }
        index++;
    }
    return index;
}

int checkspecial(char* symbol) {
  int index = 0;
  while(symbol[index] != '\0') {
    if(symbol[index] ==  ('@') || symbol[index] ==  ('(') || symbol[index] == (')')  || symbol[index] == ('$') ||
        symbol[index] == ('=') ||  symbol[index] == ('+') || symbol[index] == ('-') || symbol[index] == ('!')) {
      return 0;
    }
    index++;
  }

  return 1;
}
