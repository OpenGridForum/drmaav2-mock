#include <stdio.h>
#include "persistence.h"



int main(int argc, char **argv)
{
    if( argc!=3 || (strcmp(argv[2], "SETUP") && strcmp(argv[2], "RESET")))
    {
        fprintf(stderr, "Usage: %s DATABASE (SETUP | RESET)\n", argv[0]);
        return(1);
    }

    if (!strcmp(argv[2], "SETUP"))
        drmaa2_setup_db(argv[1]);
    else
        drmaa2_reset_db(argv[1]);

    return 0;
}