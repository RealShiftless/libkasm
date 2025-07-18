#include "getopt.h"
#include <string.h>
#include <stdio.h>

char *optarg;
int optind = 1;
int opterr = 1;
int optopt;

int getopt(int argc, char * const argv[], const char *optstring) {
    static int optpos = 1;

    if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0') {
        return -1;
    }

    char *arg = argv[optind];

    if (strcmp(arg, "--") == 0) {
        optind++;
        return -1;
    }

    optopt = arg[optpos];
    const char *optdecl = strchr(optstring, optopt);

    if (!optdecl) {
        if (opterr) {
            fprintf(stderr, "Unknown option: -%c\n", optopt);
        }

        if (++optpos >= strlen(arg)) {
            optind++;
            optpos = 1;
        }

        return '?';
    }

    if (optdecl[1] == ':') {
        if (optpos + 1 < strlen(arg)) {
            optarg = &arg[optpos + 1];
        } else if (optind + 1 < argc) {
            optarg = argv[++optind];
        } else {
            if (opterr) {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            }
            optpos = 1;
            optind++;
            return '?';
        }

        optind++;
        optpos = 1;
    } else {
        if (++optpos >= strlen(arg)) {
            optpos = 1;
            optind++;
        }
        optarg = NULL;
    }

    return optopt;
}