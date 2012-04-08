#include "drmaa2.h"
#include "drmaa2-list.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



// Constants, filled on demand in according functions
drmaa2_version version=NULL;

// TODO:    use jobinfo
//          replace pid by id
typedef struct drmaa2_j_s
{
    const char *id;
    const char *session_name;
    pid_t pid;
} drmaa2_j_t;

typedef struct drmaa2_jsession_s
{
    const char *contact;
    const char *name;
    drmaa2_j_h jobs;
} drmaa2_jsession_t;

typedef struct drmaa2_rsession_s
{
    const char *contact;
    const char *name;
    //TODO: complete
} drmaa2_rsession_t;

typedef struct drmaa2_msession_s
{
    const char *name;
} drmaa2_msession_t;

drmaa2_jtemplate  drmaa2_jtemplate_create(void)
{
    drmaa2_jtemplate jt = (drmaa2_jtemplate)malloc(sizeof(drmaa2_jtemplate_s));
    jt->remoteCommand       = DRMAA2_UNSET_STRING;
    jt->args                = DRMAA2_UNSET_LIST;          
    jt->submitAsHold        = DRMAA2_UNSET_BOOL;
    jt->rerunnable          = DRMAA2_UNSET_BOOL;
    jt->jobEnvironment      = DRMAA2_UNSET_DICT;
    jt->workingDirectory    = DRMAA2_UNSET_STRING;        
    jt->jobCategory         = DRMAA2_UNSET_STRING;            
    jt->email               = DRMAA2_UNSET_LIST;
    jt->emailOnStarted      = DRMAA2_UNSET_BOOL;
    jt->emailOnTerminated   = DRMAA2_UNSET_BOOL;
    jt->jobName             = DRMAA2_UNSET_STRING;              
    jt->inputPath           = DRMAA2_UNSET_STRING;
    jt->outputPath          = DRMAA2_UNSET_STRING;
    jt->errorPath           = DRMAA2_UNSET_STRING;
    jt->joinFiles           = DRMAA2_UNSET_BOOL;
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


drmaa2_error drmaa2_jtemplate_free(drmaa2_jtemplate jt)
{
    free(jt);
    return DRMAA2_SUCCESS;
}


drmaa2_rtemplate drmaa2_rtemplate_create(void)
{
    drmaa2_rtemplate rt = (drmaa2_rtemplate)malloc(sizeof(drmaa2_rtemplate_s));
    rt->reservationName = DRMAA2_UNSET_STRING;          
    rt->startTime = DRMAA2_UNSET_TIME;          
    rt->endTime = DRMAA2_UNSET_TIME;          
    rt->duration = DRMAA2_UNSET_TIME;          
    rt->minSlots = DRMAA2_UNSET_NUM;              
    rt->maxSlots = DRMAA2_UNSET_NUM;
    rt->jobCategory = DRMAA2_UNSET_STRING;
    rt->usersACL = DRMAA2_UNSET_LIST;            
    rt->candidateMachines = DRMAA2_UNSET_LIST; 
    rt->minPhysMemory = DRMAA2_UNSET_NUM;            
    rt->machineOS = -1;    //TODO    
    rt->machineArch = -1;   //TODO
}


drmaa2_error drmaa2_rtemplate_free(drmaa2_rtemplate rt)
{
    free(rt);
    return DRMAA2_SUCCESS;
}


drmaa2_j_h drmaa2_jsession_run_job(const drmaa2_jsession_h js, const drmaa2_jtemplate jt)
{
    pid_t childpid;
    char ** args = NULL;

    //TODO: copy job template, work only with the copy 
    if ((childpid = fork()) == -1)
    {
        perror("fork failed\n");
        exit(1);
    }
    else if (childpid == 0)
        {
            // child
            if (jt->args) 
		        args=stringlist_get_array(jt->args);
            execv(jt->remoteCommand, args);     //TODO: no clean up necessary?
            if (jt->args)
                stringlist_free_array(args);
            exit(0);
        }
        else
        {
            // parent
            drmaa2_j_h j = (drmaa2_j_h)malloc(sizeof(drmaa2_j_t));
            j->session_name = js->name;
            j->pid = childpid;
            return j;
        }
}


drmaa2_j_h drmaa2_j_wait_terminated(const drmaa2_j_h j, const time_t timeout)
{
    pid_t child;
    int status = 0;

    child = waitpid(j->pid, &status, 0);
    printf("status of child %d: %d\n", child, status);

    if (WIFEXITED(status))
    {
        printf("Process terminated normally by a call to _exit(2) or exit(3).\n");
        printf("%d  - evaluates to the low-order 8 bits of the argument passed to _exit(2) or exit(3) by the child.\n", WEXITSTATUS(status));
    }
    if (WIFSIGNALED(status))
    {
        printf("Process terminated due to receipt of a signal.\n");
        printf("%d  - evaluates to the number of the signal that caused the termination of the process.\n", WTERMSIG(status));
        printf("%d  - evaluates as true if the termination of the process was accompanied by the creation of a core \
             file containing an image of the process when the signal was received.\n", WCOREDUMP(status));
    }
    if (WIFSTOPPED(status))
    {
        printf("Process has not terminated, but has stopped and can be restarted.  This macro can be true only if the wait call \
             specified the WUNTRACED option or if the child process is being traced (see ptrace(2)).\n");
        printf("%d  - evaluates to the number of the signal that caused the process to stop.\n", WSTOPSIG(status));
    }
    return j;
}


char * drmaa2_get_drms_name(void)
{
    return NULL;
}

drmaa2_version drmaa2_get_drms_version(void)
{
    return NULL;
}

char *drmaa2_get_drmaa_name(void)
{
    return "drmaa2-mock";
}

drmaa2_version drmaa2_get_drmaa_version(void)
{
    if (version == NULL) {
        version=malloc(sizeof(drmaa2_version_s));
        version->major="2";
        version->minor="0";
    }
    return version;
}


drmaa2_jsession_h drmaa2_create_jsession(const char * session_name, const char * contact)
{
    drmaa2_jsession_h js = (drmaa2_jsession_h)malloc(sizeof(drmaa2_jsession_t));
    js->name = session_name;
    js->contact = contact;
    // TODO: append job-session to js-list
    return js;
}


drmaa2_rsession_h drmaa2_create_rsession(const char * session_name, const char * contact)
{
    drmaa2_rsession_h rs = (drmaa2_rsession_h)malloc(sizeof(drmaa2_rsession_t));
    rs->name = session_name;
    rs->contact = contact;
    // TODO: append reservation-session to rs-list
    return rs;
}


drmaa2_msession_h drmaa2_open_msession(const char * session_name)
{
    drmaa2_msession_h ms = (drmaa2_msession_h)malloc(sizeof(drmaa2_msession_t));
    ms->name = session_name;
    return ms;
}


drmaa2_error drmaa2_close_jsession(drmaa2_jsession_h js)
{
    // should this method be called before destruction?
    free(js);
    // TODO: persist information (which??)
    return 0;
}


drmaa2_error drmaa2_destroy_jsession(const char * session_name)
{
    // TODO: reap persistent information
    return 0;
}
