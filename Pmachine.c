/* Brandon Couts */
/* Cameron Yeager */
/* System software */

/* Simple Virtual Machine */

/*  Professor's website */
/* www.cs.ucf.edu/~eurip */

#include <stdio.h>  // input/output
#include <string.h> // string comparitors
#include <stdlib.h> // atoi : str to int
#include <time.h>   // log entry time

#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3

#define DEBUG 1
#define USING_FILE 1
#define TESTFILE "test.txt"

struct instruction {
    int OP;
    int L;
    int M;
};

struct pointers {
    int SP;
    int BP;
    int PC;
    struct instruction IR;
};

/* data sttuctures */

struct instruction code[MAX_CODE_LENGTH];
int memory[MAX_STACK_HEIGHT];
struct pointers CPU;
char OPcodes[11][4] = {"END\0", "LIT\0", "OPR\0", "LOD\0", "STO\0", "CAL\0", "INC\0", "JMP\0", "JPC\0", "WRT\0", "RIN\0"};

/* Memory flags */

int memoryLoadedFlag;
int is_running;
int end_of_prog;

/* Global variables */
char * filename;

/* Setup functions - Data initialize, menu system, CPU main, and code loader */

void menu();                //functional
void start ();              //functional - Start is the main for the CPU Functions
void load (char * name);    //functional
void reset ();              //functional

/* Print functions */

void printCode();                               // functional
void printOutCode();                            // functional 
void printStack(FILE *file, int sp, int bp);    // functional

/* CPU functions - execution cycle */
void step(int num);
void fetchCycle();
void executeCycle();

int main () {
    memoryLoadedFlag = 0;
    is_running = 0;
    
    if (USING_FILE == 1)
        load(TESTFILE);

    char input[100] = " ";
    char * result = NULL;
    char delims[] = " ";

    menu();

    int num = 0;

    while(1) {
        gets(input);

        /* Tokenizer */
        result = strtok( input, delims );
        if ( result != NULL ) 
            if ( strcmp( result, "run") == 0 ){
                start();
            } else if ( strcmp( result, "step") == 0 ) {
                result = strtok( NULL, delims );
                if (result != NULL) {
                    num = atoi(result);
                    step(num);
                } else {
                    step(1);
                } 
            } else if ( strcmp( result, "menu") == 0 ) 
                menu();
            else if ( strcmp( result, "reset") == 0 ) 
                reset();

            else if ( strcmp( result, "load") == 0 ) {
                result = strtok( NULL, delims );
                if (result == NULL) {
                    printf("  Enter a filename.\n");
                    gets(input);
                    load(input);
                } else {
//                    printf("%s",result);
                    load(result);
                }
            } else if ( strcmp( result, "print") == 0 ) {
                result = strtok( NULL, delims );
                if (result != NULL) {
                    if ( strcmp(result, "code") == 0)
                        printCode();
                } else {
                    printf("  code\n");
                    gets(input);
                    if ( strcmp(input, "code") == 0)
                        printCode();
                }
            } else if ( strcmp( result, "exit") == 0 )
                return 0;
    }
    return 0;
}

void menu () {
    printf("  run\n");
    printf("  step <steps>\n");
    printf("  menu\n");
    printf("  reset\n");
    printf("  load <file>\n");
    printf("  exit\n");
}


void load (char * name) {    
    int i;
    for (i = 0; i < MAX_CODE_LENGTH; i++) {
        code[i].OP = 0;
        code[i].L  = 0;
        code[i].M  = 0;
    }

    int counter = 0;

    if(name != NULL) {
        filename = name;
        FILE *file;
        file = fopen(name,"r");

        if (file == NULL)
            printf("File could not be opened.\n");
        else {
            while (!feof(file)) {
                fscanf(file, "%d", &code[counter].OP );
                fscanf(file, "%d", &code[counter].L );
                fscanf(file, "%d", &code[counter].M );
                //printf("%d %d %d\n", code[counter].OP, code[counter].L, code[counter].M);
                counter++;
            }
            memoryLoadedFlag = 1;
        }
        fclose(file);
    } else {
        printf("No file given.\n");
    }
    //printCode();
    if(USING_FILE != 1)
        printf("  done\n");
    printOutCode();
}

void printOutCode() {
    FILE *file;
    file = fopen("code.txt","w");
    int i = 0;
    while (i < MAX_CODE_LENGTH && code[i].OP != 0) {
        fprintf(file, "%d\t%s %d %d\n", i, OPcodes[code[i].OP], code[i].L, code[i].M);
        i++;
    }
    fclose(file);
}

void printCode() {
    int i;
    printf("  Printing code from memory:\n");
    for (i = 0; i < MAX_CODE_LENGTH; i++) {
        if (!((code[i].OP == 0) && (code[i].L == 0) && (code[i].M == 0)))
            printf("  %d: %d %d %d\n", i, code[i].OP, code[i].L, code[i].M);
    }
}

void printStack(FILE *file, int sp, int bp) {
    int bracket = 0;
    if(sp != -1) {
        if (sp == bp - 1 && sp != 0) {
            bp = memory[bp];
            bracket = 1;
        }
        printStack(file, sp - 1, bp);
        if (bracket == 1)
            fprintf(file, " |");
        fprintf(file, " %d", memory[sp]);
    }
    if(CPU.SP == 0)
        fprintf(file, " %d", memory[0]);
}

void printLog(char ab) {
    if(DEBUG == 1) {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer [80];
        
        FILE *file;
        file = fopen("log.txt","a");
        
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer, 80, " %X %p %A %x", timeinfo);

        if (is_running == 0 && end_of_prog == 0) {
            is_running = 1;
            fprintf(file,"\n%s - %s\n\n",filename,buffer);
            fprintf(file,"\t\t\t\tPC\tBP\tSP\tSTACK\nPC\tcode\tL\tM");
            fprintf(file,"\t%d\t%d\t%d",CPU.PC,CPU.BP,CPU.SP);
            fprintf(file,"\t");
            printStack(file, CPU.SP - 1, CPU.BP);
            fprintf(file,"\n");
        }
        if (ab == 'a') {
            fprintf(file,"%d\t%s\t%d\t%d\t",CPU.PC,OPcodes[CPU.IR.OP],CPU.IR.L,CPU.IR.M);
        }
        if (ab == 'b') {
            fprintf(file,"%d\t%d\t%d",CPU.PC,CPU.BP,CPU.SP);
            fprintf(file,"\t");
            printStack(file, CPU.SP - 1, CPU.BP);
            fprintf(file,"\n");
        }
        fclose(file);
    }
}

void reset () {
    CPU.SP = 0;
    CPU.BP = 1;
    CPU.PC = 0;
    CPU.IR.OP = 0;
    CPU.IR.L = 0;
    CPU.IR.M = 0;

    int i;

    for(i = 0; i < MAX_CODE_LENGTH; i++)
        memory[i] = 0;

    end_of_prog = 0;
}

void start () {
    end_of_prog = 0;
    reset();
    if (memoryLoadedFlag == 1){
        /* CPU CYCLE */
        while ( end_of_prog == 0 ){
            fetchCycle();
            executeCycle();
            printLog('b');
        }
    } else {
        printf("  Program not loaded.\n");
    }
    is_running = 0;
}

void step(int num) {
    int end_of_prog = 0;
    int stepper = 0;
    reset();
    if (memoryLoadedFlag == 1){
        /* CPU CYCLE */
        while ( end_of_prog != 1 && stepper < num){
            fetchCycle();
            executeCycle();
            stepper++;
        }
    } else {
        printf("  Program not loaded.\n");
    }
    printf("  Stepped %d\n",num);
}

void fetchCycle() {
    CPU.IR.OP = code[CPU.PC].OP;
    CPU.IR.L  = code[CPU.PC].L;
    CPU.IR.M  = code[CPU.PC].M;
    printLog('a');
    CPU.PC = CPU.PC + 1;
}

//possibly doesnt work, check with habitat for humanity
int base(l, basepointer) // l stand for L in the instruction format
{
  while (l > 0) {
//    printf("SL: %d",memory[basepointer - 1]);
    basepointer = memory[basepointer - 1];
//    printf("newBP: %d\n",basepointer);
    l--;
  }
  return basepointer;
}

void executeCycle() {
    /* END - when op code is zero the program is finished */
    if(CPU.IR.OP == 0) {
        end_of_prog = 1;
        
    /* LIT 0, M - Push constant value (literal) M onto the stack */
    //Stack Pointer increased by 1, and the M is copied into that spot
    } else if(CPU.IR.OP == 1) {
        CPU.SP = CPU.SP + 1; 
		memory[CPU.SP - 1] = CPU.IR.M; 
        
    /* OPR 0, M - Operation to be performed on the data at the top of the stack */
    } else if(CPU.IR.OP == 2) {
        /* 0  RET (sp <- bp -1 and pc <- stack[sp + 3] and bp <- stack[sp + 2]) */
        if (CPU.IR.M == 0) {
            CPU.PC = memory[CPU.BP + 1];
            CPU.SP = CPU.BP - 1;
            CPU.BP = memory[CPU.BP];
            if(CPU.BP == 0) 
                end_of_prog = 1;    
        /* 1  NEG (-stack[sp]) */
        }else if (CPU.IR.M == 1) {  
            memory[CPU.SP - 1] = (0 - memory[CPU.SP - 1]);
        /* 2  ADD (sp <- sp – 1 and  stack[sp] <- stack[sp] + stack[sp + 1]) */
        }else if (CPU.IR.M == 2) {
            CPU.SP = (CPU.SP - 1);
            memory[CPU.SP - 1] = (memory[CPU.SP - 1] + memory[CPU.SP]);
        /* 3  SUB (sp <- sp – 1 and  stack[sp] <- stack[sp] - stack[sp + 1]) */
        }else if (CPU.IR.M == 3) {
            CPU.SP = (CPU.SP - 1);
            memory[CPU.SP - 1] = (memory[CPU.SP - 1] - memory[CPU.SP]);
        /* 4  MUL (sp <- sp – 1 and  stack[sp] <- stack[sp] * stack[sp + 1]) */
        }else if (CPU.IR.M == 4) {
            CPU.SP = (CPU.SP - 1);
            memory[CPU.SP - 1] = (memory[CPU.SP - 1] * memory[CPU.SP]);
        /* 5  DIV (sp <- sp – 1 and stack[sp] <- stack[sp] / stack[sp + 1]) */
        }else if (CPU.IR.M == 5) {
            CPU.SP = (CPU.SP - 1);
            memory[CPU.SP - 1] = (memory[CPU.SP - 1] / memory[CPU.SP]);
        /* 6  ODD (stack[sp] <- stack[sp] mod 2) or ord(odd(stack[sp])) */
        }else if (CPU.IR.M == 6) {
            memory[CPU.SP] = (memory[CPU.SP]%2);
        /* 7  MOD (sp <- sp – 1 and  stack[sp] <- stack[sp] mod stack[sp + 1]) */
        }else if (CPU.IR.M == 7) {
            CPU.SP = (CPU.SP - 1);
            memory[CPU.SP - 1] = (memory[CPU.SP - 1] % memory[CPU.SP]);
        /* 8  EQL (sp <- sp – 1 and  stack[sp]  <- stack[sp] = = stack[sp + 1]) */
        }else if (CPU.IR.M == 8) {
            CPU.SP = (CPU.SP - 1);
            if (memory[CPU.SP - 1] == memory[CPU.SP])
                memory[CPU.SP - 1] = 1;
            else
                memory[CPU.SP - 1] = 0; 
        /* 9  NEQ (sp <- sp – 1 and  stack[sp]  <- stack[sp] != stack[sp + 1]) */
        }else if (CPU.IR.M == 9) {
            CPU.SP = (CPU.SP - 1);
            if(memory[CPU.SP - 1] != memory[CPU.SP])
                memory[CPU.SP - 1] = 1;
            else
                memory[CPU.SP - 1] = 0;
        /* 10 LSS (sp <- sp – 1 and  stack[sp] <-  stack[sp]  <  stack[sp + 1]) */
        }else if (CPU.IR.M == 10) {
            CPU.SP = (CPU.SP - 1);
            if(memory[CPU.SP - 1] < memory[CPU.SP])
                memory[CPU.SP - 1] = 1;
            else
                memory[CPU.SP - 1] = 0;
        /* 11 LEQ (sp <- sp – 1 and  stack[sp]  <- stack[sp] <=  stack[sp + 1]) */
        }else if (CPU.IR.M == 11) {
            CPU.SP = (CPU.SP - 1);
            if(memory[CPU.SP - 1] <= memory[CPU.SP])
                memory[CPU.SP - 1] = 1;
            else
                memory[CPU.SP - 1] = 0;
        /* 12 GTR (sp <- sp – 1 and  stack[sp]  <- stack[sp] >  stack[sp + 1]) */
        }else if (CPU.IR.M == 12) {
            CPU.SP = (CPU.SP - 1);
            if(memory[CPU.SP - 1] > memory[CPU.SP])
                memory[CPU.SP - 1] = 1;
            else
                memory[CPU.SP - 1] = 0;
        /* 13 GEQ (sp <- sp – 1 and  stack[sp]  <- stack[sp] >= stack[sp + 1]) */
        }else if (CPU.IR.M == 13) {
            CPU.SP = (CPU.SP - 1);
            if(memory[CPU.SP - 1] >= memory[CPU.SP])
                memory[CPU.SP - 1] = 1;
            else
                memory[CPU.SP - 1] = 0;
        }       
        
    /* LOD L, M - Load value to top of stack from the stack location at offset M from L lexicographical levels down */
    //Stack Pointer is increased by 1, and whatever is in spot M from L levels down gets copied
    } else if(CPU.IR.OP == 3) {
        CPU.SP = CPU.SP + 1; 
		memory[CPU.SP - 1] = memory[ base(CPU.IR.L, CPU.BP) + CPU.IR.M - 1];
        
    /* STO L, M - Store value at top of stack in the stack location at offset M from L lexicographical levels down */
    //Take the value from the top of the stack, and store it in spot M, L levels down
    } else if(CPU.IR.OP == 4) {
        memory[base(CPU.IR.L, CPU.BP) + CPU.IR.M - 1] = memory[CPU.SP - 1]; 
		CPU.SP = CPU.SP - 1;

    /* CAL L, M - Call procedure at code index M (generates new Activation Record and pc <- M) */
    //
    } else if(CPU.IR.OP == 5) {
        if (CPU.IR.L == 0)
            memory[CPU.SP    ] = 1; 	/* copy into the static link (SL)*/
        else
            memory[CPU.SP    ] = CPU.IR.L; 	/* copy into the static link (SL)*/ 
        memory[CPU.SP + 1] = CPU.BP;		/* copy into the dynamic link (DL)*/
	    memory[CPU.SP + 2] = CPU.PC;	 	/* copy into the return address (RA) */
        CPU.BP = CPU.SP + 1;            //move the base pointer of the AR
        CPU.PC = CPU.IR.M;                //Set the PC
        CPU.SP = CPU.BP + 2;

    /* INC 0, M - Allocate M locals (increment sp by M). 
    First three are Static Link(SL), Dynamic Link (DL), and Return Address (RA) */
    } else if(CPU.IR.OP == 6) {
        CPU.SP = CPU.BP + CPU.IR.M - 1;
        
    /* JMP 0, M - Jump to instruction M */
    } else if(CPU.IR.OP == 7) {
        CPU.PC = CPU.IR.M;
    /* JPC 0, M - Jump to instruction M if top stack element is 0 */
    } else if(CPU.IR.OP == 8) {
        if(memory[CPU.SP - 1] == 0){ 
            CPU.PC = CPU.IR.M; 
		}
		CPU.SP = CPU.SP - 1;
    /* WRT 0, 0 - Write the top stack element to the screen */
    } else if(CPU.IR.OP == 9) {
        printf("%d\n", memory[CPU.SP - 1]);
		CPU.SP = CPU.SP - 1;

    /* RIN 0, 0 - Read in input from the user and store it at the top of the stack */
    } else if(CPU.IR.OP == 10) {
        CPU.SP = CPU.SP + 1;
        scanf("%d\n", memory[CPU.SP - 1]);

    } else {
        printf("  Run time error: Invalid OP code\n");
        end_of_prog = 1;
    }
}
