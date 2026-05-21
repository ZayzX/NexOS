#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define MAX_PROCESSES 16

typedef enum { PROC_RUNNING, PROC_SLEEPING, PROC_ZOMBIE } proc_state_t;

struct proc_entry {
    int pid;
    proc_state_t state;
    char name[16];
};

void proc_register(const char* name);
int proc_count(void);
const struct proc_entry* proc_list(void);

#endif
