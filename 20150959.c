#include "20150959.h"

int main(void) {
	int i, length, argnum = 0, totallength = 0, currbp = -1, nextbp = -1;
	int wrongcmdflag = 0, hexproblemflag = 0, dumptrialnum = 0;
	int hex1 = MEGABYTE, hex2 = MEGABYTE, hex3 = MEGABYTE;
	int startpoint, endpoint;
	char fullcmd[MAX_CMD_LEN], temp[MAX_CMD_LEN], *maincmd, *arg1, *arg2, *arg3, *arg4;

//--------------------------------INITIALIZE MEMORY SPACE AND VARIABLES----------------------------------------
	reset_func();
	read_opcode_func();
	add_symbol_func("A", 0);	add_symbol_func("X", 1);	add_symbol_func("L", 2);
	add_symbol_func("PC", 8);	add_symbol_func("SW", 9);	add_symbol_func("B", 3);
	add_symbol_func("S", 4);	add_symbol_func("T", 5);	add_symbol_func("F", 6);
	A = 0; X  = 0; L = 0xFFFFFF; PC = 0; B = 0; S = 0; T = 0; PCtemp = -1;
	progaddr = 0; curraddr = 0; execaddr = 0;
	memset(breakpoint, 0, MEGABYTE); bpnum = 0;
	exthead = exttemp = extnewnode = NULL;

	while(1) {
		argnum = 0;
		wrongcmdflag = 0;
		hexproblemflag = 0;

		while(1) {
			printf("sicsim> ");

			/*  initialize string variables  */
			memset(fullcmd, 0, MAX_CMD_LEN);
			memset(temp, 0, MAX_CMD_LEN);
			arg1 = arg2 = arg3 = arg4 = NULL;

//------------------------------------------READ COMMAND LINE--------------------------------------------------

			fgets(fullcmd, MAX_CMD_LEN, stdin);
			length = strlen(fullcmd);
			fullcmd[--length] = '\0';
			strcpy(temp, fullcmd);

			/*  if the user wrote nothing  */
			if(length == 0)		break;

//----------------------------------SEPARATE THE COMMAND LINE INTO PARTS--------------------------------------
			maincmd = strtok(temp, " \t");
			arg1 = strtok(NULL, " \t,");
			arg2 = strtok(NULL, " \t,");
			arg3 = strtok(NULL, " \t,");
			arg4 = strtok(NULL, " \t,");

			/*  decide if the main command is an existing one  */
			if(strcmp(maincmd, "h") && strcmp(maincmd, "help")
			&& strcmp(maincmd, "f") && strcmp(maincmd, "fill")
			&& strcmp(maincmd, "d") && strcmp(maincmd, "dir")
			&& strcmp(maincmd, "q") && strcmp(maincmd, "quit")
			&& strcmp(maincmd, "hi") && strcmp(maincmd, "history")
			&& strcmp(maincmd, "du") && strcmp(maincmd, "dump")
			&& strcmp(maincmd, "e") && strcmp(maincmd, "edit")
			&& strcmp(maincmd, "f") && strcmp(maincmd, "fill")
			&& strcmp(maincmd, "reset")
			&& strcmp(maincmd, "opcode")
			&& strcmp(maincmd, "opcodelist")
			&& strcmp(maincmd, "assemble")
			&& strcmp(maincmd, "type")
			&& strcmp(maincmd, "symbol")
			&& strcmp(maincmd, "progaddr")
			&& strcmp(maincmd, "loader")
			&& strcmp(maincmd, "run")
			&& strcmp(maincmd, "bp")) {
				wrongcmdflag = 1;
				break;
			}

			if(!maincmd)		break;
			else if(!arg1)		argnum = 0;
			else if(!arg2)		argnum = 1;
			else if(!arg3)		argnum = 2;
			else if(!arg4)		argnum = 3;
			else /* argnum == 4 */	{	wrongcmdflag = 1;	break;	}


//----------------------------------HANDLE EXCEPTIONS CONCERNING COMMAS---------------------------------------
			/*  if there is a space before the main command  */
			if(!strcmp(maincmd, "loader")) {
				for(i = 0; i < length; i++)
					if(fullcmd[i] == ',') {	wrongcmdflag = 1; break; }
				if(wrongcmdflag)	break;
			}

			else {
				for(i = 0; i < length; i++)
					if(fullcmd[i] != ' ' && fullcmd[i] != '\t')
						break;

				/* there should not be a comma between maincmd and arg1  */
				if(argnum >= 0) {
					for(i += strlen(maincmd); i < length; i++) {
						if(fullcmd[i] != ' ' && fullcmd[i] != '\t') {
							if(fullcmd[i] == ',') {
								wrongcmdflag = 1;
								break;
							}
							else	break;	// start of arg1
						}
					}
					if(wrongcmdflag) break;
				}

				/*  there should be a single comma between arg1 and arg2  */
				if(argnum >= 1) {
					if((i += strlen(arg1)) < length) wrongcmdflag = 1;
					for( ; i < length; i++) {
						if(fullcmd[i] != ' ' && fullcmd[i] != '\t') {
							if(fullcmd[i] == ',') {
								if(wrongcmdflag)
									wrongcmdflag = 0;
								else { // more than one comma
									wrongcmdflag = 1;
									break;
								}
							}
							else	break;	// start of arg2
						}
					}
					if(wrongcmdflag)	break;
				}

				/*  there should be a single comma between arg2 and arg3  */
				if(argnum >= 2) {
					if((i += strlen(arg2)) < length) wrongcmdflag = 1;
					for( ; i < length; i++) {
						if(fullcmd[i] != ' ' && fullcmd[i] != '\t') {
							if(fullcmd[i] == ',') {
								if(wrongcmdflag)
									wrongcmdflag = 0;
								else { // more than one comma
									wrongcmdflag = 1;
									break;
								}
							}
							else	break;	// start of arg3
						}
					}
					if(wrongcmdflag)	break;
				}

				/*  fullcmd should not end with comma  */
				for(i = length - 1; i >= 0; i--) {
					if(fullcmd[i] != ' ' && fullcmd[i] != '\t') {
						if(fullcmd[i] == ',') {
							wrongcmdflag = 1;
							break;
						}
						else	break;
					}
				}
				if(wrongcmdflag)	break;
			}


			/*  decide if the adequate number of arguments is given for the main command  */
			if(argnum == 0) {
				if(!strcmp(maincmd, "e") || !strcmp(maincmd, "edit")
				|| !strcmp(maincmd, "f") || !strcmp(maincmd, "fill")
				|| !strcmp(maincmd, "opcode")
				|| !strcmp(maincmd, "assemble")
				|| !strcmp(maincmd, "type")
				|| !strcmp(maincmd, "loader")) {
					wrongcmdflag = 1;
					break;
				}
			}

			else {	// argnum = 1, 2, 3
				/*  these commands are not supposed to have arguments  */
				if(!strcmp(maincmd, "h") || !strcmp(maincmd, "help")
				|| !strcmp(maincmd, "d") || !strcmp(maincmd, "dir")
				|| !strcmp(maincmd, "q") || !strcmp(maincmd, "quit")
				|| !strcmp(maincmd, "hi") || !strcmp(maincmd, "history")
				|| !strcmp(maincmd, "reset")
				|| !strcmp(maincmd, "opcodelist")
				|| !strcmp(maincmd, "symbol")
				|| !strcmp(maincmd, "run")) {
					wrongcmdflag = 1;
					break;
				}

				else if(((!strcmp(maincmd, "du") || !strcmp(maincmd, "dump")) && argnum > 2)
				|| ((!strcmp(maincmd, "e") || !strcmp(maincmd, "edit")) && argnum != 2)
				|| ((!strcmp(maincmd, "f") || !strcmp(maincmd, "fill")) && argnum != 3)
				|| (!strcmp(maincmd, "opcode") && argnum != 1)
				|| (!strcmp(maincmd, "assemble") && argnum != 1)
				|| (!strcmp(maincmd, "type") && argnum != 1)
				|| (!strcmp(maincmd, "progaddr") && argnum != 1)
				|| (!strcmp(maincmd, "bp") && argnum > 1)) {
					wrongcmdflag = 1;
					break;
				}

				if(!strcmp(maincmd, "du") || !strcmp(maincmd, "dump")
				|| !strcmp(maincmd, "e") || !strcmp(maincmd, "edit")
				|| !strcmp(maincmd, "f") || !strcmp(maincmd, "fill")
				|| !strcmp(maincmd, "progaddr")
				|| (!strcmp(maincmd, "bp") && (strcmp(arg1, "clear")))) {	// command is not opcode
					/*  arg1 and arg2 are addresses; hence they are written in hexadecimal only
					    arg3 is hexadecimal value  */
					if(argnum >= 1) {
						for(i = 0; i < strlen(arg1); i++) {
							if((arg1[i] >= 65 && arg1[i] <= 70)		// A ~ F
							|| (arg1[i] >= 97 && arg1[i] <= 102)	// a ~ f
							|| (arg1[i] >= 48 && arg1[i] <= 57))	// 0 ~ 9
								continue;

							else if(i == 1 && arg1[i - 1] == '0'
							&& (arg1[i] == 'X' || arg1[i] == 'x'))
								continue;

							else {
								hexproblemflag = 1;
								wrongcmdflag = 1;
								break;
							}
						}
						if(wrongcmdflag == 1)	break;
					}

					if(argnum >= 2) {
						for(i = 0; i < strlen(arg2); i++) {
							if((arg2[i] >= 65 && arg2[i] <= 70)		// A ~ F
							|| (arg2[i] >= 97 && arg2[i] <= 102)	// a ~ f
							|| (arg2[i] >= 48 && arg2[i] <= 57))	// 0 ~ 9
								continue;

							else if(i == 1 && arg2[i - 1] == '0'
							&& (arg2[i] == 'X' || arg2[i] == 'x'))
								continue;

							else {
								hexproblemflag = 1;
								wrongcmdflag = 1;
								break;
							}
						}
						if(wrongcmdflag == 1)	break;
					}

					if(argnum == 3) {
						for(i = 0; i < strlen(arg3); i++) {
							if((arg3[i] >= 65 && arg3[i] <= 70)		// A ~ F
							|| (arg3[i] >= 97 && arg3[i] <= 102)	// a ~ f
							|| (arg3[i] >= 48 && arg3[i] <= 57))	// 0 ~ 9
								continue;

							else if(i == 1 && arg3[i - 1] == '0'
							&& (arg3[i] == 'X' || arg3[i] == 'x'))
								continue;

							else {
								hexproblemflag = 1;
								wrongcmdflag = 1;
								break;
							}
						}

						if(wrongcmdflag == 1)	break;
					}

//---------------------------------CONVERT ARGUMENTS INTO HEXADECIMAL------------------------------------------
					if(argnum >= 1)		hex1 = (int)strtol(arg1, NULL, 16);
					if(argnum >= 2)		hex2 = (int)strtol(arg2, NULL, 16);
					if(argnum == 3)		hex3 = (int)strtol(arg3, NULL, 16);
				}

				if(wrongcmdflag == 1)	break;
			}

			check_history_num_func();

//----------------------------------------------HELP----------------------------------------------
			if(!strcmp(maincmd, "h") || !strcmp(maincmd, "help")) {
				add_history_func(fullcmd);
				help_func();
				break;
			}

//--------------------------------------------DIRECTORY-------------------------------------------
			else if(!strcmp(maincmd, "d") || !strcmp(maincmd, "dir")) {
				add_history_func(fullcmd);
				dir_func();
			}

//----------------------------------------------QUIT----------------------------------------------
			else if(!strcmp(maincmd, "q") || !strcmp(maincmd, "quit"))
				return 0;

//--------------------------------------------HISTORY---------------------------------------------
			else if(!strcmp(maincmd, "hi") || !strcmp(maincmd, "history")) {
				add_history_func(fullcmd);
				show_history_func();
			}

//----------------------------------------------DUMP----------------------------------------------
			else if(!strcmp(maincmd, "du") || !strcmp(maincmd, "dump")) {
				if(argnum == 0) {
					if(dumptrialnum == 0) {
						startpoint = DISPLAYSTART;
						endpoint = DISPLAYEND;
					}

					else {
						if(endpoint == MEGABYTE - 1)
							startpoint = 0;
						else	startpoint = endpoint + 1;
						endpoint = startpoint + DISPLAYEND;
					}
				}

				else if(argnum == 1) {
					startpoint = hex1;
					endpoint = startpoint + DISPLAYEND;

					if(endpoint >= MEGABYTE)
						endpoint = MEGABYTE - 1;

					if(startpoint < 0 || startpoint >= MEGABYTE) {
						printf("The address should be within boundary (0x00000 - 0xFFFFF).\n");
						break;
					}
				}

				else {	// argnum == 2
					startpoint = hex1;
					endpoint = hex2;

					if(startpoint > endpoint) {
						printf("The start address should be smaller than the end address.\n");
						break;
					}

					if(startpoint < 0 || startpoint >= MEGABYTE
					|| endpoint < 0 ||endpoint >= MEGABYTE) {
						printf("The address should be within boundary (0x00000 - 0xFFFFF).\n");
						break;
					}
				}

				dump_func(startpoint, endpoint);
				dumptrialnum++;
				add_history_func(fullcmd);
			}

//----------------------------------------------EDIT----------------------------------------------
			else if(!strcmp(maincmd, "e") || !strcmp(maincmd, "edit")) {
				edit_func(hex1, hex2);
				add_history_func(fullcmd);
			}

//----------------------------------------------FILL----------------------------------------------
			else if(!strcmp(maincmd, "f") || !strcmp(maincmd, "fill")) {
				if(hex1 > hex2) {
					printf("The start address should be smaller than the end address.\n");
					break;
				}

				fill_func(hex1, hex2, hex3);
				add_history_func(fullcmd);
			}

//---------------------------------------------RESET----------------------------------------------
			else if(!strcmp(maincmd, "reset")) {
				reset_func();
				add_history_func(fullcmd);
			}

//--------------------------------------------OPCODE----------------------------------------------
			else if(!strcmp(maincmd, "opcode")) {
				int found = find_opcode_func(arg1, 1);
				if(!found)	break;

				add_history_func(fullcmd);
			}

//------------------------------------------OPCODELIST--------------------------------------------
			else if(!strcmp(maincmd, "opcodelist")) {
				opcode_list_func();
				add_history_func(fullcmd);
			}

//-------------------------------------------ASSEMBLE---------------------------------------------

			else if(!strcmp(maincmd, "assemble")) {
				assemble_func(arg1);
				add_history_func(fullcmd);
			}

//---------------------------------------------TYPE-----------------------------------------------
			else if(!strcmp(maincmd, "type")) {
				type_func(arg1);
				add_history_func(fullcmd);
			}

//--------------------------------------------SYMBOL----------------------------------------------
			else if(!strcmp(maincmd, "symbol")) {
				symbol_print_func();
				add_history_func(fullcmd);
			}

//--------------------------------------------SYMBOL----------------------------------------------
			else if(!strcmp(maincmd, "progaddr")) {
				if(argnum == 0) progaddr_func(0);
				else progaddr_func(hex1);
				add_history_func(fullcmd);
			}

			else if(!strcmp(maincmd, "loader")) {
				totallength = loader_func(arg1, arg2, arg3);
				add_history_func(fullcmd);
			}

			else if(!strcmp(maincmd, "bp")) {
				if(argnum == 0)
					bp_print_func();
				else if(argnum == 1) {
					if(!strcmp(arg1, "clear")) bp_clear_func();
					else add_bp_func(hex1);
				}

				add_history_func(fullcmd);
			}

			else if(!strcmp(maincmd, "run")) {
				run_func(totallength, &currbp, &nextbp);
				add_history_func(fullcmd);
			}
		}

		if(wrongcmdflag) {
			if(hexproblemflag)
				printf("The arguments should be given in hexadecimal form.\n");
			else
				printf("Enter the proper command. Enter \"help\" to learn more.\n");
		}
	}

	return 0;
}
