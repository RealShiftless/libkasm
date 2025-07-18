#pragma once

#include "libkasm.h"
#include "list.h"

typedef enum {
	PARSER_OK,
	PARSER_TOKEN_SEQUENCE_ERROR,
	PARSER_ALLOC_FAILED,
	PARSER_MULTIPLE_ACTIONS_ERROR,
    PARSER_INVALID_DIRECTIVE,
    PARSER_INVALID_INSTRUCTION,
    PARSER_IMMEDIATE_OUT_OF_RANGE
} ParserResult;

typedef struct {
	BuildContext* build;

    //Action* currentAction;
	uint32_t currentPosition;

    uint8_t currentActionType;
    uint16_t currentValue;
    List currentArguments;
} ParserContext;

extern ParserContext* gParserContext;

// Parses the tokens stored in the build context and populates its instruction list and label table.
ParserResult kasm_parse(BuildContext* buildContext);
