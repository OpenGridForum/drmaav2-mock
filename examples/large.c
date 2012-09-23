#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../drmaa2.h"



void error_handler()
{
}

void callback_free(void * entry)
{
    free(entry);
}


int main()
{
    drmaa2_jinfo            ji = NULL;
    drmaa2_j                 j = NULL;
    drmaa2_r                 r = NULL;
    drmaa2_machineinfo_list ml = NULL;
    drmaa2_machineinfo       m = NULL;

    drmaa2_jtemplate        jt = drmaa2_jtemplate_create();
    drmaa2_rtemplate        rt = drmaa2_rtemplate_create();
    drmaa2_string_list      cl = drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
    drmaa2_dict            env = drmaa2_dict_create(NULL);

    drmaa2_jsession js = drmaa2_create_jsession("myjsession", NULL);     // open sessions to DRM system
    if (js == NULL)
    {
        printf("%s\n", drmaa2_lasterror_text()); //error msg should be freed for long running applications
        exit(1);
    }
    drmaa2_rsession rs = drmaa2_create_rsession("myrsession", NULL);
    if (rs == NULL)
    {
        printf("%s\n", drmaa2_lasterror_text()); //error msg should be freed for long running applications
        exit(1);
    }
    drmaa2_msession ms = drmaa2_open_msession(NULL);

    ml = drmaa2_msession_get_all_machines(ms, DRMAA2_UNSET_LIST);        // determine name of first machine
    if (drmaa2_list_size(ml) < 1)
    {
        printf("No machines to assign jobs to.\n");
        exit(1);
    }
    m = (drmaa2_machineinfo)drmaa2_list_get(ml, 0);
    drmaa2_list_add(cl, m->name);

    rt->maxSlots = 4;                                                    // perform advance reservation
    if (DRMAA2_TRUE == drmaa2_supports(DRMAA2_RT_MACHINEOS)) 
        rt->machineOS = DRMAA2_LINUX;
    rt->candidateMachines = cl;
    r = drmaa2_rsession_request_reservation(rs, rt);

    jt->remoteCommand = strdup("/bin/date");                                     // submit job 
    jt->reservationId = drmaa2_r_get_id(r);
    drmaa2_dict_set(env, "FOO", "BAR");
    jt->jobEnvironment = env;
    j = drmaa2_jsession_run_job(js, jt);

    drmaa2_j_wait_terminated(j, DRMAA2_INFINITE_TIME);                  // Wait for termination and print exit status
    ji = drmaa2_j_get_info(j);
    printf("Job terminated with exit status %u\n",ji->exitStatus);
    printf("Job ran %f seconds\n", difftime(ji->finishTime, ji->dispatchTime));

    // close sessions, cleanup
    drmaa2_jtemplate_free(&jt);  // includes free of env
    drmaa2_rtemplate_free(&rt);  // includes free of cl
    drmaa2_jinfo_free(&ji);
    drmaa2_j_free(&j);
    drmaa2_r_free(&r);

    drmaa2_close_msession(ms);
    drmaa2_close_rsession(rs);
    drmaa2_destroy_jsession("myjsession");


    drmaa2_msession_free(&ms);
    drmaa2_rsession_free(&rs);
    drmaa2_jsession_free(&js);
 
    drmaa2_list_free(&ml);
}
