#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int PC = 0;
int labelled[10000] = {0}; // Adjust size as needed
int PC_to_label[10000] = {0}; // Adjust size as needed

// Function Prototypes
char* HexToBinary (const char*, char [][5]);
void initializeMap (char [][5]);
char* BinToAssembly (char*, const char*);
char* BinToReg (char*);
char* BinToImm (char*, int);
int BinToImmB (char*, int);
int BinToImmJ (char*, int);

int main (void) {
    freopen ("input.txt", "r", stdin);
    freopen ("output.txt", "w", stdout);
    FILE *read = fopen("input.txt", "r");

    char binmap[16][5];
    initializeMap(binmap);

    char s[10], *bin;
    char *lines[1000]; // Adjust size as needed

    int line_count = 0;
    while (fscanf(read, "%s", s) != EOF) {
        bin = HexToBinary(s, binmap);
        if (!labelled[PC]) labelled[PC] = 0;
        lines[line_count] = BinToAssembly(bin, s);
        PC += 4;
        line_count++;
    }

    // Assigning labels
    char text_label[100][10]; // Adjust size as needed
    int label_counter = 1;
    for (int pc = 0; pc < PC; pc += 4) {
        if (labelled[pc]) {        
            sprintf(text_label[label_counter], "L%d", label_counter);
            label_counter++;
            char temp[100];
            sprintf(temp, "%s: %s", text_label[label_counter - 1], lines[pc/4]);
            lines[pc/4] = strdup(temp);
        }
    }

    // Printing out the lines
    for (int pc = 0; pc < PC; pc += 4) {
        if (PC_to_label[pc]) {
            if (PC_to_label[pc] - 4 <= 0 || PC_to_label[pc] - 4 >= PC) {
                // Do nothing
            } else {
                for (int i = strlen(lines[pc/4]) - 1; i > -1; i--) {
                    if (lines[pc/4][i] == ' ') {
                        lines[pc/4][i + 1] = '\0';
                        break;
                    }
                }
                strcat(lines[pc/4], text_label[PC_to_label[pc] - 4]);
            }
        }
        printf("%s\n", lines[pc/4]);
    }
    fclose(read);
    return 0;
}

char* BinToAssembly (char* bin, const char* hex) {
    char opcode[8];
    strncpy(opcode, bin + 25, 7);
    opcode[7] = '\0';

    char* ins = malloc(100);
    strcpy(ins, "");
    char* invalid = "invalid";

    if (strcmp(opcode, "0110011") == 0) {
        char rs1[6], rs2[6], funct7[8], funct3[4], rd[6];
        strncpy(rs1, bin + 12, 5); rs1[5] = '\0';
        strncpy(rs2, bin + 7, 5); rs2[5] = '\0';
        strncpy(funct7, bin, 7); funct7[7] = '\0';
        strncpy(funct3, bin + 17, 3); funct3[3] = '\0';
        strncpy(rd, bin + 20, 5); rd[5] = '\0';

        if (strcmp(funct7, "0000000") == 0) {
            if (strcmp(funct3, "000") == 0) strcat(ins, "add ");
            else if (strcmp(funct3, "100") == 0) strcat(ins, "xor ");
            else if (strcmp(funct3, "110") == 0) strcat(ins, "or ");
            else if (strcmp(funct3, "111") == 0) strcat(ins, "and ");
            else if (strcmp(funct3, "001") == 0) strcat(ins, "sll ");
            else if (strcmp(funct3, "101") == 0) strcat(ins, "srl ");
            else if (strcmp(funct3, "010") == 0) strcat(ins, "slt ");
            else if (strcmp(funct3, "011") == 0) strcat(ins, "sltu ");
            else return invalid;
        } else if (strcmp(funct7, "0100000") == 0) {
            if (strcmp(funct3, "000") == 0) strcat(ins, "sub ");
            else if (strcmp(funct3, "101") == 0) strcat(ins, "sra ");
            else return invalid;
        } else return invalid;

        strcat(ins, BinToReg(rd));
        strcat(ins, ", ");
        strcat(ins, BinToReg(rs1));
        strcat(ins, ", ");
        strcat(ins, BinToReg(rs2));
        return ins;
    }

    else if (strcmp(opcode, "0010011") == 0 || strcmp(opcode, "0000011") == 0 || strcmp(opcode, "1100111") == 0) {
        char rs1[6], imm[13], funct3[4], rd[6];
        strncpy(rs1, bin + 12, 5); rs1[5] = '\0';
        strncpy(imm, bin, 12); imm[12] = '\0';
        strncpy(funct3, bin + 17, 3); funct3[3] = '\0';
        strncpy(rd, bin + 20, 5); rd[5] = '\0';
        
        int sign = 1;
        if (strcmp(opcode, "0010011") == 0) {
            if (strcmp(funct3, "000") == 0) strcat(ins, "addi ");
            else if (strcmp(funct3, "100") == 0) strcat(ins, "xori ");
            else if (strcmp(funct3, "110") == 0) strcat(ins, "ori ");
            else if (strcmp(funct3, "111") == 0) strcat(ins, "andi ");
            else if (strcmp(funct3, "001") == 0) strcat(ins, "slli ");
            else if (strcmp(funct3, "101") == 0) {
                if (strncmp(bin, "000000", 6) == 0) strcat(ins, "srli ");
                else strcat(ins, "srai ");
            } else if (strcmp(funct3, "010") == 0) strcat(ins, "slti ");
            else if (strcmp(funct3, "011") == 0) {
                strcat(ins, "sltiu ");
                sign = 0;
            } else return invalid;
        } else if (strcmp(opcode, "0000011") == 0) {
            sign = 1;
            if (strcmp(funct3, "000") == 0) strcat(ins, "lb ");
            else if (strcmp(funct3, "001") == 0) strcat(ins, "lh ");
            else if (strcmp(funct3, "010") == 0) strcat(ins, "lw ");
            else if (strcmp(funct3, "011") == 0) strcat(ins, "ld ");
            else if (strcmp(funct3, "100") == 0) {
                strcat(ins, "lbu ");
                sign = 0;
            } else if (strcmp(funct3, "101") == 0) {
                strcat(ins, "lhu ");
                sign = 0;
            } else if (strcmp(funct3, "110") == 0) {
                strcat(ins, "lwu ");
                sign = 0;
            } else return invalid;

            strcat(ins, BinToReg(rd));
            strcat(ins, ", ");
            strcat(ins, BinToImm(imm, sign));
            strcat(ins, "(");
            strcat(ins, BinToReg(rs1));
            strcat(ins, ")");
            return ins;
        } else if (strcmp(opcode, "1100111") == 0) {
            if (strcmp(funct3, "000") == 0) strcat(ins, "jalr ");
            else return invalid;
        }

        strcat(ins, BinToReg(rd));
        strcat(ins, ", ");
        strcat(ins, BinToReg(rs1));
        strcat(ins, ", ");
        strcat(ins, BinToImm(imm, sign));
        return ins;
    }

    else if (strcmp(opcode, "0100011") == 0) {
        char rs1[6], rs2[6], funct3[4], imm[13];
        strncpy(rs1, bin + 12, 5); rs1[5] = '\0';
        strncpy(rs2, bin + 7, 5); rs2[5] = '\0';
        strncpy(funct3, bin + 17, 3); funct3[3] = '\0';
        strncpy(imm, bin, 7); strncat(imm, bin + 20, 5); imm[12] = '\0';

        if (strcmp(funct3, "000") == 0) strcat(ins, "sb ");
        else if (strcmp(funct3, "001") == 0) strcat(ins, "sh ");
        else if (strcmp(funct3, "010") == 0) strcat(ins, "sw ");
        else if (strcmp(funct3, "011") == 0) strcat(ins, "sd ");
        else return invalid;

        strcat(ins, BinToReg(rs2));
        strcat(ins, ", ");
        strcat(ins, BinToImm(imm, 1));
        strcat(ins, "(");
        strcat(ins, BinToReg(rs1));
        strcat(ins, ")");
        return ins;
    }

    else if (strcmp(opcode, "1100011") == 0) {
        char rs1[6], rs2[6], funct3[4], imm[13];
        strncpy(rs1, bin + 12, 5); rs1[5] = '\0';
        strncpy(rs2, bin + 7, 5); rs2[5] = '\0';
        strncpy(funct3, bin + 17, 3); funct3[3] = '\0';
        strncpy(imm, bin, 1); strncat(imm, bin + 24, 1); strncat(imm, bin + 1, 6); strncat(imm, bin + 20, 4); imm[12] = '\0';

        int sign = 1;
        if (strcmp(funct3, "000") == 0) strcat(ins, "beq ");
        else if (strcmp(funct3, "001") == 0) strcat(ins, "bne ");
        else if (strcmp(funct3, "100") == 0) strcat(ins, "blt ");
        else if (strcmp(funct3, "101") == 0) {
            strcat(ins, "bge ");
            sign = 0;
        } else if (strcmp(funct3, "110") == 0) {
            strcat(ins, "bltu ");
            sign = 0;
        } else if (strcmp(funct3, "111") == 0) {
            strcat(ins, "bgeu ");
            sign = 0;
        } else return invalid;

        if (PC + BinToImmB(imm, sign) > -1) labelled[PC + BinToImmB(imm, sign)] = 1;
        PC_to_label[PC] = PC + BinToImmB(imm, 1) + 4;

        strcat(ins, BinToReg(rs1));
        strcat(ins, ", ");
        strcat(ins, BinToReg(rs2));
        strcat(ins, ", ");
        char imm_str[10];
        sprintf(imm_str, "%d", BinToImmB(imm, sign));
        strcat(ins, imm_str);
        return ins;
    }

    else if (strcmp(opcode, "1101111") == 0) {
        char rd[6], imm[21];
        strncpy(rd, bin + 20, 5); rd[5] = '\0';
        strncpy(imm, bin, 1); strncat(imm, bin + 12, 8); strncat(imm, bin + 11, 1); strncat(imm, bin + 1, 10); imm[20] = '\0';

        if (PC + BinToImmJ(imm, 1) > -1) labelled[PC + BinToImmJ(imm, 1)] = 1;
        PC_to_label[PC] = PC + BinToImmJ(imm, 1) + 4;

        strcat(ins, "jal ");
        strcat(ins, BinToReg(rd));
        strcat(ins, ", ");
        char imm_str[10];
        sprintf(imm_str, "%d", BinToImmJ(imm, 1));
        strcat(ins, imm_str);
        return ins;
    }

    else if (strcmp(opcode, "0110111") == 0) {
        char rd[6];
        strncpy(rd, bin + 20, 5); rd[5] = '\0';

        for (int i = 0; i < 5; i++) hex[i] = tolower(hex[i]);

        strcat(ins, "lui ");
        strcat(ins, BinToReg(rd));
        strcat(ins, ", 0x");
        strncat(ins, hex, 5);
        return ins;
    }

    else return invalid;
}

char* BinToReg (char* s) {
    int num = 0, x = 16;
    for (int i = 0; i < 5; i++) {
        if (s[i] == '1') num += x;
        x /= 2;
    }
    char* reg = malloc(5);
    sprintf(reg, "x%d", num);
    return reg;
}

char* BinToImm (char* s, int sign) {
    int num = 0, x = 2048;
    for (int i = 0; i < 12; i++) {
        if (s[i] == '1') num += x;
        x /= 2;
    }
    if (sign && num > 2047) num = 2047 - num;
    char* imm = malloc(10);
    sprintf(imm, "%d", num);
    return imm;
}

int BinToImmB (char* s, int sign) {
    int num = 0, x = 4096;
    for (int i = 0; i < 12; i++) {
        if (s[i] == '1') num += x;
        x /= 2;
    }
    if (sign && num > 4095) num = 4095 - num;
    return num;
}

int BinToImmJ (char* s, int sign) {
    int num = 0, x = 1048576;
    for (int i = 0; i < 20; i++) {
        if (s[i] == '1') num += x;
        x /= 2;
    }
    if (sign && num > 1048575) num = 1048575 - num;
    return num;
}

char* HexToBinary (const char* hex, char binmap[][5]) {
    char* bin = malloc(33);
    strcpy(bin, "");
    const char* bad = "00000000000000000000000000000000";
    for (int i = 0; i < strlen(hex); i++) {
        char lower_hex = tolower(hex[i]);
        if (!(lower_hex <= '9' && lower_hex >= '0') && !(lower_hex <= 'f' && lower_hex >= 'a')) {
            strcpy(bin, bad);
            break;
        }
        strcat(bin, binmap[lower_hex >= 'a' ? lower_hex - 'a' + 10 : lower_hex - '0']);
    }
    return bin;
}

void initializeMap (char binmap[][5]) {
    strcpy(binmap[1], "0001"); strcpy(binmap[5], "0101"); strcpy(binmap[9], "1001"); strcpy(binmap[13], "1101");
    strcpy(binmap[2], "0010"); strcpy(binmap[6], "0110"); strcpy(binmap[10], "1010"); strcpy(binmap[14], "1110");
    strcpy(binmap[3], "0011"); strcpy(binmap[7], "0111"); strcpy(binmap[11], "1011"); strcpy(binmap[15], "1111");
    strcpy(binmap[4], "0100"); strcpy(binmap[8], "1000"); strcpy(binmap[12], "1100"); strcpy(binmap[0], "0000");
}
