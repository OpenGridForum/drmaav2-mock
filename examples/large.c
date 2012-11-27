#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../drmaa2.h"



void error_handler()
{
}

void callback_free(void *entry)
{
    free(entry);
}



void my_drmaa2_callback(drmaa2_notification *notification)
{
    printf("my callback is called\n");
    printf("Job %s Status %d \n", (*notification)->jobId, (*notification)->jobState);

    drmaa2_notification_free(notification);
}


int main()
{
    drmaa2_jinfo            ji = NULL;
    drmaa2_j                 j = NULL;
    drmaa2_r                 r = NULL;
    drmaa2_machineinfo_list ml = NULL;
    drmaa2_machineinfo       m = NULL;
    //drmaa2_register_event_notification(my_drmaa2_callback);

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

    drmaa2_rinfo ri = drmaa2_r_get_info(r);
    printf("Got reservation\n id: %s\n name: %s\n", ri->reservationId, ri->reservationName);
    drmaa2_rinfo_free(&ri);

    jt->remoteCommand = strdup("./sleepFive");                                     // submit job 
    jt->reservationId = drmaa2_r_get_id(r);
    drmaa2_dict_set(env, "FOO", "BAR");
    jt->jobEnvironment = env;
    j = drmaa2_jsession_run_job(js, jt);
    drmaa2_j_wait_started(j, DRMAA2_INFINITE_TIME);
    //drmaa2_j_terminate(j);
    drmaa2_j_wait_terminated(j, DRMAA2_INFINITE_TIME);                  // Wait for termination and print exit status

    drmaa2_string substate = NULL;
    drmaa2_jstate state = drmaa2_j_get_state(j, &substate);
    printf("State of job: %d\n", state);
    drmaa2_string_free(&substate);

    ji = drmaa2_j_get_info(j);
    printf("Job terminated with exit status %i\n",ji->exitStatus);
    printf("Job ran %f seconds\n", difftime(ji->finishTime, ji->dispatchTime));

    // close sessions, cleanup
    //drmaa2_jtemplate_free(&jt);  // includes free of env
    drmaa2_rtemplate_free(&rt);  // includes free of cl
    drmaa2_jinfo_free(&ji);
    drmaa2_j_free(&j);
    drmaa2_r_free(&r);

    drmaa2_string_free(&jt->remoteCommand);
    jt->remoteCommand = strdup("./sleepFive");
    jt->workingDirectory = strdup("$DRMAA2_HOME_DIR$/hallo/$DRMAA2_INDEX$/test/$DRMAA2_INDEX$.tbl");
    jt->errorPath = strdup("/$DRMAA2_INDEX$/test/$DRMAA2_INDEX$.tbl");
    jt->inputPath = strdup("$DRMAA2_WORKING_DIR$/$DRMAA2_INDEX$/test/$DRMAA2_INDEX$.tbl");

    printf("Here comes a job array\n");
    drmaa2_jarray ja = drmaa2_jsession_run_bulk_jobs(js, jt, 1, 4, 1, DRMAA2_UNSET_NUM);

    drmaa2_jtemplate ja_jt = drmaa2_jarray_get_job_template(ja);
    drmaa2_string ja_id = drmaa2_jarray_get_id(ja);
    printf("The jobarray %s was submitted with following command:  %s\n",
        ja_id, ja_jt->remoteCommand);
    drmaa2_string_free(&ja_id);
    drmaa2_jtemplate_free(&ja_jt);

    drmaa2_jarray_terminate(ja);



    drmaa2_jarray_free(&ja);
    drmaa2_jtemplate_free(&jt);  // includes free of env

    drmaa2_list jl = drmaa2_jsession_get_jobs(js, NULL);
    printf("The job session has %ld jobs\n", drmaa2_list_size(jl));
    drmaa2_list_free(&jl);

    ji = drmaa2_jinfo_create();
    ji->jobState = DRMAA2_FAILED;
    jl = drmaa2_jsession_get_jobs(js, ji);
    printf("The job session has %ld failed jobs\n", drmaa2_list_size(jl));
    drmaa2_list_free(&jl);
    drmaa2_jinfo_free(&ji);




    drmaa2_close_msession(ms);
    drmaa2_close_rsession(rs);
    drmaa2_destroy_jsession("myjsession");
    drmaa2_destroy_rsession("myrsession");


    drmaa2_msession_free(&ms);
    drmaa2_rsession_free(&rs);
    drmaa2_jsession_free(&js);
 
    drmaa2_list_free(&ml);
    sleep(2); // wait that jobs are terminated
}
