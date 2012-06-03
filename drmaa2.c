#include "drmaa2.h"
#include "drmaa2-list.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>


int lasterror           = DRMAA2_SUCCESS;
char *lasterror_text    = NULL;


#define DRMAA2_LIST     6
// "persistent" drms information
drmaa2_list j_sessions      = DRMAA2_UNSET_LIST;
drmaa2_list r_sessions      = DRMAA2_UNSET_LIST;


unsigned long long drmaa2_last_reservation_id   = 0;
unsigned long long drmaa2_last_job_id           = 0;


// supported jobcategories
#define JOBCATEGORIES_LENGTH  3
char *jobcategories[] = {"OpenMP", "Java", "Python"};

int string_array_contains(char *array[], int len, char *string)
// false: 0     true: != 0
{
    int i;
    for (i=0; i<len; i++)
        if (strcmp(array[i], string) == 0) return 1;
    return 0;
}



typedef struct drmaa2_jsession_s
{
    const char *contact;
    const char *name;
    drmaa2_j_list jobs;
} drmaa2_jsession_s;

typedef struct drmaa2_rsession_s
{
    const char *contact;
    const char *name;
    drmaa2_r_list reservations;
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
    const char *id;
    const char *session_name;
    drmaa2_rtemplate template;
    drmaa2_rinfo info;
} drmaa2_r_s;


drmaa2_error drmaa2_string_free(char* string)
{
    free(string);
}


drmaa2_error drmaa2_lasterror(void)
{
    return lasterror;
}


char *drmaa2_lasterror_text(void)
{
    return NULL;
}


drmaa2_jinfo drmaa2_jinfo_create(void)
{
    drmaa2_jinfo ji = (drmaa2_jinfo)malloc(sizeof(drmaa2_jinfo_s));
    ji->jobId               = DRMAA2_UNSET_STRING;
    ji->exitStatus          = DRMAA2_UNSET_NUM;
    ji->terminatingSignal   = DRMAA2_UNSET_STRING;
    ji->annotation          = DRMAA2_UNSET_STRING;
    ji->jobState            = DRMAA2_UNDETERMINED;
    ji->jobSubState         = DRMAA2_UNSET_STRING;
    ji->allocatedMachines   = DRMAA2_UNSET_LIST;
    ji->submissionMachine   = DRMAA2_UNSET_STRING;
    ji->jobOwner            = DRMAA2_UNSET_STRING;
    ji->slots               = DRMAA2_UNSET_NUM;
    ji->queueName           = DRMAA2_UNSET_STRING;
    ji->wallclockTime       = DRMAA2_UNSET_TIME;
    ji->cpuTime             = DRMAA2_UNSET_NUM;
    ji->submissionTime      = DRMAA2_UNSET_TIME;
    ji->dispatchTime        = DRMAA2_UNSET_TIME;
    ji->finishTime          = DRMAA2_UNSET_TIME;
    return ji;
}


drmaa2_error drmaa2_jinfo_free(drmaa2_jinfo ji)
{
    drmaa2_string_free(ji->jobId);
    drmaa2_string_free(ji->terminatingSignal);
    drmaa2_string_free(ji->annotation);
    drmaa2_string_free(ji->jobSubState);
    drmaa2_list_free(ji->allocatedMachines);
    drmaa2_string_free(ji->submissionMachine);
    drmaa2_string_free(ji->jobOwner);
    drmaa2_string_free(ji->queueName);
    free(ji);
    return DRMAA2_SUCCESS;
}


drmaa2_rinfo drmaa2_rinfo_create(void)
{
    // no drmaa function - only used by implementation
    drmaa2_rinfo ri = (drmaa2_rinfo) malloc(sizeof(drmaa2_rinfo_s));
    ri->reservationId       = DRMAA2_UNSET_STRING;
    ri->reservationName     = DRMAA2_UNSET_STRING;
    ri->reservedStartTime   = DRMAA2_UNSET_TIME;
    ri->reservedEndTime     = DRMAA2_UNSET_TIME;
    ri->usersACL            = DRMAA2_UNSET_LIST;
    ri->reservedSlots       = DRMAA2_UNSET_NUM;
    ri->reservedMachines    = DRMAA2_UNSET_LIST;
    return ri;
}


drmaa2_error drmaa2_rinfo_free(drmaa2_rinfo ri)
{
    drmaa2_string_free(ri->reservationId);
    drmaa2_string_free(ri->reservationName);
    drmaa2_list_free(ri->usersACL);
    drmaa2_list_free(ri->reservedMachines);
    free(ri);
    return DRMAA2_SUCCESS;
}


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
    // free fields
    free(jt->remoteCommand);
    drmaa2_list_free(jt->args);          
    drmaa2_dict_free(jt->jobEnvironment);
    free(jt->workingDirectory);        
    free(jt->jobCategory);            
    drmaa2_list_free(jt->email);
    free(jt->jobName);              
    free(jt->inputPath);
    free(jt->outputPath);
    free(jt->errorPath);
    free(jt->reservationId);
    free(jt->queueName);
    drmaa2_list_free(jt->candidateMachines);
    drmaa2_dict_free(jt->stageInFiles); 
    drmaa2_dict_free(jt->stageOutFiles);
    drmaa2_dict_free(jt->resourceLimits);
    free(jt->accountingId);
    // free container
    free(jt);
    return DRMAA2_SUCCESS;
}


char* drmaa2_jtemplate_tostring(drmaa2_jtemplate jt)
{
    return DRMAA2_UNSET_STRING;
}


drmaa2_rtemplate drmaa2_rtemplate_create(void)
{
    drmaa2_rtemplate rt = (drmaa2_rtemplate)malloc(sizeof(drmaa2_rtemplate_s));
    rt->reservationName     = DRMAA2_UNSET_STRING;          
    rt->startTime           = DRMAA2_UNSET_TIME;          
    rt->endTime             = DRMAA2_UNSET_TIME;          
    rt->duration            = DRMAA2_UNSET_TIME;          
    rt->minSlots            = DRMAA2_UNSET_NUM;              
    rt->maxSlots            = DRMAA2_UNSET_NUM;
    rt->jobCategory         = DRMAA2_UNSET_STRING;
    rt->usersACL            = DRMAA2_UNSET_LIST;            
    rt->candidateMachines   = DRMAA2_UNSET_LIST; 
    rt->minPhysMemory       = DRMAA2_UNSET_NUM;            
    rt->machineOS           = DRMAA2_OTHER_OS;  
    rt->machineArch         = DRMAA2_OTHER_CPU;
    return rt;
}


drmaa2_error drmaa2_rtemplate_free(drmaa2_rtemplate rt)
{
    // free fields
    free(rt->reservationName);          
    free(rt->jobCategory);
    drmaa2_list_free(rt->usersACL);            
    drmaa2_list_free(rt->candidateMachines);
    // free container 
    free(rt);
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_machineinfo_free(drmaa2_machineinfo mi)
{
    drmaa2_string_free(mi->name);
    if (mi->machineOSVersion) drmaa2_version_free(mi->machineOSVersion);
    free(mi);
    return DRMAA2_SUCCESS;
}


drmaa2_r drmaa2_rsession_request_reservation(const drmaa2_rsession rs, const drmaa2_rtemplate rt)
{
    drmaa2_r r = (drmaa2_r)malloc(sizeof(drmaa2_r_s));

    r->id = (char *)malloc(sizeof(char) * sizeof(long long) * 2 + 1); //alloc space for max long long hex
    sprintf((char *)r->id, "%llx", drmaa2_last_reservation_id);
    drmaa2_last_reservation_id++;
    r->session_name = strdup(rs->name); // session_name cannot be NULL

    // copy reservation template, work only with the copy
    drmaa2_rtemplate template = (drmaa2_rtemplate)malloc(sizeof(drmaa2_rtemplate_s));
    memcpy(template, rt, sizeof(drmaa2_rtemplate_s)); //copies scalar values
    //deep copy
    template->reservationName   = (rt->reservationName != NULL) ? strdup(rt->reservationName) : DRMAA2_UNSET_STRING;
    template->jobCategory       = (rt->jobCategory != NULL) ? strdup(rt->jobCategory) : DRMAA2_UNSET_STRING;
    template->usersACL          = drmaa2_list_create_copy(rt->usersACL, (drmaa2_list_entryfree)drmaa2_string_free, (drmaa2_copy_data_callback)strdup);
    template->candidateMachines = drmaa2_list_create_copy(rt->candidateMachines, (drmaa2_list_entryfree)drmaa2_string_free, (drmaa2_copy_data_callback)strdup);
    r->template = template;

    //no deep copy, since information is already saved 
    drmaa2_rinfo info = drmaa2_rinfo_create();
    info->reservationId     = (char *)r->id;
    info->reservationName   = (char *)r->session_name;
    info->reservedStartTime = rt->startTime;
    info->reservedEndTime   = rt->endTime;
    info->usersACL          = rt->usersACL;
    info->reservedSlots     = rt->maxSlots;
    info->reservedMachines  = rt->candidateMachines;

    r->info = info;
    drmaa2_list_add(rs->reservations, r);
    return r;
}


char *drmaa2_r_get_id(const drmaa2_r r)
{
    //returns copy since application should call drmaa2_string_free()
    return (r->id != NULL) ? strdup(r->id) : DRMAA2_UNSET_STRING;
}


char *drmaa2_jsession_get_contact(const drmaa2_jsession js)
{
    if (js->contact) return strdup(js->contact);
    return DRMAA2_UNSET_STRING;
}


char *drmaa2_jsession_get_session_name(const drmaa2_jsession js)
{
    return strdup(js->name);
}


drmaa2_string_list drmaa2_jsession_get_job_categories(const drmaa2_jsession js)
{
    drmaa2_string_list jc = drmaa2_list_create(DRMAA2_STRINGLIST, (drmaa2_list_entryfree)drmaa2_string_free);
    int i;
    for (i=0; i<JOBCATEGORIES_LENGTH; i++)
    {
        drmaa2_list_add(jc, strdup(jobcategories[i]));
    }
    return jc;
}


drmaa2_j_list drmaa2_jsession_get_jobs (const drmaa2_jsession js, const drmaa2_jinfo filter)
{
    drmaa2_j_list jobs = drmaa2_list_create(DRMAA2_JOBLIST, DRMAA2_UNSET_CALLBACK);

    drmaa2_list_item current_item = js->jobs->head;
    while (current_item != NULL)
    {
        //TODO: filter evaluation
        drmaa2_list_add(jobs, current_item->data);
        current_item = current_item->next;
    }

    return jobs;
}


drmaa2_j drmaa2_jsession_run_job(const drmaa2_jsession js, const drmaa2_jtemplate jt)
{
    //TODO: complete template evaluation
    int i;
    if ((jt->jobCategory != DRMAA2_UNSET_STRING) && 
        !string_array_contains(jobcategories, JOBCATEGORIES_LENGTH, jt->jobCategory))
    {
        lasterror = DRMAA2_INVALID_ARGUMENT;
        lasterror_text = "Given job category is not supported.";
        return NULL;
    }

    pid_t childpid;

    if ((childpid = fork()) == -1)
    {
        perror("fork failed\n");
        exit(1);
    }
    else if (childpid == 0)
        {
            // child
            long args_size = 0;
            if (jt->args)
                args_size = drmaa2_list_size(jt->args);
            char **argv = (char **)malloc(sizeof(char *) * (args_size + 2));
            argv[0] = jt->remoteCommand;
            if (jt->args)
            {
                //TODO: refactor - get_array call not really useful
                char **args = (char **)stringlist_get_array(jt->args);
                int i;
                for (i=0; i<args_size; i++)
                {
                    argv[i+1] = args[i];
                }
            }
            argv[args_size+1] = NULL;
            execv(jt->remoteCommand, argv);
        }
        else
        {
            // parent
            drmaa2_j j = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
            j->id = NULL;
            j->session_name = (js->name != NULL) ? strdup(js->name) : DRMAA2_UNSET_STRING;
            j->pid = childpid;

            // copy job template, work only with the copy 
            j->template = (drmaa2_jtemplate)malloc(sizeof(drmaa2_jtemplate_s));
            memcpy(j->template, jt, sizeof(drmaa2_jtemplate_s));
            //TODO: deep copy

            drmaa2_jinfo info = drmaa2_jinfo_create();
            info->jobId = (j->id != NULL) ? strdup(j->id) : DRMAA2_UNSET_STRING;
            info->submissionTime = time(NULL);
            info->dispatchTime = time(NULL);

            j->info = info;
            drmaa2_list_add(js->jobs, j);
            return j;
        }
}


char *drmaa2_j_get_id(const drmaa2_j j)
{
    // returns copy since application should call drmaa2_string_free()
    return (j->id != NULL) ? strdup(j->id) : DRMAA2_UNSET_STRING;
}


drmaa2_jinfo drmaa2_j_get_info(const drmaa2_j j)
{
    drmaa2_jinfo info = j->info;
    drmaa2_jinfo i = (drmaa2_jinfo)malloc(sizeof(drmaa2_jinfo_s));
    memcpy(i, info, sizeof(drmaa2_jinfo_s));
    i->jobId                = (i->jobId != NULL) ? strdup(info->jobId) : DRMAA2_UNSET_STRING;
    i->terminatingSignal    = (i->terminatingSignal != NULL) ? strdup(info->terminatingSignal) : DRMAA2_UNSET_STRING;
    i->annotation           = (i->annotation != NULL) ? strdup(info->annotation) : DRMAA2_UNSET_STRING;
    i->jobSubState          = (i->jobSubState != NULL) ? strdup(info->jobSubState) : DRMAA2_UNSET_STRING;
    i->allocatedMachines    = info->allocatedMachines;  //TODO: deep copy
    i->submissionMachine    = (i->submissionMachine != NULL) ? strdup(info->submissionMachine) : DRMAA2_UNSET_STRING;
    i->jobOwner             = (i->jobOwner != NULL) ? strdup(info->jobOwner) : DRMAA2_UNSET_STRING;
    i->queueName            = (i->queueName != NULL) ? strdup(info->queueName): DRMAA2_UNSET_STRING;

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


drmaa2_r_list drmaa2_msession_get_all_reservations(const drmaa2_msession ms)
{
    drmaa2_r_list reservations = drmaa2_list_create(DRMAA2_RESERVATIONLIST, DRMAA2_UNSET_CALLBACK);

    if (r_sessions != DRMAA2_UNSET_LIST)
    {
        drmaa2_list_item current_session_item = r_sessions->head;
        drmaa2_list_item current_item = NULL;       //reservation-item
        drmaa2_rsession current_session = NULL;

        while (current_session_item != NULL)
        {
            current_session = (drmaa2_rsession)current_session_item->data;
            if (current_session->reservations->size != 0)
            {
                current_item = current_session->reservations->head;
                while (current_item != NULL)
                {
                    drmaa2_list_add(reservations, current_item->data);
                    current_item = current_item->next;
                }
            }
            current_session_item = current_session_item->next;
        }
    }

    return reservations;
}


drmaa2_j_list drmaa2_msession_get_all_jobs(const drmaa2_msession ms, const drmaa2_jinfo filter)
{
    drmaa2_j_list jobs = drmaa2_list_create(DRMAA2_JOBLIST, DRMAA2_UNSET_CALLBACK);

    if (j_sessions != DRMAA2_UNSET_LIST)
    {
        drmaa2_list_item current_session_item = j_sessions->head;
        drmaa2_list_item current_item = NULL;       //job-item
        drmaa2_jsession current_session = NULL;

        while (current_session_item != NULL)
        {
            current_session = (drmaa2_jsession)current_session_item->data;
            if (current_session->jobs->size != 0)
            {
                current_item = current_session->jobs->head;
                while (current_item != NULL)
                {
                    //TODO: filter
                    drmaa2_list_add(jobs, current_item->data);
                    current_item = current_item->next;
                }
            }
            current_session_item = current_session_item->next;
        }
    }

    return jobs;
}


drmaa2_queueinfo_list drmaa2_msession_get_all_queues(const drmaa2_msession ms, const drmaa2_string_list names)
{
    //TODO: implement
    return DRMAA2_UNSET_LIST;
}


drmaa2_machineinfo_list drmaa2_msession_get_all_machines(const drmaa2_msession ms, const drmaa2_string_list names)
{
    drmaa2_machineinfo_list ml = drmaa2_list_create(DRMAA2_MACHINEINFOLIST, (drmaa2_list_entryfree)drmaa2_machineinfo_free);

    // TODO: get real machine info
    drmaa2_machineinfo mi = (drmaa2_machineinfo)malloc(sizeof(drmaa2_machineinfo_s));
    mi->name                = strdup("my machine");  
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
    // returns copy since application should call drmaa2_string_free()
    return strdup("drmaa2-mock");
}

drmaa2_version drmaa2_get_drmaa_version(void)
{
    drmaa2_version version = (drmaa2_version)malloc(sizeof(drmaa2_version_s));
    version->major = strdup("2");
    version->minor = strdup("0");
    return version;
}

drmaa2_error drmaa2_version_free(drmaa2_version v)
{
    free(v->major);
    free(v->minor);
};

drmaa2_bool drmaa2_supports(const drmaa2_capability c)
{
    return DRMAA2_FALSE;
}


//TODO: avoid code duplication
drmaa2_jsession drmaa2_create_jsession(const char * session_name, const char * contact)
{
    if (j_sessions == DRMAA2_UNSET_LIST)
    {
        j_sessions = drmaa2_list_create(DRMAA2_LIST, NULL);
    }
    if (session_name == DRMAA2_UNSET_STRING)
    {
        // generate unique name
        session_name = "TODO";
    }
    else
    {
        //TODO: uniqueness test of name
    }

    drmaa2_jsession js = (drmaa2_jsession)malloc(sizeof(drmaa2_jsession_s));
    assert(session_name != DRMAA2_UNSET_STRING);
    js->name = strdup(session_name);
    if (contact) js->contact = strdup(contact);
    else js->contact = DRMAA2_UNSET_STRING;
    js->jobs = drmaa2_list_create(DRMAA2_JOBLIST, DRMAA2_UNSET_CALLBACK);

    drmaa2_list_add(j_sessions, js);
    return js;
}


drmaa2_rsession drmaa2_create_rsession(const char * session_name, const char * contact)
{
    if (r_sessions == DRMAA2_UNSET_LIST)
    {
        r_sessions = drmaa2_list_create(DRMAA2_LIST, NULL);
    }
    if (session_name == DRMAA2_UNSET_STRING)
    {
        // generate unique name
        session_name = "TODO";
    }
    else
    {
        // TODO: uniqueness test of name
    }

    // append to session list
    drmaa2_rsession rs = (drmaa2_rsession)malloc(sizeof(drmaa2_rsession_s));
    assert(session_name != DRMAA2_UNSET_STRING);
    rs->name = strdup(session_name);
    if (contact) rs->contact = strdup(contact);
    else rs->contact = DRMAA2_UNSET_STRING;
    rs->reservations = drmaa2_list_create(DRMAA2_RESERVATIONLIST, DRMAA2_UNSET_CALLBACK);

    drmaa2_list_add(r_sessions, rs);
    return rs;
}


drmaa2_jsession drmaa2_open_jsession(const char * session_name)
{
    if (j_sessions != DRMAA2_UNSET_LIST && session_name != DRMAA2_UNSET_STRING)
    {
        drmaa2_list_item current_item = j_sessions->head;
        drmaa2_jsession js;
        while (current_item != NULL)
        {
            js = (drmaa2_jsession)current_item->data;
            if (strcmp(js->name, session_name) == 0)
            {
                return js;
            }
            current_item = current_item->next;
        }
    }

    // no jobsession || empty session_name given || no match
    lasterror = DRMAA2_INVALID_ARGUMENT;
    lasterror_text = "No session with the given name.";
    return NULL;
}


drmaa2_rsession drmaa2_open_rsession(const char * session_name)
{
    if (r_sessions != DRMAA2_UNSET_LIST && session_name != DRMAA2_UNSET_STRING)
    {
        drmaa2_list_item current_item = r_sessions->head;
        drmaa2_rsession rs;
        while (current_item != NULL)
        {
            rs = (drmaa2_rsession)current_item->data;
            if (strcmp(rs->name, session_name) == 0)
            {
                return rs;
            }
            current_item = current_item->next;
        }
    }

    // no jobsession || empty session_name given || no match
    lasterror = DRMAA2_INVALID_ARGUMENT;
    lasterror_text = "No session with the given name.";
    return NULL;
}


drmaa2_msession drmaa2_open_msession(const char * session_name)
{
    drmaa2_msession ms = (drmaa2_msession)malloc(sizeof(drmaa2_msession_s));
    if (session_name) ms->name = strdup(session_name);
    return ms;
}


drmaa2_error drmaa2_close_jsession(drmaa2_jsession js)
{
    // nothing to do: session information stays until session is destroyed
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_close_rsession(drmaa2_rsession rs)
{
    // nothing to do: session information stays until session is destroyed
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_close_msession(drmaa2_msession ms)
{
    drmaa2_string_free((char *)ms->name);
    free(ms);
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_destroy_jsession(const char * session_name)
{
    if (j_sessions != DRMAA2_UNSET_LIST)
    {
        drmaa2_list_item current_item = j_sessions->head;
        // linked list -> we need to know the item before the one we want to delete
        drmaa2_list_item before_current = NULL;
        drmaa2_jsession js;

        if (current_item != NULL)
        {
            js = (drmaa2_jsession)current_item->data;
            if (strcmp(js->name, session_name) == 0)
            {
                j_sessions->size--;
                // session to destroy is first of list
                //TODO: delete all job information of session
                j_sessions->head = current_item->next;
                return DRMAA2_SUCCESS;
            }
            // linked list -> we need to know the item before the one we want to delete
            before_current = current_item;
            current_item = current_item->next;
        }

        while (current_item != NULL)
        {
            js = (drmaa2_jsession)current_item->data;
            if (strcmp(js->name, session_name) == 0)
            {
                j_sessions->size--;
                // session to destroy found in the middle of list
                //TODO: delete all job information of session
                before_current->next = current_item->next;
                return DRMAA2_SUCCESS;
            }
            before_current = current_item;
            current_item = current_item->next;
        }
    }
    return DRMAA2_INVALID_ARGUMENT;
}


drmaa2_error drmaa2_destroy_rsession(const char * session_name)
{
    if (r_sessions != DRMAA2_UNSET_LIST)
    {
        drmaa2_list_item current_item = r_sessions->head;
        // linked list -> we need to know the item before the one we want to delete
        drmaa2_list_item before_current = NULL;
        drmaa2_rsession rs;

        if (current_item != NULL)
        {
            rs = (drmaa2_rsession)current_item->data;
            if (strcmp(rs->name, session_name) == 0)
            {
                r_sessions->size--;
                // session to destroy is first of list
                //TODO: delete all job information of session
                r_sessions->head = current_item->next;
                return DRMAA2_SUCCESS;
            }
            // linked list -> we need to know the item before the one we want to delete
            before_current = current_item;
            current_item = current_item->next;
        }

        while (current_item != NULL)
        {
            rs = (drmaa2_rsession)current_item->data;
            if (strcmp(rs->name, session_name) == 0)
            {
                r_sessions->size--;
                // session to destroy found in the middle of list
                //TODO: delete all job information of session
                before_current->next = current_item->next;
                return DRMAA2_SUCCESS;
            }
            before_current = current_item;
            current_item = current_item->next;
        }
    }
    return DRMAA2_INVALID_ARGUMENT;
}


drmaa2_string_list drmaa2_get_jsession_names(void)
{
    drmaa2_string_list session_names = drmaa2_list_create(DRMAA2_STRINGLIST, (drmaa2_list_entryfree)drmaa2_string_free);
    if (j_sessions != DRMAA2_UNSET_LIST)
    {
        drmaa2_list_item current_item = j_sessions->head;
        drmaa2_jsession js;
        while (current_item != NULL)
        {
            js = (drmaa2_jsession)current_item->data;
            drmaa2_list_add(session_names, strdup(js->name));
            current_item = current_item->next;
        }
    }
    return session_names;
}

drmaa2_string_list drmaa2_get_rsession_names(void)
{
    drmaa2_string_list session_names = drmaa2_list_create(DRMAA2_STRINGLIST, (drmaa2_list_entryfree)drmaa2_string_free);
    if (r_sessions != DRMAA2_UNSET_LIST)
    {
        drmaa2_list_item current_item = r_sessions->head;
        drmaa2_rsession rs;
        while (current_item != NULL)
        {
            rs = (drmaa2_rsession)current_item->data;
            drmaa2_list_add(session_names, strdup(rs->name));
            current_item = current_item->next;
        }
    }
    return session_names;
}

