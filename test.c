#include "drmaa2.h"
#include <stdio.h>
#include <string.h>

int main ()
{
    drmaa2_jsession js;
    drmaa2_jtemplate jt;
    drmaa2_j j;

    drmaa2_version version = drmaa2_get_drmaa_version();
    char *name = drmaa2_get_drmaa_name();
    
    printf("This is %s version %s.%s\n", name, version->major, version->minor);
    js = drmaa2_create_jsession("testsession", DRMAA2_UNSET_STRING);
    jt = drmaa2_jtemplate_create();
    jt->remoteCommand = strdup("/bin/date");
    j = drmaa2_jsession_run_job(js, jt);
    drmaa2_j_wait_terminated(j, DRMAA2_INFINITE_TIME);
    drmaa2_jtemplate_free(jt);
    drmaa2_destroy_jsession("testsession");

    drmaa2_version_free(version);
    drmaa2_string_free(name);

    return 0;
}

