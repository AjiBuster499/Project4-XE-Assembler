#include "headers.h"
#define INST_MAX 59

int mrcount = 0;
int trcount = 0;
struct symbol init; //this is for seeing what the start symbol is
int objbase; //need to find the base at each moment
int generateTrec(char* first, char* second, struct symbol* tab[], unsigned int locctr, char error[], int srcline,
                 char trec[][71], char mrec[][71], struct syminst inst[], struct countertrack* tracker[]);
int createFile(FILE* fp, char trec[][71], char mrec[][71], char* header, char* end);
char* generateMrec(int ctr);
void initInstructions(struct syminst tab[]);
int returnOpcode(struct syminst insttab[], char *sname);
int validHex(char* string);
int addCtr(struct syminst tab[], char* symbol, char* tok3, unsigned int* ctr, char* line, int srcline);
void printTable(struct symbol* tab[]);
void printLine(char* line);
void arrayCopy(char line[], char clone[]);
int findRegister(char* reg);
char findChar(char* str);
int findNextValid(struct countertrack* tab[], int line);

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
      //had to readd # as comment bc the example program wouldn't compile, thought it was a symbol
		if (line[0] == 46 || line[0] == 35) { // period, . aka comment
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
      if(strcmp("BYTE", newdir) == 0) {
          tok3 = strtok(NULL, "\t\n\r");
      }
      else{
          tok3 = strtok(NULL, " \t\n\r");
      }
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
        pcount[linecount]->valid = true;
        addSymbol(symTab, start, linecount, newsym);
        linecount++;
        strcpy(init.name, newsym);
        init.address = *start;
        continue;
      }
      pcount[linecount]->line = linecount;
      pcount[linecount]->counter = *start;
      pcount[linecount]->valid = true;
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
      newdir = strtok( NULL, "\t\n\r");

      if (strcmp( "START", newsym) == 0) {
       sscanf(tok3, "%x", start);
        pcount[linecount]->line = linecount;
        pcount[linecount]->counter = *start;
        pcount[linecount]->valid = true;
      }

      pcount[linecount]->line = linecount;
      pcount[linecount]->counter = *start;
      pcount[linecount]->valid = true;
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
        if(strcmp("BYTE", newdir) == 0) {
            tok3 = strtok(NULL, "\t\n\r");
        }
        else{
            tok3 = strtok(NULL, " \t\n\r");
        }
        if (strcmp("RESB", newdir) == 0 ||
            strcmp("RESW", newdir) == 0 ||
            strcmp("START", newdir) == 0) {
          linecount2++;
          continue;
        }
      
        if (strcmp("END", newdir) == 0) {
            if(tok3 != NULL) {
                tempsym = symbolReturn(symTab, tok3);
            }
          if(tempsym == NULL &&
             tok3 != NULL) {
            printLine(lclone);
            printf("ERROR: Symbol could not be found in symbol table!\n");
            fclose(fp);
            return 0;
          }
          if(tok3 != NULL) {
              sprintf(end, "E00%06X", tempsym->address);
          }
          else{
              sprintf(end, "E00%06X", init.address);
          }
          linecount2++;
          continue;
        }
          if (strcmp("BASE", newdir) == 0)
          {
              int a = symbolIndex(symTab, tok3);
              symTab[a]->usesBase = true;
              linecount2++;
              continue;
          }
          else if (strcmp("NOBASE", newdir) == 0)
          {
              int a = symbolIndex(symTab, tok3);
              symTab[a]->usesBase = false;
              linecount2++;
              continue;
          }
        ctratline = findNextValid(pcount, linecount2);
      
        if (generateTrec(newdir, tok3, symTab, ctratline, lclone, linecount2, trecord, mrecord, inst, pcount) == 0) {
          fclose(fp);
          return 0;
        } else {
          linecount2++;
        }

      } else if (line[0] == '\t') {
        newsym = strtok(line, " \t\n\r");
        if(strcmp("BYTE", newsym) == 0)
        {
            newdir = strtok(NULL, "\t\n\r");
        }
        else
        {
              newdir = strtok(NULL, " \t\n\r");
        }
          if (strcmp("BASE", newsym) == 0)
          {
              int a = symbolIndex(symTab, newdir);
              symTab[a]->usesBase = true;
              linecount2++;
              continue;
          }
          else if (strcmp("NOBASE", newsym) == 0)
          {
              int a = symbolIndex(symTab, tok3);
              symTab[a]->usesBase = false;
              linecount2++;
              continue;
          }
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

        ctratline = findNextValid(pcount, linecount2);

        if (generateTrec(newsym, newdir, symTab, ctratline, lclone, linecount2, trecord, mrecord, inst, pcount) == 0) {
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
  for (index = 0; index < INST_MAX; index++) {
    if (strcmp(sname, insttab[index].iname) == 0) {
      result = -1;
      break;
    }
  }

  return result;
}
int findNextValid(struct countertrack* tab[], int line)
{
    line += 1;
    for(line; line < 1024 ; line++)
    {
        if(tab[line]->valid == true)
        {
            return tab[line]->counter;
        }
    }
    return -1;
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
    if (findChar(tok3) == 'X') { //hexadecimal, initiate hex parsing procedure
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
    } else if (findChar(tok3) == 'C') { //character, initiate character parsing procedure
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

      if ((strchr(tok3, '.') != NULL)) //float
      {
          if ((*ctr + (6) <= 0x10000) &&
              ((temp <= 8388608) && (temp >= -8388608))) {
              *ctr += 6;
              return 1;
          }
          temp = strtol(tok3, NULL, 10);
          } if ((*ctr + (3) <= 0x10000) &&
              ((temp <= 8388608) && (temp >= -8388608))) {
              *ctr += 3;
              return 1;
          } else {
              printLine(line);
              printf("Line %d ERROR: Word being stored is larger than SIC max! Maximum size for SIC is 8388608 in either direction!\n",
                     srcline);
              return 0;
          }


  }
  else if (strcmp("BASE", symbol) == 0)
  {
    return 1;
  }
  else if (strcmp("END", symbol) == 0) {
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
/**
 * Prints the symbol table out given a symbol table.
 */
void printTable(struct symbol* tab[]) {
  int index = 0;

  printf("LINE#   SYMBOL    ADDRESS\n");
  printf("-------------------------\n");

  while(tab[index] != NULL) {
    printf("%2d      %-7s     %4X\n", tab[index]->sourceLine, tab[index]->name, tab[index]->address);
    index++;
  }
}
/*
 * prints the line
 */
long genFloat(char* input)
{
    int whole;
    int dec;
    int deci;
    int digits = 1;

    int count = 0;
    int divisor =1;
    long sum = 0;

    int bias = 1023;
    int exponent;


    long output = 0;

    sscanf(input, "%d.%d", &whole, &deci);
    dec = deci;

    if(whole == 0 && deci == 0)
    {
        return 0;//this will return zero when the input is 0.0
    }
    //this calculates the number of digits of the whole number if it is more than 1
    if (dec >= 10)
    {
        do
        {
            dec = dec - (dec * pow(10,digits));
            ++digits;
        }while(dec > 0);
    }

    dec = deci;
    //this sets the first bit to 1 if the number is negative
    if(whole < 0)
    {
        output = output + 140737488355328;
        whole = whole * -1;
    }
    //calculates output for numbers with an absolute value that is >= 2
    if(whole > 1)
    {

        while(whole >= divisor)
        {
            count = count + 1;
            divisor = divisor * 2;

        }
        count = count - 1;
        exponent = bias + (count);
        output = output + (exponent * pow(2,36));
        output = output + ((whole - pow(2,count)) * pow(2, 36-(count)));//adds the remainder of the whole number to the 36bit section

        //calculates the value of the decimal
        for(int i = 35 - count; i >= 0; i--)
        {
            if(dec * 2 < pow(10,digits))
            {
                dec = dec * 2;
                continue;
            }
            if(dec * 2 > pow(10,digits))
            {
                dec = (dec *2) - pow(10,digits);
                sum = sum + pow(2,i);
                continue;
            }
            if(dec * 2 == pow(10,digits))
            {
                sum = sum + pow(2,i);
                break;
            }

        }
        output = output + sum;
    }
        //calculates output for numbers with an absolute value between 1 and 2
    else if(whole == 1)
    {
        output = output + (bias * pow(2,36));//sets the 11bit exponent to 1023

        //calculates the value of the decimal
        for(int i = 36; i > 0; i--)
        {
            if(dec * 2 < pow(10,digits))
            {
                dec = dec * 2;
                continue;
            }
            if(dec * 2 > pow(10,digits))
            {
                dec = (dec *2) - pow(10,digits);
                sum = sum + pow(2,i-1);
                continue;
            }
            if(dec * 2 == pow(10,digits))
            {
                sum = sum + pow(2,i-1);
                break;
            }
        }
        output = output + sum;
    }

        //calculates output for numbers that have an absolute value between 0 and 1
    else if(whole == 0)
    {
        for(int i = 36; i > 0; i--)//sets count which is the exponent
        {
            count = count + 1;
            if(dec * 2 < pow(10,digits))
            {
                dec = dec * 2;
                continue;
            }
            if(dec * 2 > pow(10,digits))
            {
                break;
            }
            if(dec * 2 == pow(10,digits))
            {

                break;
            }
        }
        output = output + ((bias - count) * pow(2,36));//adds 11bit exponent into the output
        dec = deci;

        //calculates the decimal number
        for(int i = 36; i > 0; i--)
        {
            if(dec * 2 < pow(10,digits))
            {

                dec = dec * 2;
                continue;
            }
            if(dec * 2 > pow(10,digits))
            {
                dec = (dec *2) - pow(10,digits);
                sum = sum + pow(2,i-1);
                continue;
            }
            if(dec * 2 == pow(10,digits))
            {
                dec = 0;
                sum = sum + pow(2,i-1);
                output = output + 34359738368;
                return output;
            }
        }
        sum = sum - pow(2,36 - count);//sets the first bit that is a one to a zero
        sum = sum * pow(2,count);//shifts the number to the left

        //continues filling in the remaining lower numbers that were not filled in due to the shift
        for(int i = count ; i > 0; i--)
        {
            if(dec * 2 < pow(10,digits))
            {
                dec = dec * 2;
                continue;
            }
            if(dec * 2 > pow(10,digits))
            {
                dec = (dec *2) - pow(10,digits);
                sum = sum + pow(2,i-1);
                continue;
            }
            if(dec * 2 == pow(10,digits))
            {
                dec = 0;
                sum = sum + pow(2,i-1);
                output = output + 34359738368;
                return output;
            }
        }
        output = output + sum;
    }

    return output;
}

void printLine(char* line) {
  printf("%s", line);
}

void arrayCopy(char line[], char clone[]) {
  for (int x = 0; x < 1024; x++) {
    clone[x] = line[x];
  }
}
/**
 * This function checks the validity of hex numbers to make sure they are real hex numbers.
 *
 */
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
                 char trec[][71], char mrec[][71], struct syminst inst[], struct countertrack* tracker[])
{
  unsigned int hextemp = 0;
  long word = 0;
  int opcode = 0;

  struct symbol* temp;
  temp = malloc(sizeof(struct symbol*));
  int curr = tracker[srcline]->counter; //different from locctr, this keeps track of the counter before the current instruction versus the counter with the
                                          //instruction
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
  bool xbitset = false;
  bool rsub = false;
  int r1 = 0;
  int r2 = 0;
  //opcode length 1
  char e;
  if(strcmp(first, "BYTE") == 0)
  {
      if(findChar(second) == 'X') { //hexadecimal, initiate hex parsing procedure
      strtok(second, "'");
      tempstring = strtok(NULL, "'");
     
      sscanf(tempstring, "%X", &hextemp);
      opcode = strlen(tempstring);
     
      sprintf(finalstring, "T%06X%02X%06X", curr, opcode, hextemp );
      strcpy(trec[trcount], finalstring);
     
      trcount++;
     
      return 1;
    } else if(findChar(second) == 'C') { //character, initiate character parsing procedure
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
        sprintf(finalstring, "T%06X%02X", curr, memused);
        
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
      if ((strchr(second, '.') != NULL)) //float
      {
          word = genFloat(second);
          sprintf(finalstring, "T%06X06%012lX", curr, word);
      }
      else {
          word = atoi(second);
          sprintf(finalstring, "T%06X03%06lX", curr, word);
      }
    strcpy(trec[trcount], finalstring);
    trcount++;
    return 1;
  }

  if(first[0] == 43)
    {
        isFour = true;
        first = strtok(first, "+");
    }
    opcode = returnOpcode(inst, first);
  if((strcmp(first, "FIX") == 0)|| (strcmp(first, "FLOAT") == 0) || (strcmp(first, "HIO") == 0) || (strcmp(first, "NORM") == 0) ||
          (strcmp(first, "SIO") == 0) || (strcmp(first, "TIO") == 0) )
    {
        sprintf(finalstring,"T%06X01%02X", curr, opcode);
        strcpy(trec[trcount], finalstring);
        trcount++;
    }
  //format 2
  /*
   *  r1, r2 are registers
   *    each register gets *one* byte
   *    byte length is 2
   *    **LIST OF REGISTERS**
   *    Register => Number
   *    A => 0
   *    X => 1
   *    L => 2
   *    B => 3
   *    S => 4
   *    T => 5
   *    F => 6
   *    PC => 8
   *    SW => 9
   */
  if((strcmp(first, "CLEAR") == 0) || (strcmp(first, "TIXR") == 0))
    {
        sym = strtok(second, " ,");
        //indexing = r2
        r1 = findRegister(sym);
        r2 = 0;
        if(r1 == -1)
        {
            printLine(error);
            printf("Line %d ERROR: Invalid register!\n", srcline);
            return 0;
        }
        sprintf(finalstring,"T%06X02%02X%01X%01X", curr, opcode, r1, r2);
        strcpy(trec[trcount], finalstring);
        trcount++;
        return 1;
    }
  if((strcmp(first,"SHIFTL") == 0) || strcmp(first, "SHIFTR") == 0)
    {
        //sym = r1
        sym = strtok(second, " ,");
        //indexing = r2
        r1 = findRegister(sym);
        indexing = strtok(NULL, " ,");
        // r2 = n in this case
        r2 = atoi(indexing);
        if((r1 == -1) || !((r2 >=0) && (r2 <= 15)))
        {
            printLine(error);
            printf("Line %d ERROR: Invalid register or bit shift length! The max bit shift length should be 15!\n", srcline);
            return 0;
        }
        sprintf(finalstring,"T%06X02%02X%01X%01X", curr, opcode, r1, r2);
        strcpy(trec[trcount], finalstring);
        trcount++;
        return 1;
    };
  if((strcmp(first, "ADDR") == 0)|| (strcmp(first, "DIVR") == 0 ) || (strcmp(first, "MULR") == 0)
     || (strcmp(first, "RMO") == 0) || (strcmp(first,"SUBR") == 0) || strcmp(first, "COMPR") == 0)
    {
        //sym = r1
        sym = strtok(second, " ,");
        //indexing = r2
        r1 = findRegister(sym);
        indexing = strtok(NULL, " ,");
        r2 = findRegister(indexing);
        if((r1 == -1) || (r2 == -1))
        {
            printLine(error);
            printf("Line %d ERROR: Invalid register!\n", srcline);
            return 0;
        }
        sprintf(finalstring,"T%06X02%02X%01X%01X", curr, opcode, r1, r2);
        strcpy(trec[trcount], finalstring);
        trcount++;
        return 1;
    }



  if (opcode == -1) {
    printLine(error);
    printf("Line %d ERROR: Could not find instruction!\n", srcline);
    return 0;
  }
      /*
         *  This else implements the T record for the SIC/XE version of an instruction command.
         *  Checklist - X for incomplete, Y for complete (not necessarily in order of how they should be implemented in code
         *  (might be missing some things so this list isn't comprehensive!)
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
         *
         *      X > update other data types (may not need to be updated, unsure)
         *      X > implement instructions that have a different word count (specifically instructions that require 1/2 word counts)
         */
  else {
      //ni are = 3, first two bits are set to 1
      if (strcmp("RSUB", first) == 0) {
          rsub = true;
      }

      if (rsub == false) {
          if (second[0] == 64) //@
          {
              nibitadd = 2;
              second = strtok(second, "@");
          } else if (second[0] == 35) {
              nibitadd = 1;
              second = strtok(second, "#");
          }
      }
      if (strcmp("STCH", first) == 0 ||
          strcmp("LDCH", first) == 0)
        {
            sym = strtok(second, " ,");
            indexing = strtok(NULL, " ,");
            temp = symbolReturn(tab, sym);
            if (*indexing == 'X')
            {
                xbitset = true;
            }
            else if (indexing != NULL)
            {
                printLine(error);
                printf("Line %d ERROR: Invalid addressing mode!\n", srcline);
                return 0;
            }

        }
      else if (rsub == false) {
          temp = symbolReturn(tab, second);
      }


      //invalid symbol = we have a constant or an actual invalid symbol
      if (temp == NULL && rsub == false) {
          if (strcmp("0", second) == 0) {
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
      else if (rsub == false){
          symaddress = temp->address;
      base = temp->usesBase; //store base in here so we're not calling a null symbol
           }
      if(strcmp("LDB", first) == 0) //check for changes to base
      {
          int newbase;
          if(isConstant == false)
          {
              newbase = temp->address;
          }
          else{
              newbase = threeaddr;
          }
          objbase = newbase;
      }
      if(isFour== true)
      {
          if(isConstant == false){
              threeaddr = temp->address;
          }
          opcode += nibitadd;;
          if (xbitset == true)
          {
              xbpe += 8;
          }
          if(rsub == true)
          {
              threeaddr = 0;
          }
          xbpe += 1; //rightmost bit set to 1
          sprintf(finalstring,"T%06X04%02X%01X%05X", curr, opcode, xbpe, threeaddr); //curr (6 hex) -> object code length (4 bytes, 2 hex) -> opcode(2 hex(with ni bits clipped into it)
          strcpy(trec[trcount], finalstring);
          if(isConstant == false) {
              trcount++;//(xbpe) -> 1 hex (address) -> 5 hex characters
              strcpy(mrec[mrcount], generateMrec(curr));
          }
          return 1;
      }
        if(isConstant == true)
        {
            opcode += nibitadd;
            xbpe += 0;
            if (xbitset == true)
            {
                xbpe += 8;
            }
            if(rsub == true)
            {
                threeaddr = 0;
            }
            sprintf(finalstring,"T%06X03%02X%1X%03X", curr, opcode, xbpe, threeaddr); //location counter -> object code length (3 bytes) -> opcode (ni bits clip into it)
                                                                                               //xbpe = 1 byte
            strcpy(trec[trcount], finalstring);
            trcount++;
        }
        else if (((locctr < symaddress) && (((symaddress - locctr) <= 2047 ) && ((symaddress - locctr) >= 0)) && (base == false) && (isConstant == false)) || (rsub == true )) //positive 2047 PC relative
        {
            threeaddr = symaddress - locctr;
            opcode += nibitadd;
            xbpe += 2; //second to last bit turned on
            if (xbitset == true)
            {
                xbpe += 8;
            }
            if(rsub == true)
            {
                threeaddr = 0;
                xbpe = 0;
            }
            sprintf(finalstring,"T%06X03%02X%1X%03X", curr, opcode, xbpe, threeaddr); //location counter -> object code length (3 bytes) -> opcode (ni bits clip into it)
                                                                                    //xbpe = 1 byte
            strcpy(trec[trcount], finalstring);
            trcount++;
        }
        else if ((locctr > symaddress) && (((locctr - symaddress) > 0 ) && ((locctr - symaddress) <= 2048)) && (base == false) && (isConstant == false)) //more than -2047 PC relative backwards
        {
            threeaddr = curr - symaddress;
            threeaddr = (~threeaddr) + 1; // twos complement
            threeaddr = threeaddr & 0x00000FFF; //AND makes sure that we only have the bits we need, everything else stays the same
            opcode += nibitadd;
            xbpe += 2;
            if (xbitset == true)
            {
                xbpe += 8;
            }
            if(rsub == true)
            {
                threeaddr = 0;
            }
            sprintf(finalstring,"T%06X03%02X%1X%03X", curr, opcode, xbpe, threeaddr);
            //location counter -> object code length (3 bytes) -> opcode (ni bits clip into it)
            //xbpe = 1 byte
            strcpy(trec[trcount], finalstring);
            trcount++;
        }
        else if((objbase <= symaddress) && ((symaddress - objbase <= 4095 ) && ((symaddress - objbase) >= 0)))//base relative
        {
            //base address is less than the address of the requested symbol + symbol is in the range of 4096 and 0
            threeaddr = symaddress - objbase;
            opcode += nibitadd; //add to the opcode
            xbpe += 4;  // +4 for format four object code
            if (xbitset == true)
            {
                xbpe += 8;
            }
            if(rsub == true)
            {
                threeaddr = 0;
            }
            sprintf(finalstring,"T%06X03%02X%01X%03X", curr, opcode, xbpe, threeaddr); //location counter -> object code length (3 bytes) -> opcode (ni bits clip into it)
            strcpy(trec[trcount], finalstring);
            trcount++;
        }
    //consider format 4 error
        else{
            printLine(error);
            printf("Line %d ERROR: Assembler could not resolve this into PC-Relative or Base-relative addressing, consider using Format 4.\n", srcline);
            return 0;

    }


  }

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

//this function will find the register number based on the given letter
//format 2
/*
 *  r1, r2 are registers
 *    each register gets *one* byte
 *    byte length is 2
 *    **LIST OF REGISTERS**
 *    Register => Number
 *    A => 0
 *    X => 1
 *    L => 2
 *    B => 3
 *    S => 4
 *    T => 5
 *    F => 6
 *    PC => 8
 *    SW => 9
 */
int findRegister(char* reg){
    if (reg[1] != '\0')
    {
        //either invalid or PC/SW
        if(strcmp(reg, "PC") == 0)
        {
            return 8;
        }
        else if(strcmp(reg,"SW") == 0)
        {
            return 9;
        }
        return -1;
    }
    else if(reg[0] == 'A')
    {
        return 0;
    }
    else if(reg[0] == 'X')
    {
        return 1;
    }
    else if(reg[0] == 'L')
    {
        return 2;
    }
    else if(reg[0] == 'B')
    {
        return 3;
    }
    else if(reg[0] == 'B')
    {
        return 3;
    }
    else if(reg[0] == 'S')
    {
        return 4;
    }
    else if(reg[0] == 'T')
    {
        return 5;
    }
    else if(reg[0] == 'F')
    {
        return 6;
    }
    else
    {
        //invalid one letter register
        return -1;
    }

};
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
  sprintf(result,"M%06X05+%s",ctr+1, init.name);
  mrcount++;

  return result;
}
//find a character so we don't deal with leading white spaces
char findChar(char* str)
{
    int count = 0;
    while (count <= strlen(str)) {
        if(str[count] == ' ' || str[count] == '\t')
        {
            count++;
            continue;
        }
        else {
            return str[count];
        }
    }
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
