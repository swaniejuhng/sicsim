#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <math.h>

#define MAX_CMD_LEN 100
#define DISPLAYSTART 0
#define DISPLAYEND 159
#define MEGABYTE 1048576
#define HASHTABLENUM 20
#define SYMBOLTABLENUM 30

/* error flags */
#define DUPLICATESYMBOL 11111
#define INVALIDOPCODE 22222
#define INAPPROPRIATEOPERAND 33333
#define UNDEFINEDSYMBOL 44444

int historynum;
int progaddr, curraddr, execaddr;
unsigned char memory[MEGABYTE];
int A, X, L, PC, B, S, T; // registers
int PCtemp;
short int breakpoint[MEGABYTE];
int bpnum;

typedef struct HISTORYLIST_ {
	int idx;
	char cmdline[MAX_CMD_LEN];
	struct HISTORYLIST_ *next;
} HISTORYLIST;
HISTORYLIST *historyhead, *historytemp, *historynewnode;

typedef struct HASH_ {
	int opcode;
	char mnemonic[10];
	char format[4];
	struct HASH_ *next;
} HASH;
HASH *hashtable[HASHTABLENUM], *hashhead, *hashtemp, *hashnewnode;

typedef struct SYMBOL_ {
	char symbol[10];
	int address;
	struct SYMBOL_ *next;
} SYMBOL;
SYMBOL *symboltable[SYMBOLTABLENUM], *symhead, *symtemp, *symnewnode;

typedef struct EXTSYM_ {
	char controlsection[7];
	char symbolname[7];
	char number[3];
	int address;
	int length;
	struct EXTSYM_ *next;
} EXTSYM;
EXTSYM *exthead, *exttemp, *extnewnode;

/*============== FIRST PROJECT ==============*/
void help_func(void);	// print out the list of commands
int dir_func(void);		//print out the list of directories and files in the current directory
void check_history_num_func(void);		// check the number of history list which will be used in add_history_func
void add_history_func(char* fullcmd);	// add the correct command to history list
void show_history_func(void);			// print out the history list
void dump_func(int start, int end);		// show the state of memory
void edit_func(int address, int value);	// edit the memory of given address with given value
void fill_func(int start, int end, int value);	// fill the memory within given address with given value
void reset_func(void);		// reset the memory
int read_opcode_func(void);	// generate the opcode table
void add_hash_func(int opcode, char *mnemonic, char *fm);	// add each mnemonic to the opcode table
void opcode_list_func(void);	// print out the list of opcodes
int find_opcode_func(char *mnemonic, int option);	// (option = 1) return the existence of mnemonic in opcode table
											// (option = 2) return the opcode of mnemonic

/*============= SECOND PROJECT =============*/
int type_func(char *filename);		// print out the content of the file
int key_for_symbol_func(char *sym);			// generate key to the symbol for the symbol table
void add_symbol_func(char *sym, int addr);		// add each symbol to the symbol table
int find_label_func(char *label, int option);	// (option = 1) return the existence of label in symbol table
												// (option = 2) return the address of label in symbol table
char* format_for_opcode_func(char *mnemonic);	// return the format of opcode in opcode table
void symbol_print_func(void);			// print out the list of symbols
int assemble_func(char *filename);	// read the .asm file and generate .lst file and .obj file

/*============== THIRD PROJECT =============*/
void progaddr_func(int address);	// assign the program start address
int loader_func(char *fname1, char *fname2, char *fname3);	// load up to three (sub)programs
int extdef_func(FILE *fp, int startaddr, char *progname, int drflag);	// save addresses for each extdef symbols
void extref_func(FILE *fp, char *progname, int drflag);		// save numbers of each extref symbols
void tm_record_func(FILE *fp, char *progname, int drflag);	// read T / M records and process them
void add_bp_func(int address);		// add breakpoints
void bp_print_func(void);			// print list of breakpoints
void bp_clear_func(void);			// clear out all the breakpoints
int find_mnemonic_func(int opcode, char *mnemonic);			// find mnemonic from an opcode
void run_func(int totallength, int *currbp, int *nextbp);	// run loaded program
