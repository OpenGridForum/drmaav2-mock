#include <stdio.h>
#include <string.h>
#include "config.h"
#include "persistence.h"



int main(int argc, char **argv)
{
    if( argc!=2 || (strcmp(argv[1], "SETUP") && strcmp(argv[1], "RESET")))
    {
        fprintf(stderr, "Usage: %s (SETUP | RESET)\n", argv[0]);
        return(1);
    }

    if (!strcmp(argv[1], "SETUP"))
        drmaa2_setup_db(DRMAA_DBFILE);
    else
        drmaa2_reset_db(DRMAA_DBFILE);

    return 0;
}
