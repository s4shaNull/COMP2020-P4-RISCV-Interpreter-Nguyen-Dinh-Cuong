#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "linkedlist.h"
#include "hashtable.h"
#include "riscv.h"

/************** BEGIN HELPER FUNCTIONS PROVIDED FOR CONVENIENCE ***************/
const int R_TYPE = 0;
const int I_TYPE = 1;
const int MEM_TYPE = 2;
const int U_TYPE = 3;
const int UNKNOWN_TYPE = 4;

/**
 * Return the type of instruction for the given operation
 * Available options are R_TYPE, I_TYPE, MEM_TYPE, UNKNOWN_TYPE
 */
static int get_op_type(char *op)
{
    const char *r_type_op[] = {"add", "sub", "and", "or", "xor", "slt", "sll", "sra"};
    const char *i_type_op[] = {"addi", "andi", "ori", "xori", "slti"};
    const char *mem_type_op[] = {"lw", "lb", "sw", "sb"};
    const char *u_type_op[] = {"lui"};
    for (int i = 0; i < (int)(sizeof(r_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(r_type_op[i], op) == 0)
        {
            return R_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(i_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(i_type_op[i], op) == 0)
        {
            return I_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(mem_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(mem_type_op[i], op) == 0)
        {
            return MEM_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(u_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(u_type_op[i], op) == 0)
        {
            return U_TYPE;
        }
    }
    return UNKNOWN_TYPE;
}
/*************** END HELPER FUNCTIONS PROVIDED FOR CONVENIENCE ****************/

registers_t *registers;
hashtable_t *table;
// TODO: create any additional variables to store the state of the interpreter

int reg_1;
int reg_2_imm;
int offset;
int temp;
int des_mem;
int des_reg;
int value_reg_1;
int value_reg_2;
char *temp_reg_1;
char *temp_reg_2;
char *temp_offset;
char *temp_des_reg;
char *temp_des_mem;

void init(registers_t *starting_registers)
{
    registers = starting_registers;
    // TODO: initialize any additional variables needed for state
    table = ht_init(4096);
}

// TODO: create any necessary helper functions

int get_decimal(char *token) {
	
	char *hex = "0x";
	char *negative_hex = "-0x";
	char *reg = "x";
	char temp_1[2];
	char temp_2[3];
	char temp_3[4];
	char *end;
	memset(temp_1, '\0', sizeof(temp_1));
	memset(temp_2, '\0', sizeof(temp_2));
	memset(temp_3, '\0', sizeof(temp_3));
	strncpy(temp_1, token, 1);
	strncpy(temp_2, token, 2);
	strncpy(temp_3, token, 3);

	if (strcmp(hex, temp_2) == 0 || strcmp(negative_hex, temp_3) == 0)
  {
		return (int) strtol(token, &end, 16);
	}

	else if (strcmp(reg, temp_1) == 0)
  {
		return (int) strtol(token+1, &end, 10);
	}

	else {
		return (int) strtol(token, &end, 10);
	}
}

char *trim(char* ins)
{
    while(isspace(*ins) || *ins == ',' || *ins == '(')
    {
        ins++;
    }
    return ins;
}

int standardize_i (int number)
{
    return number % 0x1000;
}

int sign_extend_i(int number) 
{
    if ((number & 0x00000800) != 0) {
            number = number | 0xfffff000;
    }
	  return number;
}

int sign_extend_lb(int number) 
{
	  if ((number & 0x00000080) != 0)
    {
		    number = number | 0xffffff00;
	  }
	  return number;
}

void step(char *instruction)
{
    // Extracts and returns the substring before the first space character,
    // by replacing the space character with a null-terminator.
    // `instruction` now points to the next character after the space
    // See `man strsep` for how this library function works
    instruction = trim(instruction);
    char *op = strsep(&instruction, " ");
    instruction = trim(instruction);
    // Uses the provided helper function to determine the type of instruction
    int op_type = get_op_type(op);
    // Skip this instruction if it is not in our supported set of instructions
    if (op_type == UNKNOWN_TYPE)
    {
        return;
    }
    // TODO: write logic for evaluating instruction on current interpreter state

    temp_des_reg = strsep(&instruction, " ,");
    des_reg = get_decimal(temp_des_reg);
    instruction = trim(instruction);
    
    if ((op_type == 0) || (op_type == 1))
    {
        if (des_reg == 0)
        {
            return;
        }
        
        temp_reg_1 = strsep(&instruction, " ,");
        reg_1 = get_decimal(temp_reg_1);
        instruction = trim(instruction);

        temp_reg_2 = strsep(&instruction, " ");
        reg_2_imm = get_decimal(temp_reg_2);

        value_reg_1 = registers->r[reg_1];

        if (op_type == 0)
        {
            value_reg_2 = registers->r[reg_2_imm];

            if (strcmp(op, "add") == 0)
            {
                registers->r[des_reg] = value_reg_1 + value_reg_2;
            }

            if (strcmp(op, "sub") == 0)
            {
                registers->r[des_reg] = value_reg_1 - value_reg_2;
            }

            if (strcmp(op, "and") == 0)
            {
                registers->r[des_reg] = value_reg_1 & value_reg_2;
            }

			      if (strcmp(op, "or") == 0) 
            {
                registers->r[des_reg] = value_reg_1 | value_reg_2;
            }

			      if (strcmp(op, "xor") == 0) 
            {
                registers->r[des_reg] = value_reg_1 ^ value_reg_2;
            }

			      if (strcmp(op, "nor") == 0) 
            {
                registers->r[des_reg] = ~(value_reg_1 | value_reg_2);
            }

			      if (strcmp(op, "slt") == 0) 
            {
                if (value_reg_1 < value_reg_2)
                {
					          registers->r[des_reg] = 1;
				        }
				        else
                {
					          registers->r[des_reg] = 0;
				        }
            }

            if (strcmp(op, "sll") == 0) 
            {
                registers->r[des_reg] = value_reg_1 << (0x0000001f & value_reg_2);
            }

			      if (strcmp(op, "sra") == 0) 
            {
                registers->r[des_reg] = value_reg_1 >> (0x0000001f & value_reg_2);
            }
        }

        if (op_type == 1) {
            reg_2_imm = standardize_i(reg_2_imm);
			      reg_2_imm = sign_extend_i(reg_2_imm);

			      if (strcmp(op, "addi") == 0) 
            {
                registers->r[des_reg] = value_reg_1 + reg_2_imm;
            }

			      if (strcmp(op, "andi") == 0)
            {
                registers->r[des_reg] = value_reg_1 & reg_2_imm;
            }

			      if (strcmp(op, "ori") == 0)
            {
                registers->r[des_reg] = value_reg_1 | reg_2_imm;
            }

     			  if (strcmp(op, "xori") == 0)
            {
                registers->r[des_reg] = value_reg_1 ^ reg_2_imm;
            }

			      if (strcmp(op, "slti") == 0)
            {
                if (value_reg_1 < reg_2_imm)
                {
                    registers->r[des_reg] = 1;
                }
                else
                {
                    registers->r[des_reg] = 0;
                }
            }
		    }
    }

    if (op_type == 2)
    {
        temp_offset = strsep(&instruction, " (");
        offset = get_decimal(temp_offset);
        instruction = trim(instruction);

        temp_des_mem = strsep(&instruction, " )");
        des_mem = registers->r[get_decimal(temp_des_mem)];

        if (strcmp(op, "lw") == 0)
        {
            if (des_reg == 0)
            {
                return;
            }

            temp = 0;

            for (int i = 3; i >= 1; i--)
            {
                temp +=ht_get(table, des_mem+offset+i);
                temp = temp << 8;
            }

            temp  += ht_get(table, des_mem+offset);
            registers->r[des_reg] = temp;
        }

        if (strcmp(op, "lb") == 0)
        {
            if (des_reg == 0)
            {
                return;
            }

            temp = ht_get(table, des_mem+offset);
            temp = sign_extend_lb(temp);
            registers->r[des_reg] = temp;
        }

        if (strcmp(op, "sw") == 0)
        {
            for (int i = 3; i >=0; i--)
            {
                ht_add(table, des_mem + offset + i, ((registers)->r[des_reg] >> 8*i) & 0x000000ff);
            }
        }

        if (strcmp(op, "sb") == 0)
        {
            temp = (registers->r[des_reg] & 0x000000ff);
            ht_add(table, des_mem+offset, temp);
        }
    }

    if (op_type == 3)
    {
        if (des_reg == 0)
        {
            return;
        }
        temp_reg_2 = strsep(&instruction, " ");
        reg_2_imm = get_decimal(temp_reg_2);
        registers->r[des_reg] = reg_2_imm << 12;
    }
}
