#include "libkasm.h"
#include <stdio.h>

#define TARGET_NAME     "km8"
#define VERSION         "v0.1.0"
#define OPCODE_COUNT    256


// Operand Type Ting
static OperandType op_reg_mem[]  = { OPERAND_REG, OPERAND_MEM };
static OperandType op_reg_imm[]  = { OPERAND_REG, OPERAND_IMM };
static OperandType op_reg_reg[]  = { OPERAND_REG, OPERAND_REG };
static OperandType op_reg[]     = { OPERAND_REG };
static OperandType op_mem[]      = { OPERAND_MEM };


static OpcodeDef gOpcodes[OPCODE_COUNT] = {
    // Data
    [0x00] = { .mnemonic = "nop",  .operandCount = 0, .operands = NULL },
    [0x01] = { .mnemonic = "ldr",  .operandCount = 2, .operands = op_reg_mem},
    [0x02] = { .mnemonic = "ldr",  .operandCount = 2, .operands = op_reg_imm},
    [0x03] = { .mnemonic = "str",  .operandCount = 2, .operands = op_reg_mem},
    [0x04] = { .mnemonic = "mov",  .operandCount = 2, .operands = op_reg_reg},
    [0x05] = { .mnemonic = "swp",  .operandCount = 2, .operands = op_reg_reg},
    [0x06] = { .mnemonic = "push", .operandCount = 2, .operands = op_reg },
    [0x07] = { .mnemonic = "pop",  .operandCount = 2, .operands = op_reg },
    [0x08] = { .mnemonic = "clr",  .operandCount = 2, .operands = op_reg },

    // Arithmetic
    [0x10] = { .mnemonic = "add",  .operandCount = 2, .operands = op_reg_reg },
    [0x11] = { .mnemonic = "add",  .operandCount = 2, .operands = op_reg_imm },
    [0x12] = { .mnemonic = "adc",  .operandCount = 2, .operands = op_reg_reg },
    [0x13] = { .mnemonic = "adc",  .operandCount = 2, .operands = op_reg_imm },
    [0x14] = { .mnemonic = "inc",  .operandCount = 1, .operands = op_reg },
    [0x15] = { .mnemonic = "sub",  .operandCount = 2, .operands = op_reg_reg },
    [0x16] = { .mnemonic = "sub",  .operandCount = 2, .operands = op_reg_imm },
    [0x17] = { .mnemonic = "sbc",  .operandCount = 2, .operands = op_reg_reg },
    [0x18] = { .mnemonic = "sbc",  .operandCount = 2, .operands = op_reg_imm },
    [0x19] = { .mnemonic = "dec",  .operandCount = 1, .operands = op_reg },
    [0x1A] = { .mnemonic = "cmp",  .operandCount = 2, .operands = op_reg_reg },
    [0x1B] = { .mnemonic = "cmp",  .operandCount = 2, .operands = op_reg_imm },

    // Bitwise
    [0x20] = { .mnemonic = "add",  .operandCount = 2, .operands = op_reg_reg },
    [0x21] = { .mnemonic = "add",  .operandCount = 2, .operands = op_reg_reg },
    [0x22] = { .mnemonic = "or",   .operandCount = 2, .operands = op_reg_reg },
    [0x23] = { .mnemonic = "or",   .operandCount = 2, .operands = op_reg_reg },
    [0x24] = { .mnemonic = "xor",  .operandCount = 2, .operands = op_reg_reg },
    [0x25] = { .mnemonic = "xor",  .operandCount = 2, .operands = op_reg_reg },
    [0x26] = { .mnemonic = "not",  .operandCount = 2, .operands = op_reg_reg },
    [0x27] = { .mnemonic = "shl",  .operandCount = 2, .operands = op_reg_reg },
    [0x28] = { .mnemonic = "shr",  .operandCount = 2, .operands = op_reg_reg },
    [0x29] = { .mnemonic = "rol",  .operandCount = 2, .operands = op_reg_reg },
    [0x2A] = { .mnemonic = "ror",  .operandCount = 2, .operands = op_reg_reg },
    [0x2B] = { .mnemonic = "tst",  .operandCount = 2, .operands = op_reg_reg },
    [0x2C] = { .mnemonic = "tst",  .operandCount = 2, .operands = op_reg_reg },

    // Conditionals
    [0x30] = { .mnemonic = "jmp",  .operandCount = 1, .operands = op_mem },
    [0x31] = { .mnemonic = "jmp",  .operandCount = 1, .operands = op_reg },
    [0x32] = { .mnemonic = "jz",   .operandCount = 1, .operands = op_mem },
    [0x33] = { .mnemonic = "jz",   .operandCount = 1, .operands = op_reg },
    [0x34] = { .mnemonic = "jnz",  .operandCount = 1, .operands = op_mem },
    [0x35] = { .mnemonic = "jnz",  .operandCount = 1, .operands = op_reg },
    [0x36] = { .mnemonic = "jc",   .operandCount = 1, .operands = op_mem },
    [0x37] = { .mnemonic = "jc",   .operandCount = 1, .operands = op_reg },
    [0x38] = { .mnemonic = "jnc",  .operandCount = 1, .operands = op_mem },
    [0x39] = { .mnemonic = "jnc",  .operandCount = 1, .operands = op_reg },
    [0x3A] = { .mnemonic = "jn",   .operandCount = 1, .operands = op_mem },
    [0x3B] = { .mnemonic = "jn",   .operandCount = 1, .operands = op_reg },
    [0x3C] = { .mnemonic = "jnn",  .operandCount = 1, .operands = op_mem },
    [0x3D] = { .mnemonic = "jnn",  .operandCount = 1, .operands = op_reg },
    [0x3E] = { .mnemonic = "jv",   .operandCount = 1, .operands = op_mem },
    [0x3F] = { .mnemonic = "jv",   .operandCount = 1, .operands = op_reg },
    [0x40] = { .mnemonic = "jnv",  .operandCount = 1, .operands = op_mem },
    [0x41] = { .mnemonic = "jnv",  .operandCount = 1, .operands = op_reg },
    [0x42] = { .mnemonic = "call", .operandCount = 1, .operands = op_mem },
    [0x43] = { .mnemonic = "call", .operandCount = 1, .operands = op_reg },
    [0x44] = { .mnemonic = "ret",  .operandCount = 0, .operands = NULL },
    [0x45] = { .mnemonic = "hlt",  .operandCount = 0, .operands = NULL },
};

static OpcodeDef* get_opcode(uint16_t index) {
    return &gOpcodes[index];
}

static void assemble_impl(const char* input) {
    printf("Hello, Build Target!\n%s\n", input);

    for(uint16_t i = 0; i < OPCODE_COUNT; i++) {
        OpcodeDef* opcode = get_opcode(i);

        if(opcode->mnemonic == NULL) {
            continue;
        }

        printf("%02X %s\n", i, opcode->mnemonic);
    }
}



static uint16_t get_operand_size(OperandType operand) {
    switch(operand) {
        case OPERAND_IMM: return 1;
        case OPERAND_REG: return 1;
        case OPERAND_MEM: return 2;
        default:          return 0;
    }
}

__declspec(dllexport)
BuildTarget* kasm_target_register() {
    static BuildTarget target = {
        .name = TARGET_NAME,
        .version = VERSION,
        .opcodeCount = OPCODE_COUNT,
        .assemble = assemble_impl,
        .get_opcode = get_opcode,

        .registerCount = 14,
        .immediateSize = 1,
        .addressSize   = 2
    };
    return &target;
}