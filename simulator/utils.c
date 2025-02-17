#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

/* Unpacks the 32-bit machine code instruction given into the correct
 * type within the instruction struct */
Instruction parse_instruction(uint32_t instruction_bits) {
  /* YOUR CODE HERE */
  Instruction instruction;
  // add x9, x20, x21   hex: 01 5A 04 B3, binary = 0000 0001 0101 1010 0000 0100 1011 0011
  // Opcode: 0110011 (0x33) Get the Opcode by &ing 0x1111111, bottom 7 bits
  instruction.opcode = instruction_bits & ((1U << 7) - 1);

  // Shift right to move to pointer to interpret next fields in instruction.
  instruction_bits >>= 7;

  switch (instruction.opcode) {
  // R-Type
  case 0x33:
    // instruction: 0000 0001 0101 1010 0000 0100 1, destination : 01001
    instruction.rtype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // instruction: 0000 0001 0101 1010 0000, func3 : 000
    instruction.rtype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    // instruction: 0000 0001 0101 1010 0, src1: 10100
    instruction.rtype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // instruction: 0000 0001 0101, src2: 10101
    instruction.rtype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // funct7: 0000 000
    instruction.rtype.funct7 = instruction_bits & ((1U << 7) - 1);
    break;
  // cases for other types of instructions
  /* YOUR CODE HERE */

  //I-type
  case 0x13:
    instruction.itype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    instruction.itype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.imm = instruction_bits & ((1U << 12) - 1);
    break;

  //S-Type
  case 0x23:
    instruction.stype.imm5 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.stype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;
 
    instruction.stype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.stype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.stype.imm7 = instruction_bits & ((1U << 7) - 1);
    break;

  //SB-Type
  case 0x63:
    // immediate part 1 (imm5) is next 5 higher bits:
    instruction.sbtype.imm5 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // funct3 is next 3 higher bits
    instruction.sbtype.funct3 = instruction_bits & ((1U << 3) - 1); 
    instruction_bits >>= 3;

    // rs1 is next 5 higher bits
    instruction.sbtype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // rs2 is next 5 higher bits
    instruction.sbtype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // immediate part 2 (imm7) is highest 7 bits 
    instruction.sbtype.imm7 = instruction_bits & ((1U << 7) - 1);
    break;

  // UJ-Type  
  case 0x6F:  
    // rd is next 7 higher bits
    instruction.ujtype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;
    
    // immediate is highest 20 bits
    instruction.ujtype.imm = instruction_bits & ((1U << 20) - 1);
    //instruction_bits >> = 20;  
    break;
  
  //U-Type
  case 0x37:
    instruction.utype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.utype.imm = instruction_bits & ((1U << 20) - 1);
    break;

   case 0x3:
    instruction.itype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    instruction.itype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.imm = instruction_bits & ((1U << 12) - 1);
    break;

   //ecall
   case 0x73:
    instruction.rest = instruction_bits & ((1U << 7) - 1);
    break;




  #ifndef TESTING
  default:
    exit(EXIT_FAILURE);
  #endif
  }
  return instruction;
}

/************************Helper functions************************/
/* Here, you will need to implement a few common helper functions, 
 * which you will call in other functions when parsing, printing, 
 * or executing the instructions. */

/* Sign extends the given field to a 32-bit integer where field is
 * interpreted an n-bit integer. */
int sign_extend_number(unsigned int field, unsigned int n) {
  /* YOUR CODE HERE */
  //check msb
  int x = (field >> (n-1)) & 1;
  if (x){//if msb is 1, the number is negative and the sign extension will add all 1s at higher bits
  	return field | (~((1<<n)-1));// OR with all 1s at higher bits and n zeros, to get the field and 1s at higher bits
  }  
  else{
  	return field & ((1<<n)-1);// and with only n 1s to get just the n-bit field with all the zeroes at higher bits
  }
}

/* Return the number of bytes (from the current PC) to the branch label using
 * the given branch instruction */
int get_branch_offset(Instruction instruction) {
  /* YOUR CODE HERE */
	int imm = 0;

	imm = imm | ((instruction.sbtype.imm5 & 0x1) << 11); // First extracting the 11th bit for LSB value of the immediate (left shift by 11 bits)
	imm = imm | ((instruction.sbtype.imm5 & 0x1e));	// Extracting bits 1 to 4 (doing AND operation with 0x1e)
  	imm = imm | ((instruction.sbtype.imm7 & 0x3f) << 5); // Extracting bits 5 to 10 (left shift by 5 then doing AND operation with 0x3F)
        imm = imm | ((instruction.sbtype.imm7 & 0x40) << 12); // Extracting the 12th bit for the MSB vlaue of the immediate (left shift by 12)
  	return sign_extend_number(imm,12);
}

/* Returns the number of bytes (from the current PC) to the jump label using the
 * given jump instruction */
int get_jump_offset(Instruction instruction) {
  /* YOUR CODE HERE */
	int imm = 0;

	imm  = ((instruction.ujtype.imm & (0x3FF << 9)) >> 9 ) << 1;	// 10:0
	imm |= ((instruction.ujtype.imm & (0x1 << 8)) >> 8) << 11;
	imm |= ((instruction.ujtype.imm & (0xFF))) << 12;
	imm |= ((instruction.ujtype.imm & (0x1 << 19)) >> 19) << 20;
	imm = sign_extend_number(imm,21);

	return imm;
}

/* Returns the number of bytes (from the current PC) to the base address using the
 * given store instruction */
int get_store_offset(Instruction instruction) {
  /* YOUR CODE HERE */
	int imm = 0;
	imm = imm | (instruction.stype.imm5 & 0x01f); //extract imm[4..0] from lower 5 bits
	imm = imm | ((instruction.stype.imm7 & 0x07f) << 5);// extract imm[11..5] and left shit by 5 to make them appear as upper 7 bits
	imm = sign_extend_number(imm, 12); //sign extend the 12-bit value to a 32 bit number
  return imm;
}
/************************Helper functions************************/

void handle_invalid_instruction(Instruction instruction) {
  printf("Invalid Instruction: 0x%08x\n", instruction.bits);
}

void handle_invalid_read(Address address) {
  printf("Bad Read. Address: 0x%08x\n", address);
  exit(-1);
}

void handle_invalid_write(Address address) {
  printf("Bad Write. Address: 0x%08x\n", address);
  exit(-1);
}


//1111 1111 0111 1110 0000
//1111 1111 1111 1110 0000

