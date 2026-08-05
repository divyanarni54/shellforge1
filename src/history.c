#include <stdio.h>
#include <readline/history.h>
#include "history.h"

void print_history(void)
{
    HIST_ENTRY **the_list;
    int i;

    the_list = history_list();

    if (the_list) {
        for (i = 0; the_list[i]; i++) {
            printf("%d: %s\n", i + history_base, the_list[i]->line);
        }
    }
}
