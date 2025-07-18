#include <stdio.h>
#include <stdlib.h>
#include "../src/libkasm.h"
#include "../src/lexer.h"

#ifdef _WIN32
#  include "getopt.h"
#else
#  include <getopt.h>
#endif

#define VERSION "indev 1.0"

#ifdef _WIN32
#include <windows.h>
#define DLLSYM __declspec(dllimport)
#else
#define DLLSYM
#endif

typedef BuildTarget* (*TargetRegisterFn)();

BuildTarget* load_target_register(const char* path) {
    TargetRegisterFn reg;
#ifdef _WIN32
    HMODULE lib = LoadLibraryA(path);
    reg = (TargetRegisterFn)GetProcAddress(lib, "kasm_target_register");
#else
    void* lib = dlopen(path, RTLD_NOW);
    reg = (TargetRegisterFn)dlsym(lib, "kasm_target_register");
#endif

    return reg();
}

int main(int argc, char *argv[]) {
    int opt;
    char* file_path = NULL;
    char* target_name = NULL;
    char targetPath[256] = {0};

    while ((opt = getopt(argc, argv, "f:V:t:h")) != -1) {
        switch (opt) {
            case 'f': // File select
                file_path = optarg;
                break;

            case 'h': // Help
                printf("Usage: kasm -f <file>\n");
                return 0;

            case 'V': // Print Version
                printf("kasm %s", VERSION);
                return 0;

            case 't': // Target
                target_name = optarg;
                snprintf(targetPath, sizeof(targetPath), "targets/%s.dll", target_name);
                break;
            default:
                return 1;
        }
    }

    if(file_path == NULL) {
        printf("No file specified.\n");
        return 1;
    }

    if(target_name == NULL) {
        printf("No target specified.\n");
        return 1;
    }
    
    printf("Loading target: %s\n", targetPath);
    BuildTarget* target = load_target_register(targetPath);

    if(target == NULL) {
        printf("Could not load target!\n");
        return 1;
    }

    printf("Opening file...\n");
    
    FILE* file = fopen(file_path, "r");
    if(file == NULL) {
        printf("Could not open file!\n");
        return 1;
    }

    List tokens = { 0 };
    list_init(&tokens);

    printf("Lexing...\n");
    LexerResult result = lex(file, &tokens);
    if(result != LEXER_OK) {
        printf("Lexer Errored: %s\n", get_lexer_result_msg(result));
        return 1;
    }

    printf("parsing...");
    

    printf("Assembling: %s\n", file_path);

    return 0;
}