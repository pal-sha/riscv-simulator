#include <stdbool.h>
#include "cache.h"
#include "riscv.h"
#include "types.h"
#include "utils.h"
#include "pipeline.h"
#include "stage_helpers.h"

uint64_t total_cycle_counter = 0;
uint64_t mem_access_counter = 0;
uint64_t miss_count = 0;
uint64_t hit_count = 0;
uint64_t stall_counter = 0;
uint64_t branch_counter = 0;
uint64_t fwd_exex_counter = 0;
uint64_t fwd_exmem_counter = 0;

simulator_config_t sim_config = {0};

///////////////////////////////////////////////////////////////////////////////

void bootstrap(pipeline_wires_t* pwires_p, pipeline_regs_t* pregs_p, regfile_t* regfile_p)
{
  // PC src must get the same value as the default PC value
  pwires_p->pc_src0 = regfile_p->PC;
  pwires_p->pc_hold = 0;
}

///////////////////////////
/// STAGE FUNCTIONALITY ///
///////////////////////////

/**
 * STAGE  : stage_fetch
 * output : ifid_reg_t
 **/ 
ifid_reg_t stage_fetch(pipeline_wires_t* pwires_p, regfile_t* regfile_p, Byte* memory_p)
{
  ifid_reg_t ifid_reg = {0};
  /**
   * YOUR CODE HERE
   */
    //update PC for next instruction
  //if(!pwires_p->pc_hold){
  	if(pwires_p->pcsrc){
  		regfile_p->PC = pwires_p->pc_src1;//take the branch address
 	 }
  	else{
	  	regfile_p->PC = pwires_p->pc_src0;
  	}
 // }

  	//fetch instruction from memory using PC
  	uint32_t instruction_bits = *(uint32_t *)(memory_p + regfile_p->PC);
  	//printf("%p %d\n", memory_p, regfile_p->PC);
	  	//write the instruction bits to ifid 
  	ifid_reg.instr.bits = instruction_bits;

  	ifid_reg.instr_addr = regfile_p->PC;
	
      	#ifdef DEBUG_CYCLE
  	printf("[IF ]: Instruction [%08x]@[%08x]: ", ifid_reg.instr.bits, regfile_p->PC);
  	decode_instruction(ifid_reg.instr.bits);
  	#endif

	//if(!pwires_p->pc_hold){
  		pwires_p->pc_src0 = regfile_p->PC + 4;
	//}

    Instruction instr = ifid_reg.instr;

    switch(ifid_reg.instr.opcode){
	  case 0x33://rtype		  
		  ifid_reg.rs1 = instr.rtype.rs1;
		  ifid_reg.rs2 = instr.rtype.rs2;
		  break;
	  case 0x13://itype
		  ifid_reg.rs1 = instr.itype.rs1;
		  ifid_reg.rs2 = 0;
		  break;
	  case 0x3://load
		  ifid_reg.rs1 = instr.itype.rs1;
		  ifid_reg.rs2 = 0;  
		  break;
	  case 0x23://stype
		  ifid_reg.rs1 = instr.stype.rs1;
		  ifid_reg.rs2 = instr.stype.rs2;	  
		  break;
	  case 0x63://sbtype
		  ifid_reg.rs1 = instr.stype.rs1;
		  ifid_reg.rs2 = instr.stype.rs2;	  
		  break;
	  default:
		  break;
    }

  return ifid_reg;
}

/**
 * STAGE  : stage_decode
 * output : idex_reg_t
 **/ 
idex_reg_t stage_decode(ifid_reg_t ifid_reg, pipeline_wires_t* pwires_p, regfile_t* regfile_p)
{
  idex_reg_t idex_reg = {0};
  /**
   * YOUR CODE HERE
   */

  //decode the instruction
  Instruction instr = ifid_reg.instr;
  #ifdef DEBUG_CYCLE
  printf("[ID ]: Instruction [%08x]@[%08x]: ", instr.bits, ifid_reg.instr_addr);
  decode_instruction(instr.bits);
  #endif

  if(idex_reg. instr. opcode == 0x73){
  	pwires_p->ecall = true;
  }
  else{
  	pwires_p->ecall = false;
  }
    //get control signals
  if(!pwires_p->pc_hold){
  	idex_reg = gen_control(instr);
  }

  //if(idex_reg.instr.opcode == 0x73){
  //	pwires_p->ecall = 1;
  //}

  //read register file for rs1 and rs2 values if required
  switch(instr.opcode){
	  case 0x33://rtype		  
		  idex_reg.rs1 = instr.rtype.rs1;
		  idex_reg.rs2 = instr.rtype.rs2;

		  idex_reg.rs1Value = regfile_p->R[instr.rtype.rs1];
		  idex_reg.rs2Value = regfile_p->R[instr.rtype.rs2];
		  idex_reg.rd = instr.rtype.rd;
		  break;
	  case 0x13://itype
		  idex_reg.rs1 = instr.itype.rs1;
		  idex_reg.rs2 = 0;

		  idex_reg.rs1Value = regfile_p->R[instr.itype.rs1];
		  idex_reg.rs2Value = 0;
		  idex_reg.rd = instr.itype.rd;
		  break;
	  case 0x3://load
		  idex_reg.rs1 = instr.itype.rs1;
		  idex_reg.rs2 = 0;

		  idex_reg.rs1Value = regfile_p->R[instr.itype.rs1];
		  idex_reg.rs2Value = 0;
		  idex_reg.rd = instr.itype.rd;		  
		  break;
	  case 0x23://stype
		  idex_reg.rs1 = instr.stype.rs1;
		  idex_reg.rs2 = instr.stype.rs2;
		  idex_reg.rs1Value = regfile_p->R[instr.stype.rs1];
		  idex_reg.rs2Value = regfile_p->R[instr.stype.rs2];
		  idex_reg.rd = 0;		  
		  break;
	  case 0x63://sbtype
		  idex_reg.rs1 = instr.stype.rs1;
		  idex_reg.rs2 = instr.stype.rs2;
		  idex_reg.rs1Value = regfile_p->R[instr.sbtype.rs1];
		  idex_reg.rs2Value = regfile_p->R[instr.sbtype.rs2];
		  idex_reg.rd = 0;		  
		  break;
	  case 0x37://utype
		  idex_reg.rs1Value = 0;
		  idex_reg.rs2Value = 0;
		  idex_reg.rd = instr.utype.rd;
		  break;
	  case 0x6F://ujtype
		  idex_reg.rs1Value = ifid_reg.instr_addr;
		  idex_reg.rs2Value = 0;
		  idex_reg.rd = instr.ujtype.rd;
		  break;
	  default:
		  break;
  }
  //get imm
  idex_reg.imm = gen_imm(instr);
  //update idex reg
  idex_reg.instr = instr;
  idex_reg.instr_addr = ifid_reg.instr_addr;


  return idex_reg;
}

/**
 * STAGE  : stage_execute
 * output : exmem_reg_t
 **/
exmem_reg_t stage_execute(idex_reg_t idex_reg, pipeline_wires_t* pwires_p)
{
  exmem_reg_t exmem_reg = {0};
  /**
   * YOUR CODE HERE
   */
  //generate alu control signals
  uint32_t alu_control = gen_alu_control(idex_reg);

  //execute alu
  uint32_t alu_result = execute_alu(idex_reg.rs1Value, idex_reg.ALUSrc ? idex_reg.imm : idex_reg.rs2Value, alu_control);
  bool zero = (alu_result == 0);
  exmem_reg.Zero = zero;

  //branch condition?
  exmem_reg.Branch = idex_reg.Branch;

  //pass the results
  exmem_reg.alu_result = alu_result;
  exmem_reg.rs1Value = idex_reg.rs1Value;
  exmem_reg.rs2Value = idex_reg.rs2Value;
  exmem_reg.imm = idex_reg.imm;
  exmem_reg.rd = idex_reg.rd;

  //exmem_reg.Branch = idex_reg.Branch;
  exmem_reg.MemRead = idex_reg.MemRead;
  exmem_reg.MemtoReg = idex_reg.MemtoReg;
  exmem_reg.MemWrite = idex_reg.MemWrite;
  exmem_reg.RegWrite = idex_reg.RegWrite;

  exmem_reg.instr = idex_reg.instr;
  exmem_reg.instr_addr = idex_reg.instr_addr;

  #ifdef DEBUG_CYCLE
  printf("[EX ]: Instruction [%08x]@[%08x]: ", idex_reg.instr.bits, idex_reg.instr_addr);
  decode_instruction(idex_reg.instr.bits);
  #endif


  return exmem_reg;
}

/**
 * STAGE  : stage_mem
 * output : memwb_reg_t
 **/ 
memwb_reg_t stage_mem(exmem_reg_t exmem_reg, pipeline_wires_t* pwires_p, Byte* memory_p, Cache* cache_p){
	memwb_reg_t memwb_reg = {0};
  	/**
   	* YOUR CODE HERE
   	*/
  	// Passing on control signals
  	memwb_reg.RegWrite = exmem_reg.RegWrite; 
  	memwb_reg.MemtoReg = exmem_reg.MemtoReg;
	memwb_reg.MemWrite = exmem_reg.MemWrite;
	memwb_reg.rd = exmem_reg.rd;
	memwb_reg.Branch = exmem_reg.Branch;
	memwb_reg.rs1Value = exmem_reg.rs1Value;
	memwb_reg.rs2Value = exmem_reg.rs2Value;
	memwb_reg.Zero = exmem_reg.Zero;
  
  	// Pass through results
 	memwb_reg.alu_result = exmem_reg.alu_result;

  	memwb_reg.instr = exmem_reg.instr;
  	memwb_reg.instr_addr = exmem_reg.instr_addr;

	if(memwb_reg.Branch){//branch instruction
		bool branch_condition = gen_branch(exmem_reg.Zero, memwb_reg.instr.sbtype.funct3);
		if(branch_condition){
			pwires_p->pcsrc = 1;
			pwires_p->pc_src1 = exmem_reg.instr_addr + (exmem_reg.imm);//branch target address
  		}
		else{
			pwires_p->pcsrc = 0;
		}

	}
	
	else if(exmem_reg.instr.opcode == 0x6F){
 		pwires_p->pcsrc = 1;
		pwires_p->pc_src1 = exmem_reg.instr_addr + (exmem_reg.imm);
  	}
  	else{
  		pwires_p->pcsrc = 0;
  	}

	 #ifdef DEBUG_CYCLE
  	printf("[MEM]: Instruction [%08x]@[%08x]: ", exmem_reg.instr.bits, exmem_reg.instr_addr);
  	decode_instruction(exmem_reg.instr.bits);
  	#endif

	// check if cache is enabled in simulation configuration
	
	if (sim_config.cache_en){
	
		if ((exmem_reg.MemWrite || exmem_reg.MemRead) && !pwires_p->ecall){ //exmem_reg.instr.bits != 0x00000073){
			
			int cache_result = processCacheOperation((uint64_t)exmem_reg.alu_result, cache_p);
		
			if (cache_result == CACHE_HIT_LATENCY){
				total_cycle_counter += (CACHE_HIT_LATENCY - 1);
				hit_count++;
			} else if (cache_result == CACHE_MISS_LATENCY){
				total_cycle_counter += (CACHE_MISS_LATENCY - 1);
				miss_count++;
			} else {
				total_cycle_counter += (CACHE_OTHER_LATENCY - 1);
				miss_count++;
			}

			#ifdef PRINT_CACHE_TRACES
			printf("[MEM]: Cache latency at addr: 0x%.8x: %d cycles\n", exmem_reg.alu_result, cache_result);
			#endif

			// add cache latency to total cycle counter: 
		        // adjust total cycle counter to account for current operation
			//total_cycle_counter += (cache_result - 1);

		}
	
	} else { // if cache is not enabled, use default memory latency
		
		// incrementing memory access counter
		mem_access_counter++;

		// add default memory latency to total memory access counter
		// adjust total cycle counter to account for current operation
		total_cycle_counter += (MEM_LATENCY - 1);
		
	}
  
	if(exmem_reg.MemWrite){
		if(exmem_reg.instr.opcode == 0x23){
			switch(exmem_reg.instr.stype.funct3){
				case 0x0://sb
					*(uint32_t*)(memory_p + exmem_reg.alu_result) = exmem_reg.rs2Value & 0xFF;
					break;
				case 0x1://sh
					*(uint32_t*)(memory_p + exmem_reg.alu_result) = exmem_reg.rs2Value & 0xFFFF;
					break;
				case 0x2://sw
					*(uint32_t*)(memory_p + exmem_reg.alu_result) = exmem_reg.rs2Value;
					break;
			}
		}
		else{
			*(uint32_t*)(memory_p + exmem_reg.alu_result) = exmem_reg.rs2Value;
		}
	}
	if(exmem_reg.MemRead){
		if(exmem_reg.instr.opcode == 0x3){
			switch(exmem_reg.instr.itype.funct3){
				case 0x0:
					memwb_reg.ReadData = (int32_t)(int8_t)*(uint32_t*)(memory_p + exmem_reg.alu_result);
					break;

				case 0x1:
					memwb_reg.ReadData = (int32_t)(int16_t)*(uint32_t*)(memory_p + exmem_reg.alu_result);
					break;

				case 0x2:
					memwb_reg.ReadData = *(uint32_t*)(memory_p + exmem_reg.alu_result);
					break;
			}

			
			
			
		}
		else{
			memwb_reg.ReadData = *(uint32_t*)(memory_p + exmem_reg.alu_result);
		}
	}

  return memwb_reg;
}

/**
 * STAGE  : stage_writeback
 * output : nothing - The state of the register file may be changed
 **/ 
void stage_writeback(memwb_reg_t memwb_reg, pipeline_wires_t* pwires_p, regfile_t* regfile_p)
{
  /**
   * YOUR CODE HERE
   */
	//if(!pwires_p->ecall){

		if(memwb_reg.RegWrite){
			//if(pwires_p->ecall){
			//	regfile_p->R[memwb_reg.rd] = 0;
			//}
			if(memwb_reg.MemtoReg){
				if(memwb_reg.instr.opcode == 0x3){
					switch(memwb_reg.instr.itype.funct3){
						case 0x0://lb
							regfile_p->R[memwb_reg.rd] = sign_extend_number(memwb_reg.ReadData,8);// & 0xFF;
							break;
						case 0x1://lh
							regfile_p->R[memwb_reg.rd] = sign_extend_number(memwb_reg.ReadData,16);// & 0xFFFF;
							break;
						case 0x2:
							regfile_p->R[memwb_reg.rd] = memwb_reg.ReadData;
							break;
				
					}
				}
				else{
					regfile_p->R[memwb_reg.rd] = memwb_reg.ReadData;

				}
			}
			else{
				if(memwb_reg.instr.opcode == 0x6F && memwb_reg.rd == 0){
					regfile_p->R[memwb_reg.rd] = 0;
				}
				else{
					regfile_p->R[memwb_reg.rd] = memwb_reg.alu_result;

				}		
			}
		
	}
  #ifdef DEBUG_CYCLE
  printf("[WB ]: Instruction [%08x]@[%08x]: ", memwb_reg.instr.bits, memwb_reg.instr_addr);
  decode_instruction(memwb_reg.instr.bits);
  #endif

}

///////////////////////////////////////////////////////////////////////////////

/** 3
 * excite the pipeline with one clock cycle
 **/
void cycle_pipeline(regfile_t* regfile_p, Byte* memory_p, Cache* cache_p, pipeline_regs_t* pregs_p, pipeline_wires_t* pwires_p, bool* ecall_exit)
{
  #ifdef DEBUG_CYCLE
  printf("v==============");
  printf("Cycle Counter = %5ld", total_cycle_counter);
  printf("==============v\n\n");
  #endif
  //printf("pcsrc_0 : %d\n", pwires_p->pc_src0);

  // process each stage

  /* Output               |    Stage      |       Inputs  */
  pregs_p->ifid_preg.inp  = stage_fetch     (pwires_p, regfile_p, memory_p);

  detect_hazard(pregs_p, pwires_p, regfile_p);

  if(pwires_p->pc_hold){
  	pregs_p->idex_preg.inp.instr = pregs_p->ifid_preg.out.instr;
	pregs_p->idex_preg.inp.instr_addr = pregs_p->ifid_preg.out.instr_addr;
	//pregs_p->exmem_preg.out.RegWrite = pregs_p->idex_preg.out.RegWrite;


	  //decode the instruction
  	Instruction instr = pregs_p->ifid_preg.out.instr;
  	#ifdef DEBUG_CYCLE
  	printf("[ID ]: Instruction [%08x]@[%08x]: ", instr.bits, pregs_p->ifid_preg.out.instr_addr);
  	decode_instruction(instr.bits);
  	#endif

  }
  else{
  
  	pregs_p->idex_preg.inp  = stage_decode    (pregs_p->ifid_preg.out, pwires_p, regfile_p);

  }
  
  gen_forward(pregs_p, pwires_p);

  pregs_p->exmem_preg.inp = stage_execute   (pregs_p->idex_preg.out, pwires_p);

  pregs_p->memwb_preg.inp = stage_mem       (pregs_p->exmem_preg.out, pwires_p, memory_p, cache_p);

                            stage_writeback (pregs_p->memwb_preg.out, pwires_p, regfile_p);

  // update all the output registers for the next cycle from the input registers in the current cycle
  pregs_p->ifid_preg.out  = pregs_p->ifid_preg.inp;
  pregs_p->idex_preg.out  = pregs_p->idex_preg.inp;
  pregs_p->exmem_preg.out = pregs_p->exmem_preg.inp;
  pregs_p->memwb_preg.out = pregs_p->memwb_preg.inp;

  if (sim_config.fwd_en){
  	handle_control_haz(pregs_p, pwires_p, regfile_p);
  }


  /////////////////// NO CHANGES BELOW THIS ARE REQUIRED //////////////////////

  // increment the cycle
  total_cycle_counter++;

  #ifdef DEBUG_REG_TRACE
  print_register_trace(regfile_p);
  #endif

  /**
   * check ecall condition
   * To do this, the value stored in R[10] (a0 or x10) should be 10.
   * Hence, the ecall condition is checked by the existence of following
   * two instructions in sequence:
   * 1. <instr>  x10, <val1>, <val2> 
   * 2. ecall
   * 
   * The first instruction must write the value 10 to x10.
   * The second instruction is the ecall (opcode: 0x73)
   * 
   * The condition checks whether the R[10] value is 10 when the
   * `memwb_reg.instr.opcode` == 0x73 (to propagate the ecall)
   * 
   * If more functionality on ecall needs to be added, it can be done
   * by adding more conditions on the value of R[10]
   */
  if( (pregs_p->memwb_preg.out.instr.bits == 0x00000073) &&
      (regfile_p->R[10] == 10) )
  {
    *(ecall_exit) = true;
  }
}

