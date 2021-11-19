#include "headers.h"
#include "symbols.c"
#define INST_MAX 26

int mrcount = 0;
int trcount = 0;
struct symbol init; //this is for seeing what the start symbol is

int generateTrec(char* first, char* second, struct symbol* tab[], unsigned int locctr, char error[], int srcline,
                 char trec[][71], char mrec[][71], struct syminst inst[]);
int createFile(FILE* fp, char trec[][71], char mrec[][71], char* header, char* end);
char* generateMrec(int ctr);
void initInstructions(struct syminst tab[]);
int returnOpcode(struct syminst insttab[], char *sname);
int validHex(char* string);
int addCtr(struct syminst tab[], char* symbol, char* tok3, unsigned int* ctr, char* line, int srcline);
void printTable(struct symbol* tab[]);
void printLine(char* line);
void arrayCopy(char line[], char clone[]);

int main( int argc, char* argv[]) {
  struct syminst inst[INST_MAX];
  initInstructions(inst);
	struct symbol* symTab[1024];
  struct countertrack* pcount[1024];
  unsigned int prev;
  char trecord[1024][71] = {};
  char mrecord[1024][71] = {};
  for(int e = 0; e < 1024; e++) {
    pcount[e] = malloc(sizeof(struct countertrack));
  }
  
  memset(symTab, '\0', 1024 * sizeof(struct symbol*));
	FILE *fp;
	char line[1024];
  char lclone[1024];
	char* newsym;
	char* newdir;
	char* tok3;
	unsigned int* start;

  start = malloc(sizeof(unsigned int));
  int linecount = 1;

	if(argc  != 2) {
		printf("ERROR: Usage: %s filename\n", argv[0]);
		return 0;
  }

	fp = fopen( argv[1], "r");
	if (fp == NULL ) {
	  printf("ERROR: %s could not be opened for reading,\n", argv[1] );
	  return 0;
  }
	newsym = malloc( 1024 * sizeof(char));
  newdir = malloc( 1024 * sizeof(char));
  tok3 = malloc( 1024 * sizeof(char));
	
  //fill this up with nulls
	//use memset
	memset( newsym, '\0', 1024 * sizeof(char));
  memset( newdir, '\0', 1024 * sizeof(char));
  memset( tok3, '\0', 1024 * sizeof(char));
  int casenum = 0;
	
  while(fgets(line, 1024, fp) != NULL) {
    arrayCopy(line, lclone);

		//good manners to close your file with fclose
		//read line by line until the end of the file
		if (line[0] == 35) {
      linecount++;
      continue;
    }
    if ((line[0] >= 97) && (line[0] <= 122)) {
      printLine(lclone);
      printf("Line %d ERROR: Symbol cannot start with a lowercase letter! \n", linecount);
      fclose(fp);
      return 0;
    }
		if( (line[0] >= 65) && ( line[0]<= 90 )) { // this lets us know the line is between A and capital Z.
      newsym = strtok( line, " \t\n\r");
      newdir = strtok( NULL, " \t\n\r");
      tok3 = strtok( NULL, "\t\n");
      casenum = IsAValidSymbol(newsym, symTab);
      switch(casenum) {
          case 1:
            break;
          case 2:
            printLine(lclone);
            printf("ERROR. Symbol name cannot be longer than 6!\n");
            fclose(fp);
            return 0;
          case 3:
            printLine(lclone);
            printf("Line %d ERROR: Symbol name cannot have the same name as an assembler directive! \n", linecount);
            fclose(fp);
            return 0;
          case 4:
            printLine(lclone);
            printf("Line %d ERROR: Symbol name cannot contain the characters $,!, =, +, -, (, ), or @! \n", linecount);
            fclose(fp);
            return 0;
          case 5:
            printLine(lclone);
            printf("Line %d ERROR: Symbol cannot exist more than once in the symbol table! \n", linecount);
            fclose(fp);
            return 0;
          default:
            break;
      }
      if (strcmp( "START", newdir) == 0) {
        sscanf(tok3, "%x", start);

        if((*start >= 0x8000)) {
          printLine(lclone);
          printf("Line %d ERROR: RAM starts at an amount equal to or greater than SIC memory! \n", linecount);
          fclose(fp);
          return 0;
        }
        pcount[linecount]->line = linecount;
        pcount[linecount]->counter = *start;
        addSymbol(symTab, start, linecount, newsym);
        linecount++;
        strcpy(init.name, newsym);
        init.address = *start;
        continue;
      }
      pcount[linecount]->line = linecount;
      pcount[linecount]->counter = *start;
      prev = *start;
      addSymbol(symTab, start, linecount, newsym);
      if (addCtr(inst, newdir, tok3, start, lclone, linecount) == 0) {
        fclose(fp);
        return 0;
      }

    } else if((line[0] == '\t') || line[0] == ' ' ) {
      newsym = strtok( line, " \t\n\r");
      newdir = strtok( NULL, " \t\n\r");

      if (strcmp( "START", newsym) == 0) {
       sscanf(tok3, "%x", start);
        pcount[linecount]->line = linecount;
        pcount[linecount]->counter = *start;
      }

      pcount[linecount]->line = linecount;
      pcount[linecount]->counter = *start;
      prev = *start;

      if(addCtr(inst, newsym, newdir, start, lclone, linecount) == 0) {
        fclose(fp);
        return 0;
      }
    } else if(line[0] == '\n' || line[0] == '\r') {
        printLine(lclone);
        printf("Line %d ERROR: The SIC specification does not allow empty lines! \n", linecount);
        fclose(fp);
        return 0;
    } else {
      printLine(lclone);
      printf("Line %d ERROR: First letter of symbol has an invalid letter! The only letters allowed are A-Z \n", linecount);
      fclose(fp);
      return 0;
    }
    
    linecount++;
  } // end while
  if (init.name[0] == '\0') {
    printf("ERROR: START directive cannot be found! \n");
    fclose(fp);
    return 0;
  }

  fclose(fp);

  fp = fopen( argv[1], "r");

  int linecount2 = 1;
  unsigned int prglen = prev - init.address;
  char* header;
  char* end;
  int ctratline;
  struct symbol* tempsym;

  tempsym = malloc(sizeof(struct symbol*));
  header = malloc(26 * sizeof(char));
  end = malloc(10 * sizeof(char));

  while(fgets(line, 1024, fp) != NULL) {
    arrayCopy(line, lclone);
    if (line[0] == 35) {
      linecount2++;
      continue;
    }
    if ((line[0] >= 65) && (line[0] <= 90)) { // capital A-Z
      newsym = strtok(line, " \t\n\r");
      newdir = strtok(NULL, " \t\n\r");
      tok3 = strtok(NULL, "\t\n");
      
      if (strcmp("RESB", newdir) == 0 ||
          strcmp("RESW", newdir) == 0 ||
          strcmp("START", newdir) == 0) {
        linecount2++;
        continue;
      }
      
      if (strcmp("END", newdir) == 0) {
        tempsym = symbolReturn(symTab, tok3);
        
        if(tempsym == NULL &&
           tok3 != NULL) {
          printLine(lclone);
          printf("ERROR: Symbol could not be found in symbol table!\n");
          fclose(fp);
          return 0;
        }
    
        sprintf(end, "E00%06X", tempsym->address);
        linecount2++;
        continue;
      }

      ctratline = pcount[linecount2]->counter;
      
      if (generateTrec(newdir, tok3, symTab, ctratline, lclone, linecount2, trecord, mrecord, inst) == 0) {
        fclose(fp);
        return 0;
      } else {
        linecount2++;
      }

    } else if (line[0] == '\t') {
      newsym = strtok(line, " \t\n\r");
      newdir = strtok(NULL, " \t\n\r");

      if (strcmp("RESB", newsym) == 0 ||
          strcmp("RESW", newsym) == 0 ||
          strcmp("START", newsym) == 0) {
        linecount2++;
        continue;
      }

      if(strcmp("END", newsym) == 0) {
        tempsym = symbolReturn(symTab, tok3);

        if(tempsym == NULL) {
          sprintf(end, "E%06X", init.address);
        } else {
          sprintf(end, "E%06X", tempsym->address);
        }

        linecount2++;
        continue;
      }

      ctratline = pcount[linecount2]->counter;

      if (generateTrec(newsym, newdir, symTab, ctratline, lclone, linecount2, trecord, mrecord, inst) == 0) {
        fclose(fp);
        return 0;
      } else {
        linecount2++;
      }
    }
  } // end while
  printTable(symTab);
  sprintf(header, "H%s\t00%04X%07X", init.name, init.address, prglen);

  fclose(fp);
  strcat(argv[1], ".obj");
  fopen(argv[1], "w");

  createFile(fp, trecord, mrecord, header, end);
  printf("Object file '%s' created.\n", argv[1]);
  fclose(fp);

	return 0;
}

// busywork to set up the opcode table.
// condensed lines for space
void initInstructions(struct syminst tab[]) {
  strcpy(tab[0].iname, "ADD");tab[0].opcode = 0x18;
  strcpy(tab[1].iname, "AND");tab[1].opcode = 0x40;
  strcpy(tab[2].iname, "DIV"); tab[2].opcode = 0x24;
  strcpy(tab[3].iname, "J"); tab[3].opcode = 0x3C;
  strcpy(tab[4].iname, "JEQ"); tab[4].opcode = 0x30;
  strcpy(tab[5].iname, "JGT"); tab[5].opcode = 0x34;
  strcpy(tab[6].iname, "JLT"); tab[6].opcode = 0x38;
  strcpy(tab[7].iname, "JSUB"); tab[7].opcode = 0x48;
  strcpy(tab[8].iname, "LDA"); tab[8].opcode = 0x00;
  strcpy(tab[9].iname, "LDCH"); tab[9].opcode = 0x50;
  strcpy(tab[10].iname, "LDL"); tab[10].opcode = 0x08;
  strcpy(tab[11].iname, "LDX"); tab[11].opcode = 0x04;
  strcpy(tab[12].iname, "MUL"); tab[12].opcode = 0x20;
  strcpy(tab[13].iname, "OR"); tab[13].opcode = 0x44;
  strcpy(tab[14].iname, "RD"); tab[14].opcode = 0xD8;
  strcpy(tab[15].iname, "RSUB"); tab[15].opcode = 0x4C;
  strcpy(tab[16].iname, "STA"); tab[16].opcode = 0x0C;
  strcpy(tab[17].iname, "STCH"); tab[17].opcode = 0x54;
  strcpy(tab[18].iname, "STL"); tab[18].opcode = 0x14;
  strcpy(tab[19].iname, "STSW"); tab[19].opcode = 0x84;
  strcpy(tab[20].iname, "STX"); tab[20].opcode = 0x10;
  strcpy(tab[21].iname, "SUB"); tab[21].opcode = 0x1C;
  strcpy(tab[22].iname, "WD"); tab[22].opcode = 0xDC;
  strcpy(tab[23].iname, "TIX"); tab[23].opcode = 0x2C;
  strcpy(tab[24].iname, "TD"); tab[24].opcode = 0xE0;
  strcpy(tab[25].iname, "COMP"); tab[25].opcode = 0x28;
}

int instructionExists(struct syminst insttab[], char *sname) {
  int result = 0;
  int index = 0;
  for (index = 0; index < INST_MAX; index ++) {
    if (strcmp(sname, insttab[index].iname) == 0) {
      result = -1;
      break;
    }
  }

  return result;
}

int addCtr(struct syminst tab[], char* symbol, char* tok3, unsigned int* ctr, char line[], int srcline) {
  int temp = 0;
  unsigned int hextemp = 0;
  char* constant;
  constant = malloc(1024 * sizeof(char));
  memset(constant, '\0', 1024 * sizeof(char));
  
  if (instructionExists(tab, symbol) != 0) {
    if (*ctr + 3 <= 0x8000) {
      *ctr += 3;
      return 1;
    } else {
      printLine(line);
      printf("Line %d ERROR: SIC program has exceeded maximum allowed SIC memory size!\n", srcline);
      return 0;
    }
  } else if (strcmp("RESB", symbol) == 0) {
    sscanf(tok3, "%d", &temp);
    
    if (*ctr + temp <= 0x8000) {
      *ctr += (temp);
    } else {
      printLine(line);
      printf("Line %d ERROR: SIC program has exceeded maximum allowed SIC memory size!\n", srcline);
      return 0;
    }
  } else if (strcmp("RESW", symbol) == 0) {
    sscanf(tok3, "%d", &temp);

    if (*ctr + 3 * temp <= 0x8000) {
      *ctr += 3*(temp);
    } else {
      printLine(line);
      printf("Line %d ERROR: SIC program has exceeded maximum allowed SIC memory size!\n", srcline);
      return 0;
    }
  } else if (strcmp("BYTE", symbol) == 0) {
    if (tok3[0] == 'X') { //hexadecimal, initiate hex parsing procedure
      strtok(tok3, "'");
      constant = strtok(NULL, "'");

      if (validHex(constant) == 0) {
        printLine(line);
        printf("Line %d ERROR: Invalid hex value found!\n", srcline);
        return 0;
      }
      sscanf(constant, "%X", &hextemp);
      
      if (strlen(constant) % 2 == 0) {
        *ctr += strlen(constant) / 2;
      } else {
        printLine(line);
        printf("Line %d ERROR: Odd hex values are not within SIC specification! Please pad your hex values with a 0.\n", srcline);
        return 0;
      }
    } else if (tok3[0] == 'C') { //character, initiate character parsing procedure
      strtok(tok3,"'");
      constant = strtok(NULL,"'");
      temp = strlen(constant);
      *ctr += temp;
    } else {
      printLine(line);
      printf("Line %d ERROR: Invalid constant type for BYTE.\n", srcline);
      return 0;
    }
  } else if (strcmp("EXPORTS", symbol) == 0 ||
             strcmp("RESR", symbol) == 0) {
    sscanf(tok3, "%x", &hextemp);
    if (*ctr + 3 * 8 <= 0x8000) {
      *ctr += 3 * 8;
      return 1;
    } else {
      printLine(line);
      printf("Line %d ERROR: SIC program has exceeded maximum allowed SIC memory size!\n", srcline);
      return 0;
    }
  } else if (strcmp("WORD", symbol) == 0) {
    temp = strtol(tok3, NULL, 10);
    if ((*ctr +(3) <= 0x8000) &&
       ((temp <= 8388608) && (temp >= -8388608))) {
      *ctr += 3;
      return 1;
    } else {
      printLine(line);
      printf("Line %d ERROR: Word being stored is larger than SIC max! Maximum size for SIC is 8388608 in either direction!\n", srcline);
      return 0;
    }
  } else if (strcmp("END", symbol) == 0) {
    if (*ctr + 3 <= 0x8000) {
      *ctr += 3;
      return 1;
    } else {
      printLine(line);
      printf("Line %d ERROR: SIC program has exceeded maximum allowed SIC memory size!\n", srcline);
      return 0;
    }
  } else {
    printLine(line);
    printf("Line %d ERROR: Invalid instruction or directive, if this file is for the SIC XE, find a different one.\n", srcline);
    return 0;
  }

  return 1;
}

void printTable(struct symbol* tab[]) {
  int index = 0;

  printf("LINE#   SYMBOL    ADDRESS\n");
  printf("-------------------------\n");

  while(tab[index] != NULL) {
    printf("%2d      %-7s     %4X\n", tab[index]->sourceLine, tab[index]->name, tab[index]->address);
    index++;
  }
}

void printLine(char* line) {
  printf("%s", line);
}

void arrayCopy(char line[], char clone[]) {
  for (int x = 0; x < 1024; x++) {
    clone[x] = line[x];
  }
}

int validHex(char* string) {
  int index = 0;
  while(string[index] != '\0') {
    if((string[index] >= 65 && string[index] <= 70) ||
       (string[index] >= 30 && string[index] <= 57)) { //checks if this is a valid hex number
      index++;
      continue;
    } else {
      return 0;
    }
  }

  return 1;
}

/**
 this function will generate a T record
 things to pass:
       instruction
       symbol table
       second word
       location counter
       trecord table
       mrecord table
*/
int generateTrec(char* first, char* second, struct symbol* tab[], unsigned int locctr, char error[], int srcline,
                 char trec[][71], char mrec[][71], struct syminst inst[])
{
  unsigned int hextemp = 0;
  int word = 0;
  int opcode = 0;

  struct symbol* temp;
  temp = malloc(sizeof(struct symbol*));

  char* tempstring;
  char* hexparse;

  hexparse = malloc(sizeof(char*));
  tempstring = malloc(sizeof(char*));

  char finalstring[71];
  char* sym;
  sym = malloc(sizeof(char*));
  
  char* indexing;
  indexing = malloc(sizeof(char*));

  if(strcmp(first, "BYTE") == 0) {
    if(second[0] == 'X') { //hexadecimal, initiate hex parsing procedure
      strtok(second, "'");
      tempstring = strtok(NULL, "'");
     
      sscanf(tempstring, "%X", &hextemp);
      opcode = strlen(tempstring);
     
      sprintf(finalstring, "T00%04X%02X%06X", locctr,opcode, hextemp );
      strcpy(trec[trcount], finalstring);
     
      trcount++;
     
      return 1;
    } else if(second[0] == 'C') { //character, initiate character parsing procedure
      strtok(second,"'");
      tempstring = strtok(NULL,"'");

      int bytelen, counter = 0, looplen, rem, memused, ind = 0;

      bytelen = strlen(tempstring);
      looplen = (bytelen + (30 - 1))/30;
      rem = bytelen % 30;
      memused = 0;

      for(int x = 1; x <= looplen; x++) { 
        if(x == looplen &&
           rem != 0) {
          memused = rem;
        } else {
          memused = 30;
        }
        sprintf(finalstring, "T00%04X%02X", locctr, memused);
        
        if(memused < 3) {
          if (memused == 2) {
            strcat(finalstring, "00");
          } else {
            strcat(finalstring, "0000");
          }
        }

        while((tempstring[ind] != '\0') &&
              (counter <= 29)) {
          sprintf(hexparse, "%02X", (unsigned int) tempstring[ind]);
          strcat(finalstring, hexparse);
          ind++;
          counter++;
        }

        strcpy(trec[trcount], finalstring);
        trcount++;
        locctr +=30;
        counter = 0;
      }
      return 1;
    }
  }
  if(strcmp(first, "WORD") == 0) {
    word = atoi(second);
    sprintf(finalstring, "T00%04X03%06X", locctr, word);
    strcpy(trec[trcount], finalstring);
    trcount++;
    return 1;
  }

  opcode = returnOpcode(inst, first);

  if(strcmp(first, "RSUB") == 0) {
    sprintf(finalstring,"T00%04X03%02X0000", locctr, opcode );
    strcpy(trec[trcount], finalstring);
    trcount++;
    strcpy(mrec[mrcount], generateMrec(locctr));
    return 1;
  }

  if (strcmp("STCH", first) == 0 ||
      strcmp("LDCH", first) == 0) {
    sym = strtok(second, " ,");
    indexing = strtok(NULL, " ,");

    if (indexing == NULL) {
      temp = symbolReturn(tab,second);
      
      if (temp == NULL) {
        printLine(error);
        printf("Line %d ERROR: Symbol not found!\n", srcline);
        return 0;
      }

      sprintf(finalstring,"T00%04X03%02X%04X", locctr, opcode, temp->address);
      
      strcpy(trec[trcount], finalstring);
      strcpy(mrec[mrcount], generateMrec(locctr));
      
      trcount++;
      
      return 1;
    } else if (*indexing == 'X') {
      temp = symbolReturn(tab,sym);
      
      if(temp == NULL) {
        return 1;
      }
      
      hextemp = temp->address;
      hextemp |= 1 << 15;
      
      sprintf(finalstring,"T00%04X03%02X%04X", locctr, opcode, hextemp);
      strcpy(trec[trcount], finalstring);
      
      trcount++;
      strcpy(mrec[mrcount], generateMrec(locctr));
      
      return 1;
    } else {
      printLine(error);
      printf("Line %d ERROR: Invalid addressing mode!\n", srcline);
      return 0;
    }
  } else if (opcode == -1) {
    printLine(error);
    printf("Line %d ERROR: Could not find instruction!\n", srcline);
    return 0;
  } else {
    temp = symbolReturn(tab,second);
    
    if (temp == NULL) {
      printLine(error);
      printf("Line %d ERROR: Symbol not found!\n", srcline);
      return 0;
    }
    
    sprintf(finalstring,"T00%04X03%02X%04X", locctr, opcode, temp->address);
    strcpy(trec[trcount], finalstring);
    strcpy(mrec[mrcount], generateMrec(locctr));
  }

  strcpy(trec[trcount], finalstring);
  trcount++;

  return 1;
}

//this finds the instruction, and returns the opcode or a -1
int returnOpcode(struct syminst insttab[], char *sname) {
  int index = 0;
  
  for (index = 0; index < INST_MAX; index ++) {
    if (strcmp(sname, insttab[index].iname) == 0) {
      return insttab[index].opcode;
    }
  }

  return -1;
}

//this function will generate a modification record
/**
 this function will generate a M record
 things to pass:
      * location counter
      * start name
*/
char* generateMrec(int ctr) {
  char* result;
  result = malloc(sizeof(char*));
  sprintf(result,"M%06X04+%s",ctr+1, init.name);
  mrcount++;

  return result;
}
int createFile(FILE* fp, char trec[][71], char mrec[][71], char* header, char* end) {
  fprintf(fp, "%s\n", header);
  int index = 0;
  
  while (index <= (trcount - 1)) {
    fprintf(fp, "%s\n", trec[index]);
    index++;
  }

  index = 1;
  while (index <= mrcount) {
    fprintf(fp, "%s\n", mrec[index]);
    index++;
  }

  fprintf(fp, "%s", end);
  return 0;
}
