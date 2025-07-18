#include "parser.h"

// The context, I have to free this
ParserContext* gParserContext;

// Uses the given rules to validate a token sequence
static uint8_t validate_token_sequence(TokenTypeDef* base, TokenTypeDef* preceding, TokenTypeDef* succeeding) {
	uint8_t pAllowFlag = base->precedingFlag;
	uint8_t sAllowFlag = base->succeedingFlag;

	uint8_t pFlag = preceding->typeFlag;
	uint8_t sFlag = succeeding->typeFlag;

	return (pAllowFlag & pFlag) && (sAllowFlag & sFlag);
}

// Allocates
static ParserResult define_label(const char* name) {
    Label* label = malloc(sizeof(Label));
    if(label == NULL) {
        return PARSER_ALLOC_FAILED;
    }

    label->name = name;
    label->position = gParserContext->currentPosition;

    list_add(&gParserContext->build->labels, label);
    return PARSER_OK;
}

static ParserResult parse_directive(Token* token) {
    if(gParserContext->currentActionType != ACTION_TYPE_NONE) {
        return PARSER_MULTIPLE_ACTIONS_ERROR;
    }

    gParserContext->currentActionType = ACTION_TYPE_DIRECTIVE;

    if(parse_directive_type(token->value, &gParserContext->currentActionType)) {
        return PARSER_INVALID_DIRECTIVE;
    }

    return PARSER_OK;
}

static ParserResult parse_instruction(Token* token) {
    if(gParserContext->currentActionType != ACTION_TYPE_NONE) {
        return PARSER_MULTIPLE_ACTIONS_ERROR;
    }

    if(parse_opcode_type(gParserContext->build, token->value, &gParserContext->currentActionType) == 0) {
        gParserContext->currentActionType = ACTION_TYPE_OPCODE;
        return PARSER_OK;
    }

    // TODO: Add macro support

    return PARSER_INVALID_INSTRUCTION;
}

static ParserResult parse_immediate(Token* token) {
    // Create the value
    uint32_t value = (uint32_t)strtoul(token->value[1], NULL, 0);

    // We calculate the size of this immediate
    uint8_t size;

    if (value <= 0xFF)          size = 1;
    else if (value <= 0xFFFF)   size = 2;
    else                        size = 4;

    // If we are out of the range of the given target throw an error
    if(gParserContext->build->target->immediateSize != size) {
        return PARSER_IMMEDIATE_OUT_OF_RANGE;
    }

    // Allocate the argument
    Argument* argument = malloc(sizeof(Argument));
    if(argument == NULL) {
        return LIST_ALLOC_FAILED;
    }

    // Create an immediate
    argument->type = ARGUMENT_IMMEDIATE;
    argument->value = value;

    // Add it
    list_add(&gParserContext->currentArguments, argument);
}

static ParserResult parse_token(Token* token) {
	switch(token->type) {
        case TOKEN_LABEL_DEF:       return define_label(token->value);
        case TOKEN_DIRECTIVE:       return parse_directive(token);
        case TOKEN_INSTRUCTION:     return parse_instruction(token);
        case TOKEN_IMMEDIATE:       return parse_immediate(token);
        case TOKEN_REGISTER:        return parse_register(token);
        case TOKEN_ADDRESS:         return parse_address(token);
        case TOKEN_LABEL_REF:       return parse_label(token);
    }
}

ParserResult kasm_parse(BuildContext* buildContext) {
    // Init the relevant lists
    if(list_init(&buildContext->actions) != LIST_OK) {
        return PARSER_ALLOC_FAILED;
    }
    
    if(list_init(&buildContext->labels)) {
        return PARSER_ALLOC_FAILED;
    }

    // If gParserContext is not null we can assume we can free it
    if(gParserContext != NULL) {
        free(gParserContext);
        gParserContext = NULL;
    }

    // Allocate a parser context, check if we succeeded
    gParserContext = calloc(1, sizeof(ParserContext));

    if(gParserContext == NULL) {
        return PARSER_ALLOC_FAILED;
    }

    // Initialize the argument list
    if(list_init(&gParserContext->currentArguments)) {
        return PARSER_ALLOC_FAILED;
    }

    // Create the current instruction to hold the data

	for (int i = 0; i < buildContext->tokens.count; i++) {
		Token* base = buildContext->tokens.values[i];

		Token* preceding = NULL;
		TokenTypeDef* precedingType = get_token_type_def(TOKEN_EOL);
		if (i > 0) {
			preceding = buildContext->tokens.values[i - 1];
			precedingType = preceding->type;
		}

		Token* succeeding = NULL;
		TokenTypeDef* succeedingType = get_token_type_def(TOKEN_EOL);
		if (i < buildContext->tokens.count - 1) {
			succeeding = buildContext->tokens.values[i + 1];
			succeedingType = succeeding->type;
		}

		if (!validate_token_sequence(base->type, precedingType, succeedingType)) {
			return PARSER_TOKEN_SEQUENCE_ERROR;
        }

		
	}
}
