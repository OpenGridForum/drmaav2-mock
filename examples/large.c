#include <stdio.h>
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
    drmaa2_jtemplate        jt = drmaa2_jtemplate_create();
    drmaa2_rtemplate        rt = drmaa2_rtemplate_create();
    drmaa2_string_list      cl = drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
    drmaa2_dict            env = drmaa2_dict_create(NULL);
    drmaa2_jinfo            ji = NULL;
    drmaa2_j                 j = NULL;
    drmaa2_r                 r = NULL;
    drmaa2_machineinfo_list ml = NULL;
    drmaa2_machineinfo       m = NULL;

    // open sessions to DRM system
    drmaa2_jsession       js = drmaa2_create_jsession("myjsession", NULL);
    drmaa2_rsession       rs = drmaa2_create_rsession("myrsession", NULL);
    drmaa2_msession       ms = drmaa2_open_msession(NULL);

    // determine name of first machine
    ml = drmaa2_msession_get_all_machines(ms, DRMAA2_UNSET_LIST);
    if (drmaa2_list_size(ml) < 1) error_handler();
    m = (drmaa2_machineinfo)drmaa2_list_get(ml, 0);
    drmaa2_list_add(cl, m->name);

    // perform advance reservation
    rt->maxSlots = 4;
    if (DRMAA2_TRUE == drmaa2_supports(DRMAA2_RT_MACHINEOS)) rt->machineOS=DRMAA2_LINUX;
    rt->candidateMachines = cl;
    r = drmaa2_rsession_request_reservation(rs, rt);

    // submit job 
    jt->remoteCommand = strdup("/bin/date");
    jt->reservationId = drmaa2_r_get_id(r);
    drmaa2_dict_set(env, "FOO", "BAR");
    jt->jobEnvironment = env;
    j = drmaa2_jsession_run_job(js, jt);

    // Wait for termination and print exit status
    drmaa2_j_wait_terminated(j, DRMAA2_INFINITE_TIME);
    ji = drmaa2_j_get_info(j);
    printf("Job terminated with exit status %u\n",ji->exitStatus);
    printf("Job ran %f seconds\n", difftime(ji->finishTime, ji->dispatchTime));

    // close sessions, cleanup
    drmaa2_jtemplate_free(jt);  // includes free of env
    drmaa2_rtemplate_free(rt);  // includes free of cl
    printf("1\n");
    drmaa2_jinfo_free(ji);
    printf("2\n");
    drmaa2_close_msession(ms);
    printf("3\n");
    drmaa2_close_rsession(rs);
    drmaa2_close_jsession(js);
    drmaa2_list_free(ml);
}
