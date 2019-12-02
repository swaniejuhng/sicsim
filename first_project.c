#include "20150959.h"

void help_func(void) {
	printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp] [start, end] \
	\ne[dit] address, value\nf[ill] start, end, value\nreset \
	\nopcode mnemonic\nopcodelist\nassemble filename\ntype filename\nsymbol\
	\nprogaddr [address]\nloader [filename1] [filename2] [filename3]\nrun\nbp [address|clear]\n");
	return;
}

int dir_func(void) {
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	struct stat status;

	if((dir = opendir("."))) {
		while((entry = readdir(dir))) {
			if(stat(entry->d_name, &status) < 0)
				perror("stat() : ");

			if(S_ISDIR(status.st_mode))
				printf("%s/\t", entry->d_name);

			else if(S_ISREG(status.st_mode)) {
				if((status.st_mode & S_IEXEC) != 0)
					printf("%s*\t", entry->d_name);
				else
					printf("%s\t", entry->d_name);
			}
		}
	}
	printf("\n");

	closedir(dir);
	return 0;
}

void check_history_num_func(void) {
	if (!historyhead)	historynum = 0;

	else {
		historytemp = historyhead;
		while(historytemp->next) {
			historytemp = historytemp->next;
			historynum = historytemp->idx;
		}
	}

	return;
}

void add_history_func(char* fullcmd) {
	if(!historyhead) {
		historyhead = (HISTORYLIST *)malloc(sizeof(HISTORYLIST));
		historyhead->idx = ++historynum;
		strcpy(historyhead->cmdline, fullcmd);
		historyhead->next = NULL;
	}

	else {
		historytemp = historyhead;
		while(historytemp->next)
			historytemp = historytemp->next;
		historynewnode = (HISTORYLIST *)malloc(sizeof(HISTORYLIST));
		historynewnode->idx = historynum + 1;
		strcpy(historynewnode->cmdline, fullcmd);
		historynewnode->next = NULL;
		historytemp->next = historynewnode;
	}

	return;
}

void show_history_func(void) {
	if(historyhead) {
		historytemp = historyhead;
		while(historytemp->next) {
			printf("%d\t%s\n", historytemp->idx, historytemp->cmdline);
			historytemp = historytemp->next;
		}
		printf("%d\t%s\n", historytemp->idx, historytemp->cmdline);
	}

	return;
}

void dump_func(int start, int end) {
	int linenum = 0, i, j, startblank, startaddr, endaddr;

	startblank = start % 16;
	startaddr = (start / 16) * 16;
	endaddr = (end / 16) * 16;

	for(i = startaddr; i <= endaddr; i += 16)
		linenum++;


	for(i = 0; i < linenum; i++) {
		if(startaddr != endaddr) {    // multiple lines to print
			if (i == 0) {    // first line
				printf("%.5X ", startaddr);                 // address
				for(j = 0; j < startblank; j++)            // blank
					printf("   ");

				for(j = start; j < startaddr + 16; j++)    // hex values
					printf("%02X ", memory[j]);
				printf(" ; ");

				for(j = 0; j < startblank; j++)            // blank values
					printf(".");

				for(j = start; j < startaddr + 16; j++) {  // ASCII values
					if (memory[j] >= 0x20 && memory[j] <= 0x7E)
						printf("%c", memory[j]);
					else
                        printf(".");
				}
				printf("\n");
			}

			else if (i == linenum - 1) { // last line
				printf("%.5X ", startaddr + i * 16);        // address

				for(j = startaddr + i * 16; j <= end; j++) // hex values
					printf("%02X ", memory[j]);

				for(j = end + 1; j < startaddr + (i + 1) * 16; j++)
					printf("   ");                          // blank

				printf(" ; ");

				for(j = startaddr + i * 16; j <= end; j++) {
					if (memory[j] >= 0x20 && memory[j] <= 0x7E)
						printf("%c", memory[j]);          // ASCII values
					else
						printf(".");
				}

				for(j = end + 1; j < startaddr + (i + 1) * 16; j++)
					printf(".");                            // blank values

				printf("\n");
			}

			else {
				printf("%.5X ", startaddr + i * 16);        // address

				for(j = startaddr + i * 16; j < startaddr + (i + 1) * 16; j++)
					printf("%02X ", memory[j]);           // hex values
				printf(" ; ");

				for(j = startaddr + i * 16; j < startaddr + (i + 1) * 16; j++) {
					if (memory[j] >= 0x20 && memory[j] <= 0x7E)
						printf("%c", memory[j]);          // ASCII values
					else
						printf(".");
				}
				printf("\n");
			}
		}

		else {        // single line to print
			printf("%.5X ", startaddr);      // address
			for(j = 0; j < startblank; j++)
				printf("   ");               // blank

			for(j = start; j <= end; j++)   // hex values
				printf("%02X ", memory[j]);

			for(j = end + 1; j < startaddr + (i + 1) * 16; j++)
				printf("   ");               // blank

			printf(" ; ");

			for(j = 0; j < startblank; j++)
				printf(".");

			for(j = start; j <= end; j++) {
				if (memory[j] >= 0x20 && memory[j] <= 0x7E)
					printf("%c", memory[j]); // ASCII values
				else
					printf(".");
			}

			for(j = end + 1; j < startaddr + (i + 1) * 16; j++)
				printf(".");                    // blank values

			printf("\n");
		}
	}

	return;
}

void edit_func(int address, int value) {
	memory[address] = value;
	return;
}

void fill_func(int start, int end, int value) {
    memset(&memory[start], value, end - start + 1);
	return;
}

void reset_func(void) {
    memset(memory, 0, MEGABYTE);
	return;
}

int read_opcode_func(void) {
    int opcode;
    char *temp1, *temp2, *temp3;
    char oneline[30], *result;

	FILE *fp = fopen("opcode.txt", "r");
	if(!fp) {
		printf("Failed to open opcode.txt.\n");
		return -1;
	}

    while((result = fgets(oneline, 30, fp))) {
        temp1 = strtok(oneline, " \t");
        temp2 = strtok(NULL, " \t");
        temp3 = strtok(NULL, " \t");
        opcode = (int)strtol(temp1, NULL, 16);
        add_hash_func(opcode, temp2, temp3);
    }

    fclose(fp);
    return 0;
}


void add_hash_func(int opcode, char *mnemonic, char *format) {
	int key = 0, i;
	for(i = 0; i < strlen(mnemonic); i++)
		key += (int)mnemonic[i];
	key %= HASHTABLENUM;

	while(1) {
		if(!hashtable[key]) {
			hashtable[key] = (HASH*)malloc(sizeof(HASH));
			hashtable[key]->opcode = opcode;
			strcpy(hashtable[key]->mnemonic, mnemonic);
		  	strcpy(hashtable[key]->format, format);
		  	hashtable[key]->next = NULL;
	  	}

	  	else {
		  	hashtemp = hashtable[key];
		  	while(hashtemp->next)
			  	hashtemp = hashtemp->next;
		  	hashnewnode = (HASH*)malloc(sizeof(HASH));
		  	hashnewnode->opcode = opcode;
		  	strcpy(hashnewnode->mnemonic, mnemonic);
		  	strcpy(hashnewnode->format, format);
		  	hashnewnode->next = NULL;
		  	hashtemp->next = hashnewnode;
	  	}

	  	break;
  	}

	return;
}

void opcode_list_func(void) {
	int i;
	hashtemp = NULL;

 	for(i = 0; i < HASHTABLENUM; i++) {
		hashtemp = hashtable[i];
		printf("%d : ", i);

		while(hashtemp) {
			if(!hashtemp->next)
				printf("[%s,%X]", hashtemp->mnemonic, hashtemp->opcode);
			else
				printf("[%s,%X] -> ", hashtemp->mnemonic, hashtemp->opcode);

			hashtemp = hashtemp->next;
 		}
		printf("\n");
	}

	return;
}

int find_opcode_func(char *mnemonic, int option) {
	int i, key = 0, found = 0;
	hashtemp = NULL;

	for(i = 0; i < strlen(mnemonic); i++)
		key += (int)mnemonic[i];
	key %= HASHTABLENUM;

	hashtemp = hashtable[key];
	while(hashtemp) {
		if(!strcmp(mnemonic, hashtemp->mnemonic)) {
			found = 1;
			if(option == 1)
				printf("Opcode is %X.\n", hashtemp->opcode);
			else if(option == 2)
				return hashtemp->opcode;
			break;
		}

		hashtemp = hashtemp->next;
	}

	if(option == 1 && found == 0)
		printf("Enter the correct mnemonic.\n");

	if(option == 1) return found;
	else			return -1;
}
