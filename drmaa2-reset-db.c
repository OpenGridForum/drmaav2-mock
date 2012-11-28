#include <stdio.h>
#include <string.h>
#include "config.h"
#include "drmaa2-persistence.h"


int main(int argc, char **argv) {
    if (argc != 1) {
        fprintf(stderr, "DRMAA2 database setup/reset\n\n\
This executable creates DRMAA2 database tables if they do not exists yet\n\
AND\n\
deletes all data from them.\n\n\
usage: %s\n", argv[0]);
        return(1);
    }
    
    drmaa2_reset_db(DRMAA_DBFILE);

    return 0;
}
