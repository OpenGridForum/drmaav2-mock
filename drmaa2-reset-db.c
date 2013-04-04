#include <stdio.h>
#include <string.h>
#include "drmaa2-persistence.h"



int main(int argc, char **argv) {
    if (argc > 1) {
        fprintf(stderr, "DRMAA2 database setup/reset\n\n\
This executable creates DRMAA2 database tables if they do not exists yet\n\
AND\ndeletes all data from them.\n\nusage: %s\n", argv[0]);
        return(1);
    }
    reset_db();
    return 0;
}
