#include "headers.h"
#define INST_MAX 59

int mrcount = 0;
int trcount = 0;
struct symbol init; //this is for seeing what the start symbol is
int objbase; //need to find the base at each moment
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
	
  while(!feof(fp)) {
    fgets(line, 1024, fp);
    // printf("%s", line);
    arrayCopy(line, lclone);

		//good manners to close your file with fclose
		//read line by line until the end of the file
		if (line[0] == 46) { // period, . aka comment
      linecount++;
      continue;
    }
    if ((line[0] >= 97) && (line[0] <= 122)) { // lowercase letter
      printLine(lclone);
      printf("Line %d ERROR: Symbol cannot start with a lowercase letter! \n", linecount);
      fclose(fp);
      return 0;
    }
		if ((line[0] >= 65) && ( line[0]<= 90 )) { // capital letter
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

        if((*start >= 0x10000)) {
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
      if (strcmp("END", newdir) == 0) { // hit end of file
        break;
      }
      if (addCtr(inst, newdir, tok3, start, lclone, linecount) == 0) {
        fclose(fp);
        return 0;
      }

    } else if((line[0] == '\t') || line[0] == ' ' ) { // non-symbol assembly line
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
      if ((line[0] >= 65) && (line[0] <= 90)) {   //capital A-Z
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
    }   //end while
   printTable(symTab);
    sprintf(header, "H%s\t %06X%07X", init.name, init.address, prglen);

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
  strcpy(tab[1].iname, "ADDF");tab[1].opcode = 0x58;
  strcpy(tab[2].iname, "ADDR"); tab[2].opcode = 0x90;
  strcpy(tab[3].iname, "AND"); tab[3].opcode = 0x40;
  strcpy(tab[4].iname, "CLEAR"); tab[4].opcode = 0xB4;
  strcpy(tab[5].iname, "COMP"); tab[5].opcode = 0x28;
  strcpy(tab[6].iname, "COMPF"); tab[6].opcode = 0x88;
  strcpy(tab[7].iname, "COMPR"); tab[7].opcode = 0xA0;
  strcpy(tab[8].iname, "DIV"); tab[8].opcode = 0x24;
  strcpy(tab[9].iname, "DIVF"); tab[9].opcode = 0x64;
  strcpy(tab[10].iname, "DIVR"); tab[10].opcode = 0x9C;
  strcpy(tab[11].iname, "FIX"); tab[11].opcode = 0xC4;
  strcpy(tab[12].iname, "FLOAT"); tab[12].opcode = 0xC0;
  strcpy(tab[13].iname, "HIO"); tab[13].opcode = 0xF4;
  strcpy(tab[14].iname, "J"); tab[14].opcode = 0x3C;
  strcpy(tab[15].iname, "JEQ"); tab[15].opcode = 0x30;
  strcpy(tab[16].iname, "JGT"); tab[16].opcode = 0x34;
  strcpy(tab[17].iname, "JLT"); tab[17].opcode = 0x38;
  strcpy(tab[18].iname, "JSUB"); tab[18].opcode = 0x48;
  strcpy(tab[19].iname, "LDA"); tab[19].opcode = 0x00;
  strcpy(tab[20].iname, "LDB"); tab[20].opcode = 0x68;
  strcpy(tab[21].iname, "LDCH"); tab[21].opcode = 0x50;
  strcpy(tab[22].iname, "LDF"); tab[22].opcode = 0x70;
  strcpy(tab[23].iname, "LDL"); tab[23].opcode = 0x08;
  strcpy(tab[24].iname, "LDS"); tab[24].opcode = 0x6C;
  strcpy(tab[25].iname, "LDT"); tab[25].opcode = 0x74;
  strcpy(tab[26].iname, "LDX");tab[26].opcode = 0x04;
  strcpy(tab[27].iname, "LPS");tab[27].opcode = 0xD0;
  strcpy(tab[28].iname, "MUL"); tab[28].opcode = 0x20;
  strcpy(tab[29].iname, "MULF"); tab[29].opcode = 0x60;
  strcpy(tab[30].iname, "MULR"); tab[30].opcode = 0x98;
  strcpy(tab[31].iname, "NORM"); tab[31].opcode = 0xC8;
  strcpy(tab[32].iname, "OR"); tab[32].opcode = 0x44;
  strcpy(tab[33].iname, "RD"); tab[33].opcode = 0xD8;
  strcpy(tab[34].iname, "RMO"); tab[34].opcode = 0xAC;
  strcpy(tab[35].iname, "RSUB"); tab[35].opcode = 0x4C;
  strcpy(tab[36].iname, "SHIFTL"); tab[36].opcode = 0xA4;
  strcpy(tab[37].iname, "SHIFTR"); tab[37].opcode = 0xA8;
  strcpy(tab[38].iname, "SIO"); tab[38].opcode = 0xF0;
  strcpy(tab[39].iname, "SSK"); tab[39].opcode = 0xEC;
  strcpy(tab[40].iname, "STA"); tab[40].opcode = 0x0C;
  strcpy(tab[41].iname, "STBR"); tab[41].opcode = 0x78;
  strcpy(tab[42].iname, "STCH"); tab[42].opcode = 0x54;
  strcpy(tab[43].iname, "STF"); tab[43].opcode = 0x80;
  strcpy(tab[44].iname, "STI"); tab[44].opcode = 0xD4;
  strcpy(tab[45].iname, "STL"); tab[45].opcode = 0x14;
  strcpy(tab[46].iname, "STS"); tab[46].opcode = 0x7C;
  strcpy(tab[47].iname, "STSW"); tab[47].opcode = 0xE8;
  strcpy(tab[48].iname, "STT"); tab[48].opcode = 0x84;
  strcpy(tab[49].iname, "STX"); tab[49].opcode = 0x10;
  strcpy(tab[50].iname, "SUB"); tab[50].opcode = 0x1C;
  strcpy(tab[51].iname, "SUBF"); tab[51].opcode = 0x5C;
  strcpy(tab[52].iname, "SUBR");tab[52].opcode = 0x94;
  strcpy(tab[53].iname, "SVC"); tab[53].opcode = 0xB0;
  strcpy(tab[54].iname, "TD"); tab[54].opcode = 0xE0;
  strcpy(tab[55].iname, "TIO"); tab[55].opcode = 0xF8;
  strcpy(tab[56].iname, "TIX"); tab[56].opcode = 0x2C;
  strcpy(tab[57].iname, "TIXR"); tab[57].opcode = 0xB8;
  strcpy(tab[58].iname, "WD"); tab[58].opcode = 0xDC;
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
  int addrInc = 3;
  
  // check if there is a special notation for XE
  // Notes:
  // + indicates the instruction uses the expanded Format 4 (4 bytes)
  // + is found at the beginning of the instruction
  // @ indicates Indirect Addressing, found at the beginning of the operand
  // ,X (Present in Vanilla SIC) indicates indexed addressing, found at the end of the operand
  // # indicates Immediate Addressing, found at the beginning of the operand
  // Additionally, some instructions, notable the register-register ones,
  // only use 2 bytes, so a check for them must be necessary

  if (symbol[0] == '+') { // instruction is using Format 4
    addrInc = 4;
    symbol++;
  }
  if (strcmp(symbol, "ADDR") == 0 || // instruction uses format 2
      strcmp(symbol, "CLEAR") == 0 ||
      strcmp(symbol, "COMPR") == 0 ||
      strcmp(symbol, "DIVR") == 0 ||
      strcmp(symbol, "MULR") == 0 ||
      strcmp(symbol, "RMO") == 0 ||
      strcmp(symbol, "SHIFTL") == 0 ||
      strcmp(symbol, "SHIFTR") == 0 ||
      strcmp(symbol, "SUBR") == 0 ||
      strcmp(symbol, "SVC") == 0 ||
      strcmp(symbol, "TIXR") == 0) {
    addrInc = 2;
  } else if (
    strcmp(symbol, "FIX") == 0 ||
    strcmp(symbol, "FLOAT") == 0 ||
    strcmp(symbol, "HIO") == 0 ||
    strcmp(symbol, "NORM") == 0 ||
    strcmp(symbol, "SIO") == 0 ||
    strcmp(symbol, "TIO") == 0
    ) {
      addrInc = 1;
  }
  if (instructionExists(tab, symbol) != 0) {
    if (*ctr + addrInc <= 0x10000) {
      *ctr += addrInc;
      return 1;
    } else {
      printLine(line);
      printf("Line %d ERROR: SIC program has exceeded maximum allowed SIC memory size!\n", srcline);
      return 0;
    }
  } else if (strcmp("RESB", symbol) == 0) {
    sscanf(tok3, "%d", &temp);
    
    if (*ctr + temp <= 0x10000) {
      *ctr += (temp);
    } else {
      printLine(line);
      printf("Line %d ERROR: SIC program has exceeded maximum allowed SIC memory size!\n", srcline);
      return 0;
    }
  } else if (strcmp("RESW", symbol) == 0) {
    sscanf(tok3, "%d", &temp);

    if (*ctr + 3 * temp <= 0x10000) {
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
    if (*ctr + 3 * 8 <= 0x10000) {
      *ctr += 3 * 8;
      return 1;
    } else {
      printLine(line);
      printf("Line %d ERROR: SIC program has exceeded maximum allowed SIC memory size!\n", srcline);
      return 0;
    }
  } else if (strcmp("WORD", symbol) == 0) {
    temp = strtol(tok3, NULL, 10);
    if ((*ctr +(3) <= 0x10000) &&
       ((temp <= 8388608) && (temp >= -8388608))) {
      *ctr += 3;
      return 1;
    } else {
      printLine(line);
      printf("Line %d ERROR: Word being stored is larger than SIC max! Maximum size for SIC is 8388608 in either direction!\n", srcline);
      return 0;
    }
  } else if (strcmp("END", symbol) == 0) {
    if (*ctr + 3 <= 0x10000) {
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

  bool isFour = false;
  short nibitadd = 3;
  int symaddress = 0;
  int xbpe = 0;
  int signedthreeaddr = 0;
  unsigned int threeaddr = 0;
  bool base = false;
  bool isConstant = false;
  if(strcmp(first, "BYTE") == 0) {
    if(second[0] == 'X') { //hexadecimal, initiate hex parsing procedure
      strtok(second, "'");
      tempstring = strtok(NULL, "'");
     
      sscanf(tempstring, "%X", &hextemp);
      opcode = strlen(tempstring);
     
      sprintf(finalstring, "T %06X%02X%06X", locctr,opcode, hextemp );
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
        sprintf(finalstring, "T %06X%02X", locctr, memused);
        
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
    sprintf(finalstring, "T %06X03%06X", locctr, word);
    strcpy(trec[trcount], finalstring);
    trcount++;
    return 1;
  }

  if(first[0] == 44)
    {
        isFour = true;
        strtok(first, "+");
    }

  opcode = returnOpcode(inst, first);

  if(strcmp(first, "RSUB") == 0) {
    sprintf(finalstring,"T %06X03%02X0000", locctr, opcode );
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

      sprintf(finalstring,"T %06X03%02X%04X", locctr, opcode, temp->address);
      
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
      
      sprintf(finalstring,"T %06X03%02X%04X", locctr, opcode, hextemp);
      strcpy(trec[trcount], finalstring);
      
      trcount++;
      strcpy(mrec[mrcount], generateMrec(locctr));
      
      return 1;
    } else {
      printLine(error);
      printf("Line %d ERROR: Invalid addressing mode!\n", srcline);
      return 0;
    }
  }
  else if (opcode == -1) {
    printLine(error);
    printf("Line %d ERROR: Could not find instruction!\n", srcline);
    return 0;
  }
      /*
         *  This else implements the T record for the SIC/XE version of an instruction command.
         *  Checklist - X for incomplete, Y for complete (not necessarily in order of how they should be implemented in code
         *      Y > Implement a mechanism for determining whether to use base or PC addressing
         *      y > Implement a mechanism for determining the wanted addressing mode using +, @, and #
         *      y > Remove the symbol used by the code at the start
         *          could possibly be implemented by recording the symbol first, then doing strtok(symbol, "@#")
         *      Y/X > Implement the various ways of implementing the flag bits + various ways of addressing
         *          flag bits are 3 bytes
         *          the various ways seem to be identical in set up, not sure if we have to do anything different to the counter part
         *          or it's just a matter of making sure the flag bits are set up correctly
         *          (only thing missing from implementing this is the ,X addressing modes)
         *      X > Float data types
         *      (might be missing some things so this list isn't comprehensive!)
         *      X > update other data types (may not need to be updated, unsure)
         */
  else {
       //ni are = 3, first two bits are set to 1
      if(strcmp("LDB", first)) //check for changes to base
      {
          objbase = locctr;
      }
      if (second[0] == 64) //#
      {
          nibitadd = 2;
          strtok(second, "@");
      } else if (second[0] == 35) {
          nibitadd = 1;
          strtok(second, "#");
      }

      temp = symbolReturn(tab, second);


      if(isFour== true)
      {
          opcode += 1;
          sprintf(finalstring,"T %06X 04 %02X %01X %05X", locctr, opcode, xbpe, temp->address); //locctr (6 hex) -> object code length (4 bytes, 2 hex) -> opcode(2 hex(with ni bits clipped into it)
          strcpy(trec[trcount], finalstring);                                       //(xbpe) -> 1 hex (address) -> 5 hex characters
          strcpy(mrec[mrcount], generateMrec(locctr));
      }
      //invalid symbol = we have a constant or an actual invalid symbol
      if (temp == NULL) {
          if (second[0] == 48 && second[1] == '\n') {
              threeaddr = 0; //test if the constant is a zero
              isConstant = true;
          }
          else {
              threeaddr = atoi(second); //atoi will return 0 if this is an actually invalid symbol
              isConstant = true;
              if (threeaddr == 0) {
                  printLine(error);
                  printf("Line %d ERROR: Symbol not found!\n", srcline);
                  return 0;
              }
          }
      }
      else {
      symaddress = temp->address;
      base = temp->usesBase; //store base in here so we're not calling a null symbol
           }
        if(isConstant == true)
        {
            opcode += nibitadd;
            xbpe += 0;
            sprintf(finalstring,"T %06X 03 %1X %04X", locctr, opcode, xbpe, threeaddr); //location counter -> object code length (3 bytes) -> opcode (ni bits clip into it)
                                                                                        //xbpe = 1 byte
        }
        else if ((locctr < symaddress) && ((symaddress - locctr) <= 0x2047) && (base == false) && (isConstant == false)) //positive 2047 PC relative
        {
            threeaddr = symaddress - locctr;
            opcode += nibitadd;
            xbpe += 2;
            sprintf(finalstring,"T %06X 03 %1X %04X", locctr, opcode, xbpe, threeaddr); //location counter -> object code length (3 bytes) -> opcode (ni bits clip into it)
                                                                                    //xbpe = 1 byte
        }
        else if ((locctr > symaddress) && ((locctr - symaddress) <= 0x2048) && (base == false) && (isConstant == false)) //more than -2047 PC relative backwards
        {
            threeaddr = locctr - symaddress;
            threeaddr = (~threeaddr) + 1;
            threeaddr = threeaddr & 0x0000FFFF; //AND makes sure that we only have the bits we need, everything else stays the same
            opcode += nibitadd;
            xbpe += 4;
            sprintf(finalstring,"T %06X 03 %1X %04X", locctr, opcode, xbpe, threeaddr); //location counter -> object code length (3 bytes) -> opcode (ni bits clip into it)
                                                                                        //xbpe = 1 byte
        }
        else if((((objbase - init.address) >= 0) && ((objbase - init.address) <= 4095 )))//base relative
        {
            threeaddr = objbase - init.address;
            opcode += nibitadd;
            xbpe += 1;
            sprintf(finalstring,"T %06X 03 %02X %04X", locctr, opcode, xbpe, threeaddr); //location counter -> object code length (3 bytes) -> opcode (ni bits clip into it)
        }
    //consider format 4 error
        else{
            printLine(error);
            printf("Line %d ERROR: Assembler could not resolve this into PC-Relative or Base-relative addressing, consider using Format 4.\n", srcline);
            return 0;
    }
    if (temp == NULL) {
      printLine(error);
      printf("Line %d ERROR: Symbol not found!\n", srcline);
      return 0;
    }
    
    sprintf(finalstring,"T %06X03%02X%04X", locctr, opcode, temp->address);

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
