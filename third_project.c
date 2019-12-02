#include "20150959.h"
#include <math.h>

void progaddr_func(int address) {
    progaddr = address;
    printf("Program starting address set to 0x%04X.\n", address);
    return;
}

int loader_func(char *fname1, char *fname2, char *fname3) {
    int totallength = progaddr, drflag_1 = 0, drflag_2 = 0, drflag_3 = 0;
    char progname1[7] = "", progname2[7] = "", progname3[7] = "", objline[MAX_CMD_LEN] = "";
    FILE *fp1, *fp2, *fp3;

    if(exthead) {
        exttemp = exthead;
        while(exthead->next) {
            exthead = exthead->next;
            free(exttemp);
            exttemp = exthead;
        }
        free(exttemp);
    }
    exthead = exttemp = extnewnode = NULL;

    fp1 = fopen(fname1, "r");
    if(!fp1) { printf("Failed to open %s.\n", fname1); return -1; }
    while(fgets(objline, MAX_CMD_LEN, fp1))
        if(objline[0] == 'D') { drflag_1 = 1; break; }
    fclose(fp1);

    if(fname2) {
        fp2 = fopen(fname2, "r");
        if(!fp2) { printf("Failed to open %s.\n", fname2); return -1; }
        while(fgets(objline, MAX_CMD_LEN, fp2))
            if(objline[0] == 'D') { drflag_2 = 1; break; }
        fclose(fp2);
    }

    if(fname3) {
        fp3 = fopen(fname3, "r");
        if(!fp3) { printf("Failed to open %s.\n", fname3); return -1; }
        while(fgets(objline, MAX_CMD_LEN, fp3))
            if(objline[0] == 'D') { drflag_3 = 1; break; }
        fclose(fp3);
    }

    fp1 = fopen(fname1, "r");
    totallength = extdef_func(fp1, totallength, progname1, drflag_1);

    if(fname2) {
        fp2 = fopen(fname2, "r");
        totallength = extdef_func(fp2, totallength, progname2, drflag_2);
    }

    if(fname3) {
        fp3 = fopen(fname3, "r");
        totallength = extdef_func(fp3, totallength, progname3, drflag_3);
    }
    totallength -= progaddr;

    extref_func(fp1, progname1, drflag_1); tm_record_func(fp1, progname1, drflag_1);
    if(fname2) { extref_func(fp2, progname2, drflag_2); tm_record_func(fp2, progname2, drflag_2); }
    if(fname3) { extref_func(fp3, progname3, drflag_3); tm_record_func(fp3, progname3, drflag_3); }


    printf("control\t\tsymbol\t\taddress\t\tlength\n");
    printf("section\t\tname\n");
    printf("----------------------------------------------------------\n");
    exttemp = exthead;
    while(exttemp) {
        if(strcmp(exttemp->controlsection, ""))
            printf("%s\t\t\t\t%04X\t\t%04X\n", exttemp->controlsection, exttemp->address, exttemp->length);
        else printf("\t\t%s\t\t%04X\n", exttemp->symbolname, exttemp->address);
        exttemp = exttemp->next;
    }
    printf("----------------------------------------------------------\n");
    printf("\t\t\t\ttotal length\t%04X\n", totallength);

    return totallength;
}

int extdef_func(FILE *fp, int startaddr, char *progname, int drflag) {
    int i, j, length = 0;
    char objline[MAX_CMD_LEN] = "";
    char hexstring[10] = "";

    fgets(objline, MAX_CMD_LEN, fp);
    if(objline[0] == 'H') {
        extnewnode = (EXTSYM *)malloc(sizeof(EXTSYM));
        strncpy(extnewnode->controlsection, &objline[1], 6);
        strcpy(progname, extnewnode->controlsection);
        for(j = 0; j < 7; j++)
            if(extnewnode->controlsection[j] == ' ' || extnewnode->controlsection[j] == '\t' || extnewnode->controlsection[j] == '\n') {
                extnewnode->controlsection[j] = '\0'; progname[j] = '\0';
            }

        strcpy(extnewnode->symbolname, "");
        strncpy(hexstring, &objline[7], 6);
        extnewnode->address = startaddr + (int)strtol(hexstring, NULL, 16);
        strncpy(hexstring, &objline[13], 6);
        extnewnode->length = (int)strtol(hexstring, NULL, 16);
        extnewnode->next = NULL;
        length = extnewnode->length;

        if(!exthead) exthead = exttemp = extnewnode;
        else { exttemp->next = extnewnode; exttemp = exttemp->next; }
    }
    else  { printf("This .obj file is not valid.\n"); return -1; }

    if(drflag == 0) return startaddr + length;

    fgets(objline, MAX_CMD_LEN, fp);
    objline[strlen(objline) - 1] = '\0';
    if(objline[0] == 'D') {
        for(i = 1; i < strlen(objline); i += 12) {
            extnewnode = (EXTSYM *)malloc(sizeof(EXTSYM));
            strcpy(extnewnode->controlsection, "");
            strncpy(extnewnode->symbolname, &objline[i], 6);
            for(j = 0; j < 7; j++)
                if(extnewnode->symbolname[j] == ' ' || extnewnode->symbolname[j] == '\t' || extnewnode->symbolname[j] == '\n')
                    extnewnode->symbolname[j] = '\0';
            strncpy(hexstring, &objline[i + 6], 6);
            extnewnode->address = startaddr + (int)strtol(hexstring, NULL, 16);
            extnewnode->length = 0;
            extnewnode->next = NULL;
            exttemp->next = extnewnode;
            exttemp = exttemp->next;
        }
    }
    return startaddr + length;
}

void extref_func(FILE *fp, char *progname, int drflag) {
    int i, j;
    char objline[MAX_CMD_LEN] = "", symbolname[7] = "", number[3] = "";
    if(drflag == 0) return;

    exttemp = exthead;
    while(exttemp) { memset(exttemp->number, 0, 3); exttemp = exttemp->next; }
    fgets(objline, MAX_CMD_LEN, fp);
    objline[strlen(objline) - 1] = '\0';
    if(objline[0] == 'R') {
        if(objline[1] >= '0' && objline[1] <= '9') {
            for(i = 1; i < strlen(objline); i += 8) {
                memset(symbolname, 0, 7); memset(number, 0, 3);
                strncpy(number, &objline[i], 2); strncpy(symbolname, &objline[i + 2], 6);
                for(j = 0; j < 7; j++)
                    if(symbolname[j] == ' ' || symbolname[j] == '\t' || symbolname[j] == '\n')
                        symbolname[j] = '\0';
                exttemp = exthead;
                while(exttemp) {
                    if(!strcmp(exttemp->controlsection, progname)) strcpy(exttemp->number, "01");
                    else if(!strcmp(exttemp->symbolname, symbolname)) strcpy(exttemp->number, number);
                    exttemp = exttemp->next;
                }
            }
        }
    }

    return;
}

void tm_record_func(FILE *fp, char *progname, int drflag) {
    char objline[MAX_CMD_LEN] = "", hexstring[7] = "", temphexstr[9] = "";
    int i, address = 0, value = 0, recpos, startaddr = 0, firstnum, tempvalue = 0;
    exttemp = exthead;

    while(exttemp) {
        if(drflag) {
            if(!strcmp(exttemp->number, "01")) { startaddr = exttemp->address; break; }
        }
        else {
            if(!strcmp(exttemp->controlsection, progname)) { startaddr = exttemp->address; break; }
        }

        exttemp = exttemp->next;
    }

    while(fgets(objline, MAX_CMD_LEN, fp)) {
        objline[strlen(objline) - 1] = '\0';
        memset(hexstring, 0, 7);

        if(objline[0] == '.')   continue;

        else if(objline[0] == 'T') {
            strncpy(hexstring, &objline[1], 6);
            address = (int)strtol(hexstring, NULL, 16);
            for(i = 9; i < strlen(objline); i += 2) {
                memset(hexstring, 0, 7);
                strncpy(hexstring, &objline[i], 2);
                value = (int)strtol(hexstring, NULL, 16);
                edit_func(startaddr + address, value);
                address += 1;
            }
        }

        else if(objline[0] == 'M') {
            memset(hexstring, 0, 7);
            strncpy(hexstring, &objline[1], 6);
            address = (int)strtol(hexstring, NULL, 16);
            memset(hexstring, 0, 7);
            strncpy(hexstring, &objline[7], 2);
            recpos = (int)strtol(hexstring, NULL, 16);

            for(i = 0; i < (recpos + 1) / 2; i++)
                sprintf(&hexstring[i * 2], "%02X", memory[startaddr + address + i]);

            tempvalue = (int)strtol(hexstring, NULL, 16);

            if(recpos % 2 == 1) {
                hexstring[1] = '\0';
                firstnum = (int)strtol(hexstring, NULL, 16);
                tempvalue = tempvalue % ((int)pow((double)16, (double)recpos));
            }
            else firstnum = 0;

            if(strlen(objline) >= 10) {
                memset(hexstring, 0, 7);
                if(objline[10] >= '0' || objline[10] <= '9') {
                    strncpy(hexstring, &objline[10], 2);
                    exttemp = exthead;
                    while(exttemp) {
                        if(!strcmp(exttemp->number, hexstring)) {
                            if(objline[9] == '+') tempvalue += exttemp->address;
                            else if(objline[9] == '-') tempvalue -= exttemp->address;
                            break;
                        }
                        exttemp = exttemp->next;
                    }
                }

                else {
                    strcpy(hexstring, &objline[10]);
                    exttemp = exthead;
                    while(exttemp) {
                        if(!strcmp(exttemp->symbolname, hexstring)) {
                            if(objline[9] == '+') tempvalue += exttemp->address;
                            else if(objline[9] == '-') tempvalue -= exttemp->address;
                            break;
                        }
                        exttemp = exttemp->next;
                    }
                }
            }

            else tempvalue += progaddr;

            memset(hexstring, 0, 7); memset(temphexstr, 0, 9);
            sprintf(temphexstr, "%06X", tempvalue);
            if(strlen(temphexstr) > 6) strcpy(hexstring, &temphexstr[2]);
            else strcpy(hexstring, temphexstr);
            tempvalue = (int)strtol(hexstring, NULL, 16);
            tempvalue += firstnum * ((int)pow((double)16, (double)recpos));

            memset(hexstring, 0, 7);
            sprintf(hexstring, "%06X", tempvalue);
            for(i = 0; i < (recpos + 1) / 2; i++) {
                memset(temphexstr, 0, 9);
                strncpy(temphexstr, &hexstring[i * 2], 2);
                memory[startaddr + address + i] = (int)strtol(temphexstr, NULL, 16);
            }
        }

        else if(objline[0] == 'E') {
            if(strlen(objline) > 1) {
                memset(hexstring, 0, 7);
                strncpy(hexstring, &objline[1], 6);
                execaddr = startaddr + (int)strtol(hexstring, NULL, 16);
            }
        }
    }

    return;
}

void add_bp_func(int address) {
    breakpoint[address] = 1; bpnum++;
    printf("[ok] create breakpoint %04X\n", address);
    return;
}

void bp_print_func(void) {
    int i;
    if(bpnum == 0) { printf("No breakpoints set.\n"); return; }

    printf("breakpoints\n");
    printf("-----------\n");
    for(i = 0; i < MEGABYTE; i++)
        if(breakpoint[i] == 1) printf("%04X\n", i);
    return;
}

void bp_clear_func(void) {
    memset(breakpoint, 0, MAX_CMD_LEN);
    bpnum = 0;
    printf("[ok] clear all breakpoints\n");
}

int find_mnemonic_func(int opcode, char *mnemonic) {
    int i;
    for(i = 0; i < HASHTABLENUM; i++) {
        hashtemp = hashtable[i];
        while(hashtemp) {
            if((hashtemp->opcode >= (opcode - 3)) && (hashtemp->opcode <= opcode)) {
                strcpy(mnemonic, hashtemp->mnemonic);
                return hashtemp->opcode;
            }
            hashtemp = hashtemp->next;
        }
    }
    return -1;
}

void run_func(int totallength, int *currbp, int *nextbp) {   //curraddr : current address
    int opcode, endflag = 0, j, bytenum, temp, firstreg, secondreg;
    int endaddr = progaddr + totallength;
    int n = 0, i = 0, x = 0, b = 0, p = 0, e = 0;
    int targetaddr, disp, comp = 0, meetbpflag = 0;
    char mnemonic[10], format[5] = "";

    if(PCtemp == -1) { PCtemp = PC; L = endaddr; }
    /* program execution from beginning */
    if(*currbp == -1) {
        curraddr = execaddr;
        if(bpnum == 0) *currbp = *nextbp = endaddr;
        else
            for(j = curraddr; j <= endaddr; j++)
                if(breakpoint[j] == 1) { *currbp = j; break; }
    }

    while(1) {
        x = 0; b = 0; p = 0; e = 0; bytenum = 0;
        /* process of finding opcode and addressing mode */
        temp = memory[curraddr];
        opcode = find_mnemonic_func(temp, mnemonic);

        if(bpnum > 0) {
            for(j = *currbp + 1; j <= endaddr; j++)
                if(breakpoint[j] == 1) { *nextbp = j; break; }
            if(j == endaddr + 1) *nextbp = endaddr;
        }

        if(strcmp(mnemonic, "")) {
            strcpy(format, format_for_opcode_func(mnemonic));
            bytenum = (int)(format[0] - '0');

            if((bytenum == 3 ) && ((memory[curraddr + 1] & 0x10) == 0x10)) { e = 1; bytenum = 4; }
            PC = curraddr + bytenum;

            // if PC is bigger than program length, then the loop should be stopped
            if(PC > endaddr) { PC = endaddr; endflag = 1; break; }

            for(j = 0; j < bytenum; j++)
                if(curraddr + j == *currbp) { meetbpflag = 1; break; }

            if(meetbpflag) { PCtemp = PC; PC = *currbp; break; }

            /* format 3 / 4 */
            if(bytenum == 3 || bytenum == 4) {
                switch(temp - opcode) {
                    case 1 : n = 0; i = 1; break; // 1 : immediate addressing
                    case 2 : n = 1; i = 0; break; // 2 : indirect addressing
                    case 3 : n = 1; i = 1; break; // 3 : simple addressing
                }

                if((memory[curraddr + 1] & 0x80) == 0x80) x = 1;
                if((memory[curraddr + 1] & 0x40) == 0x40) b = 1;
                if((memory[curraddr + 1] & 0x20) == 0x20) p = 1;

                /* format 3 */
                if(e == 0) {
                    disp = ((memory[curraddr + 1] & 0x0F) << 8) + memory[curraddr + 2];
                    if((disp & 0x800) == 0x800) disp |= 0xF000;
                    if(b == 1 || p == 1) {
                        if(p == 1) targetaddr = disp + PC;
                        else if(b == 1) targetaddr = disp + B;

                        targetaddr &= 0xFFFF;
                        /* indirect addressing */
                        if(n == 1 && i == 0) {
                            temp = (memory[targetaddr] << 16);
                            temp += (memory[targetaddr + 1] << 8);
                            temp += memory[targetaddr + 2];
                            targetaddr = temp;
                        }
                    }
                    /* immediate / simple addressing */
                    else targetaddr = disp;
                }

                /* format 4 */
                else {
                    disp = ((memory[curraddr + 1] & 0x0F) << 16);
                    disp += (memory[curraddr + 2] << 8);
                    disp += memory[curraddr + 3];
                    targetaddr = disp;
                }

                /* indexed addressing */
                if(x == 1) targetaddr += X;
            }

            else if(bytenum == 2) disp = memory[curraddr + 1];
            else if(bytenum == 1) ;

            /* function for each instruction */
            if(!strcmp(mnemonic, "LDA")) {
                if(n == 0 && i == 1) A = targetaddr;
                else {
                    A = (memory[targetaddr] << 16);
                    A += (memory[targetaddr + 1] << 8);
                    A += memory[targetaddr + 2];
                }
            }

            else if(!strcmp(mnemonic, "LDB")) {
                if(n == 0 && i == 1) B = targetaddr;
                else {
                    B = (memory[targetaddr] << 16);
                    B += (memory[targetaddr + 1] << 8);
                    B += memory[targetaddr + 2];
                }
            }

            else if(!strcmp(mnemonic, "LDT")) {
                if(n == 0 && i == 1) T = targetaddr;
                else {
                    T = (memory[targetaddr] << 16);
                    T += (memory[targetaddr + 1] << 8);
                    T += memory[targetaddr + 2];
                }
            }

            else if(!strcmp(mnemonic, "LDCH")) A = (A & 0xFFFF00) + memory[targetaddr];

            else if(!strcmp(mnemonic, "STA")) {
                temp = A;         memory[targetaddr + 2] = temp % 256;
                temp = temp >> 8; memory[targetaddr + 1] = temp % 256;
                temp = temp >> 8; memory[targetaddr] = temp % 256;
            }

            else if(!strcmp(mnemonic, "STX")) {
                temp = X;         memory[targetaddr + 2] = temp % 256;
                temp = temp >> 8; memory[targetaddr + 1] = temp % 256;
                temp = temp >> 8; memory[targetaddr] = temp % 256;
            }

            else if(!strcmp(mnemonic, "STL")) {
                temp = L;         memory[targetaddr + 2] = temp % 256;
                temp = temp >> 8; memory[targetaddr + 1] = temp % 256;
                temp = temp >> 8; memory[targetaddr] = temp % 256;
            }

            else if(!strcmp(mnemonic, "STCH")) memory[targetaddr] = (A & 0xFF);

            else if(!strcmp(mnemonic, "J")) PC = targetaddr;

            else if(!strcmp(mnemonic, "JSUB")) { L = PC; PC = targetaddr; }

            else if(!strcmp(mnemonic, "JLT")) { if(comp == -1) PC = targetaddr; }

            else if(!strcmp(mnemonic, "JEQ")) { if(comp == 0) PC = targetaddr; }

            else if(!strcmp(mnemonic, "RSUB")) PC = L;


            else if(!strcmp(mnemonic, "COMP")) {
                if(A == targetaddr) comp = 0;
                else if(A > targetaddr) comp = 1;
                else comp = -1;
            }

            else if(!strcmp(mnemonic, "COMPR")) {
                switch((disp & 0xF0) >> 4) {
                    case 0 : firstreg = A; break;
                    case 1 : firstreg = X; break;
                    case 2 : firstreg = L; break;
                    case 3 : firstreg = B; break;
                    case 4 : firstreg = S; break;
                    case 5 : firstreg = T; break;
                }

                switch((disp & 0x0F) >> 4) {
                    case 0 : secondreg = A; break;
                    case 1 : secondreg = X; break;
                    case 2 : secondreg = L; break;
                    case 3 : secondreg = B; break;
                    case 4 : secondreg = S; break;
                    case 5 : secondreg = T; break;
                }

                if(firstreg == secondreg) comp = 0;
                else if(firstreg > secondreg) comp = 1;
                else comp = -1;
            }

            else if(!strcmp(mnemonic, "CLEAR")) {
                switch((disp & 0xF0) >> 4) {
                    case 0 : A = 0; break;
                    case 1 : X = 0; break;
                    case 2 : L = 0; break;
                    case 3 : B = 0; break;
                    case 4 : S = 0; break;
                    case 5 : T = 0; break;
                }
            }

            else if(!strcmp(mnemonic, "TIXR")) {
                switch((disp & 0xF0) >> 4) {
                    case 0 : firstreg = A; break;
                    case 1 : firstreg = X; break;
                    case 2 : firstreg = L; break;
                    case 3 : firstreg = B; break;
                    case 4 : firstreg = S; break;
                    case 5 : firstreg = T; break;
                }

                X++;
                if(X == firstreg) comp = 0;
                else if(X > firstreg) comp = 1;
                else comp = -1;
            }

            else if(!strcmp(mnemonic, "TD")) comp = 1;

            else if(!strcmp(mnemonic, "RD")) A &= 0xFFFF00;

            else if(!strcmp(mnemonic, "WD")) ;
        }

        if(PC >= MEGABYTE - 1) { PC = endaddr; endflag = 1; break; }
        if(!endflag) curraddr = PC;
        else curraddr = PCtemp;


    }

    printf("A : %06X X : %06X\n", A, X);
    printf("L : %06X PC: %06X\n", L, PC);
    printf("B : %06X S : %06X\n", B, S);
    printf("T : %06X\n", T);

    if(meetbpflag) {
        printf("Stop at checkpoint[%04X]\n", *currbp);
        *currbp = *nextbp;
        meetbpflag = 0;
    }

    if(PC >= 0xFFFFF || endflag) {
        printf("End the program.\n");
        PC = execaddr;
        L = endaddr;
        *currbp = *nextbp = -1;
    }
}
