#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "history.h"

int main(void)
{
    char *input;

    printf("=========================\n");
    printf(" Welcome to Shellforge\n");
    printf("=========================\n");

    while (1) {
        input = readline("shellforge> ");

        if (input == NULL) {
            break;
        }

        if (input[0] != '\0') {
            add_history(input);
        }

        if (strcmp(input, "history") == 0) {
            print_history();
        }
        else if (strcmp(input, "exit") == 0) {
            printf("Closing Shellforge...\n");
            free(input);
            break;
        }
        else {
            printf("Command entered: %s\n", input);
        }

        free(input);
    }

    return 0;
}
