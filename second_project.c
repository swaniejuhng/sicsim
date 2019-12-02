#include "20150959.h"

int type_func(char *filename) {
	char *filestr, buffer[1000];
	FILE *fp = fopen(filename, "r");
	if(!fp) {
		printf("Failed to open %s.\n", filename);
		return -1;
	}

	while (1) {
		filestr = fgets(buffer, 1000, fp);
		if (filestr == NULL) break;
		printf("%s", filestr);
	}
	printf("\n");
	return 0;
}

int key_for_symbol_func(char *sym) {
  int i, sum = 0;
  for(i = 0; i < strlen(sym); i++)
    sum += sym[i];
  return sum % SYMBOLTABLENUM;
}

void add_symbol_func(char *sym, int addr) {
    int key = key_for_symbol_func(sym);
    if(!symboltable[key]) {
        symboltable[key] = (SYMBOL *)malloc(sizeof(SYMBOL));
        strcpy(symboltable[key]->symbol, sym);
        symboltable[key]->address = addr;
        symboltable[key]->next = NULL;
    }
    else {
        symtemp = symboltable[key];
        while(symtemp->next && strcmp(symtemp->symbol, sym))
            symtemp = symtemp->next;
        symnewnode = (SYMBOL *)malloc(sizeof(SYMBOL));
        strcpy(symnewnode->symbol, sym);
        symnewnode->address = addr;
        symnewnode->next = NULL;
        symtemp->next = symnewnode;
    }
}

int find_label_func(char *label, int option) {
    int i;
    for(i = 0; i < SYMBOLTABLENUM; i++) {
        symtemp = symboltable[i];
        while(symtemp) {
            if(!strcmp(symtemp->symbol, label)) {
                if(option == 1) return 1;
                if(option == 2) return symtemp->address;
            }
            symtemp = symtemp->next;
        }
    }
    if(option == 1) return 0;
    else            return -1;
}

char *format_for_opcode_func(char *mn) {
    int i;
    for(i = 0; i < HASHTABLENUM; i++) {
        hashtemp = hashtable[i];
        while(hashtemp) {
            if(!strcmp(hashtemp->mnemonic, mn))
                return hashtemp->format;
            hashtemp = hashtemp->next;
        }
    }
    return "";
}

void symbol_print_func(void) {
    int i, j, n, maxidx;
    char symbolname[100][10] = {0}, tempstr[10] = "";
    int symboladdr[100], tempaddr = 0;

    symtemp = symhead = symnewnode = NULL;
    n = 0;
    for(i = 0; i < SYMBOLTABLENUM; i++) {
        symtemp = symboltable[i];

        while(symtemp) {
            if(strcmp(symtemp->symbol, "A") && strcmp(symtemp->symbol, "X") && strcmp(symtemp->symbol, "L")
            && strcmp(symtemp->symbol, "PC") && strcmp(symtemp->symbol, "SW") && strcmp(symtemp->symbol, "B")
            && strcmp(symtemp->symbol, "S") && strcmp(symtemp->symbol, "T") && strcmp(symtemp->symbol, "F")) {
                strcpy(symbolname[n], symtemp->symbol);
                symboladdr[n++] = symtemp->address;
            }
            symtemp = symtemp->next;
        }
    }
    for(i = 0; i < n - 1; i++) {
        maxidx = i;
        for(j = i + 1; j < n; j++)
            if(strcmp(symbolname[j], symbolname[maxidx]) > 0)
                maxidx = j;
        strcpy(tempstr, symbolname[maxidx]); strcpy(symbolname[maxidx], symbolname[i]); strcpy(symbolname[i], tempstr);
        tempaddr = symboladdr[maxidx]; symboladdr[maxidx] = symboladdr[i]; symboladdr[i] = tempaddr;
    }

    for(i = 0; i < n; i++)
        printf("\t%s\t%04X\n", symbolname[i], symboladdr[i]);
}

int assemble_func(char *filename) {
    int i, line = 0, startaddr, locctr, errorflag = 0, bytenum = 0, pglength = 0, constflag = 0, errorcount = 0;
    int opcodenum, targetaddr, disp;
    char *reg1, *reg2;
    char lstfname[10] = "", objfname[10] = "", record[70] = "";
    char asmline[MAX_CMD_LEN] = "", templine[MAX_CMD_LEN] = "";
    int N = 0, I = 0, X = 0, B = 0, P = 0, E = 0, pgctr = 0, base = 0;
    char *temp1, *temp2, *temp3, *temp4, *temp5;
    char label[10] = "", opcode[10] = "", operand[10] = "", format[5] = "", comment[50] = "";
    char objcode[9] = "", hexstring[9] = "", tempoperand[10] = "";
    FILE *fp, *itmfp, *lstfp, *objfp, *errfp;

    for(i = 0; i < strlen(filename); i++)
        if(filename[i] == '.')
            break;
    strncpy(lstfname, filename, i); strcat(lstfname, ".lst");
    strncpy(objfname, filename, i); strcat(objfname, ".obj");
    lstfname[strlen(lstfname)] = objfname[strlen(objfname)] = '\0';
    if(filename[strlen(filename) - 3] != 'a'
    || filename[strlen(filename) - 2] != 's'
    || filename[strlen(filename) - 1] != 'm') {
        printf("The input file is not .asm file.\n"); return -1; }

    fp = fopen(filename, "r");
    if(!fp) { printf("Failed to open %s.\n", filename);   return -1; }

    itmfp = fopen("intermediate", "w");
    if(!itmfp) { printf("Failed to open intermediate file.\n");  return -1; }

    errfp = fopen("errorflagfile", "w");
    if(!errfp) { printf("Failed to open error flag file.\n"); }
    fprintf(errfp, "-----------PASS 1 ERRORS-----------\n");

/*------------------  READ ASM FILE LINE  ----------------------*/


    while(fgets(asmline, MAX_CMD_LEN, fp)) {
        asmline[strlen(asmline) - 1] = '\0';
        temp1 = temp2 = temp3 = temp4 = temp5 = NULL;
        memset(label, 0, 10); memset(opcode, 0, 10); memset(operand, 0, 10); memset(format, 0, 5);

        strcpy(templine, asmline);
        temp1 = strtok(templine, " \t");
        temp2 = strtok(NULL, " \t");
        temp3 = strtok(NULL, " \t");
        temp4 = strtok(NULL, " \t");
        temp5 = strtok(NULL, " \t");

/*-----------------  PREPROCESSING INPUT FOR PASS 1  -------------------*/
        // nothing written
        if(!temp1)  continue;
		line += 5;
        // if comment then pass
        if(temp1[0] == '.')
            fprintf(itmfp, "%3d\t%04X\t\t%s\n", line, locctr, asmline);
        // only opcode exists
        else if(!temp2) strcpy(opcode, temp1);
        // no label
        else if(!temp3) { strcpy(opcode, temp1); strcpy(operand, temp2); }

        else if(!temp4) {
            // no label, two operands
            if(temp2[strlen(temp2) - 1] == ',' || temp3[0] == ',') {
                strcpy(opcode, temp1);
                strcpy(operand, temp2); strcat(operand, temp3);
            }
            // no label, one operand
            else { strcpy(label, temp1); strcpy(opcode, temp2); strcpy(operand, temp3);}
        }
        // no label, two operands with separate comma
        else if(!strcmp(temp3, ",")) {
            strcpy(opcode, temp1);
            strcpy(operand, temp2); strcat(operand, temp3); strcat(operand, temp4);
        }
        // label exists and two operands
        else if(!strcmp(temp4, ",")) {
            strcpy(label, temp1); strcpy(opcode, temp2);
            strcpy(operand, temp3); strcat(operand, temp4); strcat(operand, temp5);
        }

/*----------------------------  PASS 1  ----------------------------*/
        if(line == 5) {
            if(!strcmp(opcode, "START")) {
				startaddr = locctr = (int)strtol(operand, NULL, 16);
				fprintf(itmfp, "%3d\t%04X\t%s\t%s\t%s\n", line, locctr, label, opcode, operand);
	            continue;
			}

            else {
				printf("WARNING: The program does not start with START directive.\n");
				startaddr = locctr = 0;
			}
        }

        if(temp1[0] != '.') {      // while OPCODE != "END"
            if(strcmp(opcode, "END")) {       // if this is not a comment line
                if(strcmp(label, "")) {     // if there is a symbol in the LABEL field then
                    errorflag = find_label_func(label, 1);  // search SYMTAB for LABEL, if found then set error flag
                    if(errorflag)   {
						errorcount++;	errorflag = DUPLICATESYMBOL;
                        fprintf(errfp, "line %d : duplicate symbol (%s)\n", line, label);
                    }
                    else    add_symbol_func(label, locctr);
                }

                // finding out the format
                if(opcode[0] == '+')    bytenum = 4;

                else if(find_opcode_func(opcode, 2) != -1) {
                    strcpy(format, format_for_opcode_func(opcode));
                    bytenum = (int)(format[0] - '0');
                }
                else if(!strcmp(opcode, "WORD"))    bytenum = 3;
                else if(!strcmp(opcode, "RESW"))    bytenum = 3 * (int)strtol(operand, NULL, 10);
                else if(!strcmp(opcode, "RESB"))    bytenum = (int)strtol(operand, NULL, 10);
                else if(!strcmp(opcode, "BYTE")) {
                    if(operand[0] == 'C')           bytenum = (strlen(operand)- 3);
                    if(operand[0] == 'X' && operand[1] == '\'') bytenum = ((strlen(operand) - 3) / 2);
                }
				else if(!strcmp(opcode, "BASE"))	bytenum = 0;
                else {
                    errorcount++; errorflag = INVALIDOPCODE; bytenum = 0;
                    fprintf(errfp, "line %d : invalid opcode (%s)\n", line, opcode);
                }

                fprintf(itmfp, "%3d\t%04X\t%s\t%s\t%s\n", line, locctr, label, opcode, operand);
            }
            else {  // END
                fprintf(itmfp, "%3d\t%04X\t%s\t%s\t%s\n", line, locctr, label, opcode, operand);
                pglength = locctr - startaddr;
            }

            locctr += bytenum;  // lext locctr = program counter
        }

    }
    fclose(fp); fclose(itmfp);

/*-----------------  PREPROCESSING INPUT FOR PASS 2  -------------------*/
    itmfp = fopen("intermediate", "r");
    objfp = fopen(objfname, "w");
    lstfp = fopen(lstfname, "w");
    memset(record, 0, 70);

    fprintf(errfp, "\n-----------PASS 2 ERRORS-----------\n");

    while(fgets(asmline, MAX_CMD_LEN, fp)) {
        asmline[strlen(asmline) - 1] = '\0';
        temp1 = temp2 = temp3 = temp4 = temp5 = NULL;
        memset(label, 0, 10); memset(opcode, 0, 10); memset(operand, 0, 10);
        memset(format, 0, 5); memset(objcode, 0, 9); memset(hexstring, 0, 9);
        memset(comment, 0, 50); memset(tempoperand, 0, 10);
        N = I = X = B = P = E = 0;
        strcpy(templine, asmline);
        temp1 = strtok(asmline, " \t");     // line number
        temp2 = strtok(NULL, " \t");     // locctr
        temp3 = strtok(NULL, " \t");     // label    or  opcode  or   '.'(comment)
        if(temp3[0] != '.') {
            temp4 = strtok(NULL, " \t");     // opcode   or  operand(s)
            temp5 = strtok(NULL, " \t");     // operand(s) or NULL
        }
        else {
            temp4 = strtok(NULL, "");
            if(temp4) strcpy(comment, temp4);
        }

        line = (int)strtol(temp1, NULL, 10);
        locctr = (int)strtol(temp2, NULL, 16);
        if(temp3[0] == '.') {
            if(!temp4)  fprintf(lstfp, "\t.\n");
            else        fprintf(lstfp, "\t.  %s\n", comment);
            continue;
        }
        else if(!temp4) strcpy(opcode, temp3);
        else if(!temp5) { strcpy(opcode, temp3); strcpy(operand, temp4); }
        else            { strcpy(label, temp3); strcpy(opcode, temp4); strcpy(operand, temp5); }


        if(opcode[0] == '+')    bytenum = 4;

        else if(find_opcode_func(opcode, 2) != -1) {
            strcpy(format, format_for_opcode_func(opcode));
            bytenum = (int)(format[0] - '0');
        }

        else if(!strcmp(opcode, "WORD"))    bytenum = 3;
        else if(!strcmp(opcode, "RESW"))    bytenum = 3 * (int)strtol(operand, NULL, 10);
        else if(!strcmp(opcode, "RESB"))    bytenum = (int)strtol(operand, NULL, 10);
        else if(!strcmp(opcode, "BYTE")) {
            if(operand[0] == 'C')   bytenum = (strlen(operand) - 3);
            if(operand[0] == 'X' && operand[1] == '\'')    bytenum = ((strlen(operand) - 3) / 2);
        }
		else ;
        pgctr = locctr + bytenum;

/*----------------------------  PASS 2  ----------------------------*/
        if(line == 5) {
			record[0] = 'H';
			strcat(record, label);
			if(strlen(label) < 6)
				for(i = 0; i < (6 - strlen(label)); i++)
					strcat(record, " ");
			sprintf(objcode, "%06X", startaddr);   strcat(record, objcode);
			sprintf(objcode, "%06X", pglength);    strcat(record, objcode);
			record[19] = '\0';
			fprintf(objfp, "%s\n", record);
			memset(record, 0, 70);

            if(!strcmp(opcode, "START")) {
				fprintf(lstfp, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);
				continue;
			}
        }

        if(strcmp(opcode, "END")) { // opcode != END
            // search OPTAB for OPCODE
            if(((opcodenum = find_opcode_func(opcode, 2)) != -1)
            || (bytenum == 4 && (opcodenum = find_opcode_func(&opcode[1], 2)) != -1)) {
                if(constflag == 1)  constflag = 2;
                    if(bytenum == 1) {
					if(strcmp(operand, "")) {
						errorcount++; errorflag = INAPPROPRIATEOPERAND; targetaddr = 0;
						fprintf(errfp, "line %d : inappropriate operand for format 1 (%s)\n", line, operand);
					}
					else sprintf(objcode, "%02X", opcodenum);
				}
                else if(bytenum == 2) {
                    strcpy(tempoperand, operand);
                    sprintf(objcode, "%02X", opcodenum);
                    reg1 = strtok(tempoperand, ",");
                    reg2 = strtok(NULL, ",");
					// reg1
					if(!reg1) {
						errorcount++; errorflag = UNDEFINEDSYMBOL; targetaddr = 0;
						fprintf(errfp, "line %d : inappropriate operand for format 2 (%s)\n", line, operand);
					}
					else {
						if(find_label_func(reg1, 2) != -1) {
							if(strcmp(reg1, "A") && strcmp(reg1, "X") && strcmp(reg1, "L")
							&& strcmp(reg1, "PC") && strcmp(reg1, "SW") && strcmp(reg1, "B")
							&& strcmp(reg1, "S") && strcmp(reg1, "T") && strcmp(reg1, "F")) {
								errorcount++; errorflag = INAPPROPRIATEOPERAND; targetaddr = 0;
								fprintf(errfp, "line %d : inappropriate operand for format 2 (%s)\n", line, operand);
							}
							else objcode[2] = (char)(find_label_func(reg1, 2) + '0');
						}
						else {
							if((int)strtol(reg1, NULL, 10) < 0 || (int)strtol(reg1, NULL, 10) >= 16) {
								errorcount++; errorflag = INAPPROPRIATEOPERAND; targetaddr = 0;
								fprintf(errfp, "line %d : inappropriate operand for format 2 (%s)\n", line, operand);
							}
							else objcode[2] = (char)((int)strtol(reg1, NULL, 16) + '0');
						}
					}

					// reg2
					if(reg2) {
						if(find_label_func(reg2, 2) != -1) {
							if(strcmp(reg2, "A") && strcmp(reg2, "X") && strcmp(reg2, "L")
							&& strcmp(reg2, "PC") && strcmp(reg2, "SW") && strcmp(reg2, "B")
							&& strcmp(reg2, "S") && strcmp(reg2, "T") && strcmp(reg2, "F")) {
								errorcount++; errorflag = INAPPROPRIATEOPERAND; targetaddr = 0;
								fprintf(errfp, "line %d : inappropriate operand for format 2 (%s)\n", line, operand);
							}
							else objcode[3] = (char)(find_label_func(reg2, 2) + '0');
						}
						else {
							if((int)strtol(reg2, NULL, 10) < 0 || (int)strtol(reg2, NULL, 10) >= 16) {
								errorcount++; errorflag = INAPPROPRIATEOPERAND; targetaddr = 0;
								fprintf(errfp, "line %d : inappropriate operand for format 2 (%s)\n", line, operand);
							}
							else objcode[3] = (char)((int)strtol(reg2, NULL, 16) + '0');
						}
					}
					else        objcode[3] = '0';
                }

                else if(bytenum == 3 || bytenum == 4) {
                    if(strcmp(operand, "")) {   // if there is a symbol in OPERAND field
                        if(operand[0] == '@' || operand[0] == '#') {
                            if(operand[0] == '@')       { N = 1; I = 0; }     // indirect addressing
                            else if(operand[0] == '#')  { N = 0; I = 1; }     // immediate addressing

                            targetaddr = find_label_func(&operand[1], 2);
                            if(targetaddr == -1) {  // target address is written in number
                                if(operand[1] >= '0' && operand[1] <= '9') {
                                    targetaddr = (int)strtol(&operand[1], NULL, 10);
                                    disp = targetaddr;
                                }
                                else {
                                    errorcount++; errorflag = UNDEFINEDSYMBOL; targetaddr = 0;
                                    fprintf(errfp, "line %d : undefined symbol (%s)\n", line, operand);
                                }
                            }
                        }
                        else {// simple addressing
                            // indexed addressing
                            if(operand[strlen(operand) - 2] == ',' && operand[strlen(operand) - 1] == 'X') {
                                X = 1; operand[strlen(operand) - 1] = '\0'; operand[strlen(operand) - 1] = '\0';
                                targetaddr = find_label_func(operand, 2);
                            }

                            N = 1; I = 1;
                            targetaddr = find_label_func(operand, 2);
                            if(targetaddr == -1) {  // target address is written in number
                                if(operand[0] >= '0' && operand[0] <= '9')
                                    targetaddr = (int)strtol(operand, NULL, 10);
                                else {
                                    errorcount++; errorflag = UNDEFINEDSYMBOL; targetaddr = 0;
                                    fprintf(errfp, "line %d : undefined symbol (%s)\n", line, operand);
                                }
                            }
                        }
                    } // if symbol
                    else { N = 1; I = 1; disp = targetaddr = 0; }   // example. RSUB
                        if(bytenum == 3) {
                        if(disp == targetaddr) { B = 0; P = 0; }
                        else {
                            disp = targetaddr - pgctr;
                            // PC relative address
                            if(disp >= -2048 && disp <= 2047)  { B = 0; P = 1; }
                            // Base relative address
                            else {
                                B = 1; P = 0;
                                disp = targetaddr - base;
                            }
                        }
                    }

                    else if(bytenum == 4) { B = 0; P = 0; E = 1; disp = targetaddr; }

                    sprintf(objcode, "%02X", (opcodenum + N * 2 + I * 1));
                    sprintf(&objcode[2], "%01X", (X * 8 + B * 4 + P * 2 + E * 1));

                    if(bytenum == 3) {
                        sprintf(hexstring, "%08X", disp);
                        strcat(objcode, &hexstring[5]);
                    }
                    else if(bytenum == 4) {
                        sprintf(hexstring, "%08X", disp);
                        strcat(objcode, &hexstring[3]);
                    }
                }
            }// if opcode found

            else if(!strcmp(opcode, "BASE")) {
                if(operand[0] == '@' || operand[0] == '#')
                    base = find_label_func(&operand[1], 2);
                else    base = find_label_func(operand, 2);
            }

            else if(!strcmp(opcode, "BYTE")) {
                if(operand[0] == 'C')
                    for(i = 2; i < strlen(operand) - 1; i++) {
                        sprintf(hexstring, "%02X", operand[i]);
                        strcat(objcode, hexstring);
                    }
                else if(operand[0] == 'X')
                    strncpy(objcode, &operand[2], strlen(operand) - 3);
            }

            else if(!strcmp(opcode, "WORD"))
                sprintf(objcode, "%06X", (int)strtol(operand, NULL, 10));

            else if(!strcmp(opcode, "RESW") || !strcmp(opcode, "RESB"))
                constflag = 1;

            if(strlen(record) + strlen(objcode) >= 70 || constflag == 2) {
                constflag = 0;
                sprintf(hexstring, "%02X", (int)((strlen(record) - 9) / 2));
                record[7] = hexstring[0]; record[8] = hexstring[1];   // write length of object code
                fprintf(objfp, "%s\n", record);
                memset(record, 0, 70);
            }

            if(strlen(record) == 0) {
                record[0] = 'T';
                sprintf(hexstring, "%06X", locctr); strcat(record, hexstring);
                strcat(record, "00"); // temporarily assign length of object code
            }

            strcat(record, objcode);
        }

        else {  // opcode = END
            if(strlen(record) > 0) {
                sprintf(hexstring, "%02X", (int)((strlen(record) - 9) / 2));
                record[7] = hexstring[0]; record[8] = hexstring[1];   // write length of object code
                fprintf(objfp, "%s\n", record);
                memset(record, 0, 70);
                record[0] = 'E';
                sprintf(&record[1], "%06X", startaddr);
                fprintf(objfp, "%s", record);
            }
        }
    	if(!strcmp(label, "BASE") || !strcmp(label, "END"))
        	fprintf(lstfp, "\t\t%s\t%s\t%s\t%s\n", label, opcode, operand, objcode);
        else if(X == 1)     fprintf(lstfp, "%04X\t%s\t%s\t%s,X\t%s\n", locctr, label, opcode, operand, objcode);
    	else        fprintf(lstfp, "%04X\t%s\t%s\t%s\t\t%s\n", locctr, label, opcode, operand, objcode);
	}

    fclose(itmfp);  fclose(objfp);  fclose(errfp); fclose(lstfp);
	if(errorcount) {
		type_func("errorflagfile");
		printf("\nFailed to create %s and %s.\n", lstfname, objfname);
		remove(lstfname); remove(objfname);
	}
	else    printf("output file: [%s], [%s]\n", lstfname, objfname);
    remove("intermediate");	remove("errorflagfile");
    return 0;
}
