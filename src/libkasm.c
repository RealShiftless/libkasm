#include "libkasm.h"

#include <string.h>
#include "lexer.h"
//#include "parser.h"
//#include "assembler.h"
//#include "opcodes.h"


// Lexer Func
// Checks if the token is a label definition (e.g., "@start:")
static uint8_t parse_label_def(char* token, uint8_t length) {
    return (token[0] == '@' && token[length - 1] == ':');
}

// Checks if the token is a label reference (e.g., "@start")
static uint8_t parse_label_ref(char* token, uint8_t length) {
    return (token[0] == '@');
}

// Checks if the token is a directive (e.g., ".data", ".text")
static uint8_t parse_directive(char* token, uint8_t length) {
    return (token[0] == '.');
}

// Checks if the token is a valid instruction (matches an opcode)
static uint8_t parse_instruction(char* token, uint8_t length) {
    for (int i = 0; i < length; i++) {
        // We enable the 6th bit here, to set the string to lower, we then check if it falls
        // inbetween the range for a-z, if so we continue, if not we return 0
        char chr = token[i] | 0x20;
        if (chr >= 'a' && chr <= 'z')
            continue;

        return 0;
    }
    return 1;
}

// Number parsing helper
static uint8_t parse_number(char* token, uint8_t length, uint8_t isHexadecimal) {
    for (int i = 0; i < length; i++) {
        char chr = token[i] | 0x20;
        if (!((chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'f' && isHexadecimal))) {
            return 0; // invalid hex character
        }
    }
    return 1;
}

// Checks if the token is a valid register (e.g., r0, r1, ..., r5)
static uint8_t parse_register(char* token, uint8_t length) {

    if (token[0] == 'p' && token[1] == 'c' && length == 2) {
        return 1;
    }
    else if(token[0] == 's' && token[1] == 'p' && length == 2) {
        return 1;
    }

    if (token[0] != 'r')
        return 0;

    return parse_number(&token[1], length - 1, 0);
}

// Checks if the token represents an immediate value (e.g., #10, #0xFF)
static uint8_t parse_immediate(char* token, uint8_t length) {
    if(token[0] != '#') {
        return 0;
    }
    
    if (token[1] == '0' && (token[2] | 0x20) == 'x') { // Hex
        return parse_number(&token[3], length - 3, 1);
    }
    else {
        return parse_number(&token[1], length - 1, 0);
    }

}

// Checks if the token represents an address (e.g., $1000, $FF)
static uint8_t parse_address(char* token, uint8_t length) {
    // Needs at least "$0x0" = 4 chars
    if (length < 4) return 0;

    // Must start with '$0X'
    if (token[0] != '$' || token[1] != '0' || ((token[2] | 0x20) != 'x')) {
        return 0;
    }

    return parse_number(&token[3], length - 3, 1);
}

// Checks if the token is a comma (used to separate operands)
static uint8_t parse_comma(char* token, uint8_t length) {
    return (token[0] == ',' && length == 1);
}

// Checks if the token is an end-of-line (newline character '\n')
static uint8_t parse_eol(char* token, uint8_t length) {
    return (token[0] == '\n' && length == 1);
}

// Parse string
static uint8_t parse_string(char* token, uint8_t length) {
    return (token[0] == '"' && token[length - 1] == '"');
}

static TokenTypeDef gTokenTypes[] = {
    [TOKEN_LABEL_DEF]   = { parse_label_def,   TOKEN_FLAG_LABEL,  TOKEN_FLAG_EOL,                       TOKEN_FLAG_EOL },
    [TOKEN_DIRECTIVE]   = { parse_directive,   TOKEN_FLAG_ACTION, TOKEN_FLAG_EOL,                       TOKEN_FLAG_VALUE | TOKEN_FLAG_STRING | TOKEN_FLAG_EOL },
    [TOKEN_INSTRUCTION] = { parse_instruction, TOKEN_FLAG_ACTION, TOKEN_FLAG_EOL,                       TOKEN_FLAG_VALUE | TOKEN_FLAG_EOL },
    [TOKEN_REGISTER]    = { parse_register,    TOKEN_FLAG_VALUE,  TOKEN_FLAG_ACTION | TOKEN_FLAG_COMMA, TOKEN_FLAG_COMMA | TOKEN_FLAG_EOL },
    [TOKEN_IMMEDIATE]   = { parse_immediate,   TOKEN_FLAG_VALUE,  TOKEN_FLAG_ACTION | TOKEN_FLAG_COMMA, TOKEN_FLAG_COMMA | TOKEN_FLAG_EOL },
    [TOKEN_ADDRESS]     = { parse_immediate,   TOKEN_FLAG_VALUE,  TOKEN_FLAG_ACTION | TOKEN_FLAG_COMMA, TOKEN_FLAG_COMMA | TOKEN_FLAG_EOL },
    [TOKEN_LABEL_REF]   = { parse_label_ref,   TOKEN_FLAG_VALUE,  TOKEN_FLAG_ACTION | TOKEN_FLAG_COMMA, TOKEN_FLAG_COMMA | TOKEN_FLAG_EOL },
    [TOKEN_COMMA]       = { parse_comma,       TOKEN_FLAG_COMMA,  TOKEN_FLAG_VALUE,                     TOKEN_FLAG_VALUE },
    [TOKEN_STRING]      = { parse_string,      TOKEN_FLAG_STRING, TOKEN_FLAG_ACTION,                    TOKEN_FLAG_EOL},
    [TOKEN_EOL]         = { parse_eol,         TOKEN_FLAG_EOL,    0b111011 /* all but comma */,         TOKEN_FLAG_LABEL | TOKEN_FLAG_ACTION | TOKEN_FLAG_EOL }
};

uint8_t parse_token_type(char* value, uint8_t length, KasmTokenType* tokenType) {
    for (uint8_t i = 1; i < TOKEN_MAX; i++) {
        if (!gTokenTypes[i].can_parse(value, length))
            continue;

        *tokenType = i;
        return 0;
    }
    return 1;
}

TokenTypeDef* get_token_type_def(KasmTokenType type) {
    return &gTokenTypes[type];
}

const char* get_token_type_name(KasmTokenType type) {
    switch (type) {
        case TOKEN_UNKNOWN:     return "Unknown";
        case TOKEN_LABEL_DEF:   return "Label Definition";
        case TOKEN_DIRECTIVE:   return "Directive";
        case TOKEN_INSTRUCTION: return "Instruction";
        case TOKEN_REGISTER:    return "Register";
        case TOKEN_IMMEDIATE:   return "Immediate";
        case TOKEN_ADDRESS:     return "Address";
        case TOKEN_LABEL_REF:   return "Label Refrence";
        case TOKEN_COMMA:       return "Comma";
        case TOKEN_STRING:      return "String";
        case TOKEN_EOL:         return "End Of Line";
        default:                return "???";
    }
}


// Parser Func
static DirectiveTypeDef gDirectiveTypes[] = {
    [DIRECTIVE_ORG]  = { .name = "org",  .serializeArguments = 0 },
    [DIRECTIVE_BANK] = { .name = "bank", .serializeArguments = 0 },
    [DIRECTIVE_DB]   = { .name = "db",   .serializeArguments = 1 }
};

uint8_t parse_directive_type(const char* value, DirectiveType* type) {
    for (uint16_t i = 0; i < DIRECTIVE_MAX; i++) {
        if(strcmp(value, gDirectiveTypes[i].name)) {
            continue;
        }

        *type = i;
        return 0;   // Success
    }

    return 1;
}

uint8_t parse_opcode_type(BuildContext* context, char* value, uint16_t* opcodeId) {
    for (uint16_t i = 0; i < context->target->opcodeCount; i++) {
        OpcodeDef* opcode = context->target->get_opcode(i);
        if (opcode->mnemonic == NULL) {
            continue;
        }

        if (strcmp(value, opcode->mnemonic)) {
            continue;
        }

        *opcodeId = i;
        return 0;
    }

    return 1;
}


// Build Func
uint8_t kasm_build(const char* input, const char* output, BuildContext* context) {
    context->buildState = BUILD_STATE_LOAD_FILE;
    FILE* file = fopen(input, "r");
    if (!file) {
        context->assemblerResult = BUILD_RESULT_FILE_ERROR;
        return 1;
    }

    // Allocate the list of tokens
    context->buildState = BUILD_STATE_ALLOC_TOKENS;

    List tokens = { 0 };
    list_init(&tokens);

    // Tokenize the stream
    context->buildState = BUILD_STATE_TOKENIZE;
    LexerResult result;
    if ((context->tokenizerResult = lex(file, &tokens)) != LEXER_OK) {
        switch (context->tokenizerResult) {
        case LEXER_TOKEN_OVERFLOW:
            context->assemblerResult = BUILD_RESULT_BUFFER_OVERFLOW;
            break;

        case LEXER_TOKEN_UNKNOWN:
            context->assemblerResult = BUILD_RESULT_SYNTAX_ERROR;
            break;

        case LEXER_ALLOC_FAILED:
            context->assemblerResult = BUILD_RESULT_ALLOC_FAILED;
            break;

        case LEXER_STREAM_ERROR:
            context->assemblerResult = BUILD_RESULT_FILE_ERROR;
            break;

        default:
            context->assemblerResult = BUILD_RESULT_UNKOWN_ERROR;
            break;
        }

        free(tokens.values);
        return 1;
    }

    // Close current file, we need to open a new one for writting :)
    if (ferror(file)) {
        free(tokens.values);
        fclose(file);

        context->assemblerResult = BUILD_RESULT_FILE_ERROR;
        return 1;
    }

    fclose(file);

    // Start actually parsing the tokens and writing bytes to a file

    // Finalize
    context->buildState = BUILD_STATE_FINALIZE;
    free(tokens.values);
}