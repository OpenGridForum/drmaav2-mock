#include "drmaa2.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


// TODO:    use jobinfo
//          replace pid by id
typedef struct job
{
    const char *id;
    const char *session_name;
    pid_t pid;
} job_s;
typedef job_s *job_t;


typedef struct job_session
{
    const char *contact;
    const char *name;
    job_t jobs;
} job_session_s;
typedef job_session_s *job_session_t;




drmaa2_job_template_t  drmaa2_job_template_create(void)
{
    drmaa2_job_template_t jt = (drmaa2_job_template_t)malloc(sizeof(drmaa2_job_template_s));
    jt->remoteCommand       = DRMAA2_UNSET_STRING;
    jt->args                = DRMAA2_UNSET_LIST;          
    jt->submitAsHold        = DRMAA2_UNSET_BOOLEAN;
    jt->rerunnable          = DRMAA2_UNSET_BOOLEAN;
    jt->jobEnvironment      = DRMAA2_UNSET_DICT;
    jt->workingDirectory    = DRMAA2_UNSET_STRING;        
    jt->jobCategory         = DRMAA2_UNSET_STRING;            
    jt->email               = DRMAA2_UNSET_LIST;
    jt->emailOnStarted      = DRMAA2_UNSET_BOOLEAN;
    jt->emailOnTerminated   = DRMAA2_UNSET_BOOLEAN;
    jt->jobName             = DRMAA2_UNSET_STRING;              
    jt->inputPath           = DRMAA2_UNSET_STRING;
    jt->outputPath          = DRMAA2_UNSET_STRING;
    jt->errorPath           = DRMAA2_UNSET_STRING;
    jt->joinFiles           = DRMAA2_UNSET_BOOLEAN;
    jt->reservationId       = DRMAA2_UNSET_STRING;
    jt->queueName           = DRMAA2_UNSET_STRING;
    jt->minSlots            = DRMAA2_UNSET_NUM;
    jt->maxSlots            = DRMAA2_UNSET_NUM;
    jt->priority            = DRMAA2_UNSET_NUM;
    jt->candidateMachines   = DRMAA2_UNSET_LIST;
    jt->minPhysMemory       = DRMAA2_UNSET_NUM;
    jt->machineOS           = DRMAA2_UNSET_ENUM;
    jt->machineArch         = DRMAA2_UNSET_ENUM;
    jt->startTime           = DRMAA2_UNSET_TIME;
    jt->deadlineTime        = DRMAA2_UNSET_TIME;
    jt->stageInFiles        = DRMAA2_UNSET_DICT; 
    jt->stageOutFiles       = DRMAA2_UNSET_DICT;
    jt->resourceLimits      = DRMAA2_UNSET_DICT;
    jt->accountingId        = DRMAA2_UNSET_STRING;
    return jt;
}


drmaa2_error_t drmaa2_job_template_free(drmaa2_job_template_t jt)
{
    free(jt);
    return 0;
}


drmaa2_job_h drmaa2_js_run_job(const drmaa2_jobsession_h js, const drmaa2_job_template_t jt)
{
    pid_t childpid;

    job_session_t js_t = (job_session_t)js;

    if ((childpid = fork()) == -1)
    {
        perror("fork failed\n");
        exit(1);
    }
    else if (childpid == 0)
        {
            // child
            execv(jt->remoteCommand, jt->args);
            exit(0);
        }
        else
        {
            // parent
            job_t j = (job_t)malloc(sizeof(job_s));
            j->session_name = js_t->name;
            j->pid = childpid;
            return j;
        }
}


drmaa2_job_h drmaa2_j_wait_terminated(const drmaa2_job_h j, const time_t timeout)
{
    pid_t child;
    int status;

    job_t j_t = (job_t)j;
    child = waitpid(j_t->pid, &status, 0);
    return j;
}


char * drmaa2_get_drms_name(void)
{
    return NULL;
}

drmaa2_version_t drmaa2_get_drms_version(void)
{
    return NULL;
}

char *drmaa2_get_drmaa_name(void)
{
    return NULL;
}

drmaa2_version_t drmaa2_get_drmaa_version(void)
{
    return NULL;
}


drmaa2_jobsession_h drmaa2_create_jobsession(const char * session_name, const char * contact){
    job_session_t js = (job_session_t)malloc(sizeof(job_session_s));
    js->name = session_name;
    js->contact = contact;
    // TODO: append job-session to js-list
    return js;
}


drmaa2_error_t drmaa2_close_jobsession(drmaa2_jobsession_h js)
{
    // should this method be called before destruction?
    free(js);
    // TODO: persist information (which??)
    return 0;
}


drmaa2_error_t drmaa2_destroy_jobsession(const char * session_name)
{
    // TODO: reap persistent information
    return 0;
}
