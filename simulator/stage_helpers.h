#ifndef __STAGE_HELPERS_H__
#define __STAGE_HELPERS_H__

#include <stdio.h>
#include "utils.h"
#include "pipeline.h"

/// EXECUTE STAGE HELPERS ///

/**
 * input  : idex_reg_t
 * output : uint32_t alu_control signal
 **/
uint32_t gen_alu_control(idex_reg_t idex_reg)
{
  uint32_t alu_control = 0;
  /**
   * YOUR CODE HERE
   */
  switch(idex_reg.ALUOp){
	  case 0x0: //ADD
		  alu_control = 0x2;//Add(0010)
		  break;
	  case 0x1: //SUB
		  alu_control = 0x6;//Subtract(0110)
		  break;	  
	  case 0x2: //r-type instructions
		  switch(idex_reg.instr.rtype.funct3){
		  	case 0x0://add or subtract
				switch(idex_reg.instr.rtype.funct7){
					case 0x0://add
						alu_control = 0x2;
						break;
					case 0x20://subtract
						alu_control = 0x6;
						break;
					case 0x01://mul
						alu_control = 0xA;
						break;
				}
				break;
			case 0x01://sll
				alu_control = 0x4;
				break;
			case 0x2://slt
				alu_control = 0xC;
				break;
			case 0x4:
				switch(idex_reg.instr.rtype.funct7){
					case 0x00://xor
						alu_control = 0x8;
						break;
					case 0x01://div
						alu_control = 0xB;
						break;
				}
				break;
			case 0x5:
				switch(idex_reg.instr.rtype.funct7){
					case 0x00://srl
						alu_control = 0x7;
						break;
					case 0x20://sra
						alu_control = 0xD;
						break;
				}
				break;
			case 0x7://and
				alu_control = 0x0;//And(0000)
				break;
			case 0x6:
				switch(idex_reg.instr.rtype.funct7){
					case 0x00://or
						alu_control = 0x1;
						break;
					case 0x01://rem
						alu_control = 0x9;
						break;
				}
				break;
		  	}
		 	break;
	  case 0x3://i-type instructions
			switch(idex_reg.instr.itype.funct3){
				case 0x0://addi
					alu_control = 0x2;
					break;
				case 0x1://slli
					alu_control = 0x4;
					break;
				case 0x2://slti
					alu_control = 0xC;
					break;
				case 0x4://xori
					alu_control = 0x8;
					break;
				case 0x5:
					switch(sign_extend_number((idex_reg.instr.itype.imm >> 5) & 0x7f,7)){
						case 0x00://srli
							alu_control = 0x7;
							break;
						case 0x20://srai
							alu_control = 0xD;
							break;
					}
					break;
				case 0x6://ori
					alu_control = 0x1;
					break;
				case 0x7://andi
					alu_control = 0x0;
					break;
			}
		  break;
	  case 0x4://utype instructions
		  alu_control = 0x3;
		  break;
	  case 0x5://ujtype
		  alu_control = 0xE;
		  break;
	  default:
      		alu_control = 0xBADCAFFE;//error code
     		break;

  }
  return alu_control;
}

/**
 * input  : alu_inp1, alu_inp2, alu_control
 * output : uint32_t alu_result
 **/
uint32_t execute_alu(uint32_t alu_inp1, uint32_t alu_inp2, uint32_t alu_control)
{
  uint32_t result;
  switch(alu_control){
    /**
     * YOUR CODE HERE
     */
    case 0x0://and
	    result = alu_inp1 & alu_inp2;
	    break;
    case 0x1://or
	    result = alu_inp1 | alu_inp2;
	    break;
    case 0x2: //add
      	    result = alu_inp1 + alu_inp2;
     	    break;
    case 0x6://subtract
            result = alu_inp1 - alu_inp2;
	    break;
    case 0x3://LUI
	    result = alu_inp2;
    	    break;
   case 0x4://left shift
	    result = alu_inp1 << alu_inp2;
	    break;
   case 0x7://right shift
	    result = alu_inp1 >> alu_inp2;
	    break;
   case 0x8://xor
	    result = alu_inp1 ^ alu_inp2;
	    break;
   case 0x9://rem
	    result = alu_inp1 % alu_inp2;
	    break;
   case 0xA://multiply
	    result = alu_inp1 * alu_inp2;
	    break;
   case 0xB://divide
	    result = alu_inp1 / alu_inp2;
	    break;
   case 0xC://slt
	    result = (int32_t)alu_inp1 < (int32_t)alu_inp2 ? 1:0;
            break;
   case 0xD://sra
	    if (alu_inp1 & 0x80000000){
	    	result = (alu_inp1 >> alu_inp2) | (0xFFFFFFFF << (32 - alu_inp2));
	    } else {
	    	result = alu_inp1 >> alu_inp2;
	    }
	    break;
   case 0xE:
	    result = alu_inp1 + 4;//rd = pc + 4 for jal
	    break;
    default:
      result = 0xBADCAFFE;
      break;
  };
  return result;
}

/// DECODE STAGE HELPERS ///

/**
 * input  : Instruction
 * output : idex_reg_t
 **/
uint32_t gen_imm(Instruction instruction)
{
  int imm_val = 0;
  /**
   * YOUR CODE HERE
   */
  switch(instruction.opcode) {
        case 0x63: //B-type
            imm_val = get_branch_offset(instruction);
            break;
        /**
         * YOUR CODE HERE
         */
	case 0x13://I-type
	    imm_val = sign_extend_number(instruction.itype.imm, 12);
	    break;
	case 0x03://I-type load instructions
	    imm_val = sign_extend_number(instruction.itype.imm, 12);
	    break;
	case 0x23://S-type
	    imm_val = get_store_offset(instruction);
	    break;
	case 0x37://U-type
	    imm_val = instruction.utype.imm << 12;
	    break;
	case 0x6f://UJ-type
	    imm_val = get_jump_offset(instruction);
	    break;
        default: // R and undefined opcode
            break;
    };
    return imm_val;
}

/**
 * generates all the control logic that flows around in the pipeline
 * input  : Instruction
 * output : idex_reg_t
 **/
idex_reg_t gen_control(Instruction instruction)
{
  idex_reg_t idex_reg = {0};
  switch(instruction.opcode) {
      case 0x33:  //R-type
        /**
         * YOUR CODE HERE
         */
	  idex_reg.ALUSrc 	= 	0;
	  idex_reg.MemtoReg 	= 	0;
	  idex_reg.RegWrite 	= 	1;
	  idex_reg.MemRead 	= 	0;
	  idex_reg.MemWrite	= 	0;
	  idex_reg.Branch 	= 	0;
	  idex_reg.ALUOp 	= 	0x2;//ALUOp for Rtype instr: 10
          break;

      case 0x03:  //I-type load instructions
	  idex_reg.ALUSrc 	= 	1;
	  idex_reg.MemtoReg 	= 	1;
	  idex_reg.RegWrite 	= 	1;
	  idex_reg.MemRead 	= 	1;
	  idex_reg.MemWrite	= 	0;
	  idex_reg.Branch 	= 	0;
	  idex_reg.ALUOp 	= 	0x0;//ALUOp for ADD: 00
	  break;

      case 0x13:  //I-type instructions
	  idex_reg.ALUSrc 	= 	1;
	  idex_reg.MemtoReg 	= 	0;
	  idex_reg.RegWrite 	= 	1;
	  idex_reg.MemRead 	= 	0;
	  idex_reg.MemWrite	= 	0;
	  idex_reg.Branch 	= 	0;
	  idex_reg.ALUOp 	= 	0x3;//ALUOp for Itype instr: 11
	  break;

	        
      case 0x23:  //S-type instructions
	  idex_reg.ALUSrc 	= 	1;
	  idex_reg.MemtoReg 	= 	0;
	  idex_reg.RegWrite 	= 	0;
	  idex_reg.MemRead 	= 	0;
	  idex_reg.MemWrite	= 	1;
	  idex_reg.Branch 	= 	0;
	  idex_reg.ALUOp 	= 	0x0;//ALUOp for ADD: 00
	  break;

      case 0x63:  //SB-type instructions
	  idex_reg.ALUSrc 	= 	0;
	  idex_reg.MemtoReg 	= 	0;
	  idex_reg.RegWrite 	= 	0;
	  idex_reg.MemRead 	= 	0;
	  idex_reg.MemWrite	= 	0;
	  idex_reg.Branch 	= 	1;
	  idex_reg.ALUOp 	= 	0x1;//ALUOp for SUB: 01
	  break;

      case 0x37:  //U-type instructions
	  idex_reg.ALUSrc 	= 	1;
	  idex_reg.MemtoReg 	= 	0;	  
	  idex_reg.RegWrite 	= 	1;
	  idex_reg.MemRead 	= 	0;
	  idex_reg.MemWrite	= 	0;
	  idex_reg.Branch 	= 	0;
	  idex_reg.ALUOp 	= 	0x4;//ALUOp for lui: 100
	  break;

      case 0x6f:  //UJ-type instructions
	  idex_reg.ALUSrc 	= 	0;
	  idex_reg.MemtoReg 	= 	0;	  
	  idex_reg.RegWrite 	= 	1;
	  idex_reg.MemRead 	= 	0;
	  idex_reg.MemWrite	= 	0;
	  idex_reg.Branch 	= 	0;
	  idex_reg.ALUOp 	= 	0x5;//ALUOp for JAL
	  break;


	  
      default:  // Remaining opcodes
          break;
  }
  return idex_reg;
}

/// MEMORY STAGE HELPERS ///

/**
 * evaluates whether a branch must be taken
 * input  : <open to implementation>
 * output : bool
 **/
bool gen_branch(bool zero, unsigned int funct3)
{
  /**
   * YOUR CODE HERE
   */
  
  bool takeBranch = false; 
  	switch(funct3){
		  // beq
		  case (0x0):
			  //printf("checking beq condition\n:");
			  if (zero){
				  //printf("BRANCH TAKEN\n");
		  		takeBranch = true;
			  } else {
				   //printf("BRANCH NOT TAKEN\n");
			  	takeBranch = false;
			  }
		  	  break;
		  // bne	  
		  case (0x1):
			  if (!zero){
			  	takeBranch = true;
			  } else {
			  	takeBranch = false;
			  }
			  break;

	  	  default:
			  break;

 	 }
  
  return takeBranch;
}


/// PIPELINE FEATURES ///

/**
 * Task   : Sets the pipeline wires for the forwarding unit's control signals
 *           based on the pipeline register values.
 * input  : pipeline_regs_t*, pipeline_wires_t*
 * output : None
*/
void gen_forward(pipeline_regs_t* pregs_p, pipeline_wires_t* pwires_p)
{
  /**
   * YOUR CODE HERE
   */
	//conditions for ex and mem hazards

	//printf("exmem RegWrite = %d\n",pregs_p->exmem_preg.out.RegWrite);
	//printf("memwb RegWrite = %d\n",pregs_p->memwb_preg.out.RegWrite);


  bool ex_haz_rs1 = (pregs_p->exmem_preg.out.RegWrite) && (pregs_p->exmem_preg.out.rd != 0) && (pregs_p->exmem_preg.out.rd == pregs_p->idex_preg.out.rs1);

  bool ex_haz_rs2 = (pregs_p->exmem_preg.out.RegWrite) && (pregs_p->exmem_preg.out.rd != 0) && (pregs_p->exmem_preg.out.rd == pregs_p->idex_preg.out.rs2);

  bool mem_haz_rs1 = (pregs_p->memwb_preg.out.RegWrite) && (pregs_p->memwb_preg.out.rd != 0) && (pregs_p->memwb_preg.out.rd == pregs_p->idex_preg.out.rs1);

  bool mem_haz_rs2 = (pregs_p->memwb_preg.out.RegWrite) && (pregs_p->memwb_preg.out.rd != 0) && (pregs_p->memwb_preg.out.rd == pregs_p->idex_preg.out.rs2);

  if(pregs_p->exmem_preg.out.instr_addr == pregs_p->idex_preg.out.instr_addr){
  	ex_haz_rs1 = 0;
	ex_haz_rs2 = 0;
  }

  // EX hazard
  // RS1
    if (ex_haz_rs1){
	    
	printf("[FWD]: Resolving EX hazard on rs1: x%d\n", pregs_p->idex_preg.out.rs1);
	
	pregs_p->idex_preg.out.rs1Value = pregs_p->exmem_preg.out.alu_result;
	//printf("%08x\n", );
	fwd_exex_counter++;
  }
  // RS2
    if (ex_haz_rs2){
	printf("[FWD]: Resolving EX hazard on rs2: x%d\n", pregs_p->idex_preg.out.rs2);

	pregs_p->idex_preg.out.rs2Value = pregs_p->exmem_preg.out.alu_result;

	fwd_exex_counter++;
  }
  // MEM hazard
  // RS1
    if (mem_haz_rs1 && !ex_haz_rs1){
	printf("[FWD]: Resolving MEM hazard on rs1: x%d\n", pregs_p->idex_preg.out.rs1);
	if(pregs_p->memwb_preg.out.MemtoReg){	
		pregs_p->idex_preg.out.rs1Value = pregs_p->memwb_preg.out.ReadData;
	}else {

		pregs_p->idex_preg.out.rs1Value = pregs_p->memwb_preg.out.alu_result;
	}
	fwd_exmem_counter++;
  }
  // RS2
    if (mem_haz_rs2 && !ex_haz_rs2){
	printf("[FWD]: Resolving MEM hazard on rs2: x%d\n", pregs_p->idex_preg.out.rs2);

	if(pregs_p->memwb_preg.out.MemtoReg){	
		pregs_p->idex_preg.out.rs2Value = pregs_p->memwb_preg.out.ReadData;
	}else{

		pregs_p->idex_preg.out.rs2Value = pregs_p->memwb_preg.out.alu_result;
	}
	
	fwd_exmem_counter++;

  }


}



/**
 * Task   : Sets the pipeline wires for the hazard unit's control signals
 *           based on the pipeline register values.
 * input  : pipeline_regs_t*, pipeline_wires_t*
 * output : None
*/
void detect_hazard(pipeline_regs_t* pregs_p, pipeline_wires_t* pwires_p, regfile_t* regfile_p)
{
  /**
   * YOUR CODE HERE
   */
		// exec normally if pc_hold is true
		if (pwires_p->pc_hold) {
			pregs_p->ifid_preg.out.instr = pregs_p->idex_preg.out.instr;
			pregs_p->ifid_preg.out.instr_addr = pregs_p->idex_preg.out.instr_addr;

			//pregs_p->exmem_preg.inp.RegWrite = pregs_p->idex_preg.out.RegWrite;
			//pregs_p->exmem_preg.out.MemRead = pregs_p->idex_preg.out.MemRead;
  			//pregs_p->exmem_preg.out.MemtoReg = pregs_p->idex_preg.out.MemtoReg;
  			//pregs_p->exmem_preg.out.MemWrite = pregs_p->idex_preg.out.MemWrite;
  			//pregs_p->memwb_preg.out.RegWrite = pregs_p->idex_preg.out.RegWrite;
			//pregs_p->memwb_preg.out.RegWrite = 0;

			pwires_p->pc_hold = 0;
		}
	
  	if(pregs_p->idex_preg.out.MemRead && ((pregs_p->idex_preg.out.rd == pregs_p->ifid_preg.out.rs1) || (pregs_p->idex_preg.out.rd == pregs_p->ifid_preg.out.rs2))){
	      // insert bubble to stall pipeline
	      // write zero to control wires
	  pregs_p->idex_preg.inp.ALUSrc 	= 	0;
	  pregs_p->idex_preg.inp.MemtoReg 	= 	0;
	  pregs_p->idex_preg.inp.RegWrite 	= 	0;
	  pregs_p->idex_preg.inp.MemRead 	= 	0;
	  pregs_p->idex_preg.inp.MemWrite	= 	0;
	  pregs_p->idex_preg.inp.Branch 	= 	0;
	  pregs_p->idex_preg.inp.ALUOp 		=	0;
	  pwires_p->pc_hold = 1;

	  

	  // print load-use hazard
	printf("[HZD]: Stalling and rewriting PC: 0x%08x\n", regfile_p->PC);

	  // decrement PC to prevent progression to next instruction
	  //printf("pcsrc0 = %x\n",pwires_p->pc_src0);
	  pwires_p->pc_src0 -= 4;
	  //printf("pcsrc0 = %x\n",pwires_p->pc_src0);
	  // increment stall counter
	//printf(" PC: 0x%08x\n", regfile_p->PC);
	  
	  stall_counter++;
      }

}

// define helper function for uj/sb-type instructions
void handle_control_haz(pipeline_regs_t* pregs_p, pipeline_wires_t* pwires_p, regfile_t* regfile_p){
      // Control hazard (check opcode of memwb register to check if it is jal, jalr)  
  bool jal = (pregs_p->memwb_preg.out.instr.opcode == 0x6F);
  bool jalr = (pregs_p->memwb_preg.out.instr.opcode == 0x67);
	if(jal || jalr){
		// flush pipeline by setting nops
		printf("[CPL]: Pipeline Flushed\n");
		ifid_reg_t ifid_reg = {0};
		idex_reg_t idex_reg = {0};
		exmem_reg_t exmem_reg = {0};

		pregs_p->ifid_preg.out  = ifid_reg;
  		pregs_p->idex_preg.out  = idex_reg;
  		pregs_p->exmem_preg.out = exmem_reg;

		pregs_p->ifid_preg.out.instr  = parse_instruction(0x13);
  		pregs_p->idex_preg.out.instr  = parse_instruction(0x13);
  		pregs_p->exmem_preg.out.instr = parse_instruction(0x13);

		
		pregs_p->ifid_preg.out.instr_addr  = regfile_p->PC;
  	        pregs_p->idex_preg.out.instr_addr  = regfile_p->PC - 4;
  	        pregs_p->exmem_preg.out.instr_addr  = regfile_p->PC - 8;


		// jump to specified instruction by editing PC
		if(jal){
			regfile_p->PC += get_jump_offset(pregs_p->memwb_preg.out.instr) - 16;
		}else if (jalr){
			regfile_p->PC += get_jump_offset(pregs_p->memwb_preg.out.instr) - 16;
		}

		regfile_p->R[0] = 0;

		branch_counter++;

	}

	// Control hazard (check opcode of memwb register to see if it is branch type)
	if(pregs_p->memwb_preg.out.instr.opcode == 0x63){

		if(gen_branch(pregs_p->memwb_preg.out.Zero, pregs_p->memwb_preg.out.instr.sbtype.funct3)) {

			// flush pipeline by setting nops
			printf("[CPL]: Pipeline Flushed\n");
			ifid_reg_t ifid_reg = {0};
			idex_reg_t idex_reg = {0};
			exmem_reg_t exmem_reg = {0};

			pregs_p->ifid_preg.out  = ifid_reg;
  			pregs_p->idex_preg.out  = idex_reg;
  			pregs_p->exmem_preg.out = exmem_reg;

			pregs_p->ifid_preg.out.instr  = parse_instruction(0x13);
  			pregs_p->idex_preg.out.instr  = parse_instruction(0x13);
  			pregs_p->exmem_preg.out.instr = parse_instruction(0x13);

			pregs_p->ifid_preg.out.instr_addr  = regfile_p->PC;
  		        pregs_p->idex_preg.out.instr_addr  = regfile_p->PC - 4;
  		        pregs_p->exmem_preg.out.instr_addr  = regfile_p->PC - 8;

						
			regfile_p->PC += get_branch_offset(pregs_p->memwb_preg.out.instr) - 12;
			branch_counter++;


		}
		else{
			regfile_p->PC = pregs_p->ifid_preg.out.instr_addr + 4;
		}
		regfile_p->R[0] = 0;
	}
		
}


///////////////////////////////////////////////////////////////////////////////


/// RESERVED FOR PRINTING REGISTER TRACE AFTER EACH CLOCK CYCLE ///
void print_register_trace(regfile_t* regfile_p)
{
  // print
  for (uint8_t i = 0; i < 8; i++)       // 8 columns
  {
    for (uint8_t j = 0; j < 4; j++)     // of 4 registers each
    {
      printf("r%2d=%08x ", i * 4 + j, regfile_p->R[i * 4 + j]);
    }
    printf("\n");
  }
  printf("\n");
}

#endif // __STAGE_HELPERS_H__
