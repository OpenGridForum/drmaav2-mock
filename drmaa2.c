#include "drmaa2.h"
#include "drmaa2-list.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>



// Constants, filled on demand in according functions
drmaa2_version version=NULL;


typedef struct drmaa2_jsession_s
{
    const char *contact;
    const char *name;
    drmaa2_j jobs;
} drmaa2_jsession_s;

typedef struct drmaa2_rsession_s
{
    const char *contact;
    const char *name;
    //TODO: complete
} drmaa2_rsession_s;

typedef struct drmaa2_msession_s
{
    const char *name;
} drmaa2_msession_s;

typedef struct drmaa2_j_s
{
    const char *id;
    const char *session_name;
    pid_t pid;
    drmaa2_jtemplate template;
    drmaa2_jinfo info;
} drmaa2_j_s;

typedef struct drmaa2_r_s
{
    const char *id;;
    const char *session_name;
    drmaa2_rtemplate template;
    drmaa2_rinfo info;
} drmaa2_r_s;


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
    //TODO: free fields if necessary
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
    rt->machineOS = DRMAA2_OTHER_OS;  
    rt->machineArch = DRMAA2_OTHER_CPU;
    return rt;
}


drmaa2_error drmaa2_rtemplate_free(drmaa2_rtemplate rt)
{
    //TODO: free fields if necessary
    free(rt);
    return DRMAA2_SUCCESS;
}


drmaa2_r drmaa2_rsession_request_reservation(const drmaa2_rsession rs, const drmaa2_rtemplate rt)
{
    drmaa2_r r = (drmaa2_r)malloc(sizeof(drmaa2_r_s));
    r->id = NULL;
    r->session_name = rs->name;
    //copy reservation template, work only with the copy
    r->template = (drmaa2_rtemplate)malloc(sizeof(drmaa2_rtemplate_s));
    memcpy(r->template, rt, sizeof(drmaa2_rtemplate_s));

    drmaa2_rinfo info = (drmaa2_rinfo)malloc(sizeof(drmaa2_rinfo_s));
    info->reservationId = (char *)r->id;
    info->reservationName = (char *)r->session_name;
    info->reservedStartTime = rt->startTime;
    info->reservedEndTime = rt->endTime;
    info->usersACL = rt->usersACL;
    info->reservedSlots = rt->maxSlots;
    info->reservedMachines = rt->candidateMachines;

    r->info = info;
    return r;
}


char *drmaa2_r_get_id(const drmaa2_r r)
{
    return (char *)r->id;
}


drmaa2_j drmaa2_jsession_run_job(const drmaa2_jsession js, const drmaa2_jtemplate jt)
{
    pid_t childpid;

    if ((childpid = fork()) == -1)
    {
        perror("fork failed\n");
        exit(1);
    }
    else if (childpid == 0)
        {
            // child
            char **args = NULL;
            if (jt->args) 
		        args = (char **)stringlist_get_array(jt->args);
            execv(jt->remoteCommand, args);
        }
        else
        {
            // parent
            drmaa2_j j = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
            j->id = NULL;
            j->session_name = js->name;
            j->pid = childpid;

            //copy job template, work only with the copy 
            j->template = (drmaa2_jtemplate)malloc(sizeof(drmaa2_jtemplate_s));
            memcpy(j->template, jt, sizeof(drmaa2_jtemplate_s));

            drmaa2_jinfo info = (drmaa2_jinfo)malloc(sizeof(drmaa2_jinfo_s));
            info->jobId = (char *)j->id;
            info->exitStatus = DRMAA2_UNSET_NUM;
            info->terminatingSignal = DRMAA2_UNSET_STRING;
            info->annotation = DRMAA2_UNSET_STRING;
            info->jobState = DRMAA2_UNDETERMINED;
            info->jobSubState = DRMAA2_UNSET_STRING;
            info->allocatedMachines = DRMAA2_UNSET_STRING;
            info->submissionMachine = DRMAA2_UNSET_STRING;
            info->jobOwner = DRMAA2_UNSET_STRING;
            info->slots = DRMAA2_UNSET_NUM;
            info->queueName = DRMAA2_UNSET_STRING;
            info->wallclockTime = DRMAA2_UNSET_TIME;
            info->cpuTime = DRMAA2_UNSET_NUM;
            info->submissionTime = time(NULL);
            info->dispatchTime = time(NULL);
            info->finishTime = DRMAA2_UNSET_TIME;

            j->info = info;
            return j;
        }
}


drmaa2_jinfo drmaa2_j_get_info(const drmaa2_j j)
{
    return j->info;
}


drmaa2_j drmaa2_j_wait_terminated(const drmaa2_j j, const time_t timeout)
{
    pid_t child;
    int status;

    child = waitpid(j->pid, &status, 0);

    if (WIFEXITED(status))
    {
        j->info->exitStatus = WEXITSTATUS(status);
        j->info->finishTime = time(NULL);

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


drmaa2_machineinfo_list drmaa2_msession_get_all_machines(const drmaa2_msession ms, const drmaa2_string_list names)
{
    drmaa2_machineinfo_list ml = drmaa2_list_create(DRMAA2_MACHINEINFOLIST, NULL);
    //TODO: set callback for cleanup

    // TODO: get real machine info
    drmaa2_machineinfo mi = (drmaa2_machineinfo)malloc(sizeof(drmaa2_machineinfo_s));
    char *name = (char *)malloc(sizeof("my machine"));
    strcpy(name, "my machine");
    mi->name                = name;  
    mi->available           = 1;    
    mi->sockets             = 1;      
    mi->coresPerSocket      = 1;
    mi->threadsPerCore      = 1;  
    mi->load                = 0;  
    mi->physMemory          = 4194304;
    mi->virtMemory          = 4194304;    
    mi->machineOS           = DRMAA2_MACOS;  
    mi->machineOSVersion    = NULL;
    mi->machineArch         = DRMAA2_X86;

    drmaa2_list_add(ml, mi);
    return ml;
}


char *drmaa2_get_drms_name(void)
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


drmaa2_bool drmaa2_supports(const drmaa2_capability c)
{
    return DRMAA2_FALSE;
}


drmaa2_jsession drmaa2_create_jsession(const char * session_name, const char * contact)
{
    // TODO: uniqueness test of name
    // handle empty names
    drmaa2_jsession js = (drmaa2_jsession)malloc(sizeof(drmaa2_jsession_s));
    js->name = session_name;
    js->contact = contact;
    return js;
}


drmaa2_rsession drmaa2_create_rsession(const char * session_name, const char * contact)
{
    // TODO: uniqueness test of name
    // handle empty names
    drmaa2_rsession rs = (drmaa2_rsession)malloc(sizeof(drmaa2_rsession_s));
    rs->name = session_name;
    rs->contact = contact;
    return rs;
}


drmaa2_msession drmaa2_open_msession(const char * session_name)
{
    drmaa2_msession ms = (drmaa2_msession)malloc(sizeof(drmaa2_msession_s));
    ms->name = session_name;
    return ms;
}


drmaa2_error drmaa2_close_jsession(drmaa2_jsession js)
{
    free(js);
    // TODO: persist information (which??)
    // TODO: cleanup
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_close_msession(drmaa2_msession ms)
{
    free(ms);
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_destroy_jsession(const char * session_name)
{
    // TODO: reap persistent information
    // TODO: cleanup
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_destroy_rsession(const char * session_name)
{
    // TODO: reap persistent information
    // TODO: cleanup
    return DRMAA2_SUCCESS;
}


drmaa2_string_list drmaa2_get_jsession_names(void)
{
    //return only persistent data
    return NULL;
}

drmaa2_string_list drmaa2_get_rsession_names(void)
{
    //return only persistent data
    //or set error
    return NULL;
}

