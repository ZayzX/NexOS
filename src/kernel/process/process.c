#include "process.h"
#include <string.h>

static struct proc_entry table[MAX_PROCESSES];
static int next_pid = 1;

void proc_register(const char* name) {
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        if (table[i].pid == 0) {
                    table[i].pid = next_pid++;
                    table[i].state = PROC_RUNNING;
                    int j = 0;
                    while (j < (int)sizeof(table[i].name)-1 && name[j]) { table[i].name[j] = name[j]; j++; }
                    table[i].name[j] = 0;
            return;
        }
    }
}

int proc_count(void) {
    int c = 0;
    for (int i = 0; i < MAX_PROCESSES; ++i) if (table[i].pid) c++;
    return c;
}

const struct proc_entry* proc_list(void) {
    return table;
}
