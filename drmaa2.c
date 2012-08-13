#include "drmaa2.h"
#include "drmaa2-list.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "drmaa2-mock.h"
#include "persistence.h"
#include "drmaa2-debug.h"


// internal global error variables
int drmaa2_lasterror_v          = DRMAA2_SUCCESS;
char *drmaa2_lasterror_text_v   = NULL;

// supported jobcategories
// TODO: Programmatic
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




void drmaa2_string_free(char* string)
{
    free(string);
}


drmaa2_error drmaa2_lasterror(void)
{
    return drmaa2_lasterror_v;
}


char *drmaa2_lasterror_text(void)
{
    return drmaa2_lasterror_text_v ? strdup(drmaa2_lasterror_text_v) : NULL;
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


void drmaa2_jinfo_free(drmaa2_jinfo ji)
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
}


// no drmaa function - only used by implementation
drmaa2_rinfo drmaa2_rinfo_create(void)
{
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


void drmaa2_rinfo_free(drmaa2_rinfo ri)
{
    drmaa2_string_free(ri->reservationId);
    drmaa2_string_free(ri->reservationName);
    drmaa2_list_free(ri->usersACL);
    drmaa2_list_free(ri->reservedMachines);
    free(ri);
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


void drmaa2_jtemplate_free(drmaa2_jtemplate jt)
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


void drmaa2_rtemplate_free(drmaa2_rtemplate rt)
{
    // free fields
    free(rt->reservationName);          
    free(rt->jobCategory);
    drmaa2_list_free(rt->usersACL);            
    drmaa2_list_free(rt->candidateMachines);
    // free container 
    free(rt);
}


void drmaa2_machineinfo_free(drmaa2_machineinfo mi)
{
    drmaa2_string_free(mi->name);
    drmaa2_version_free(mi->machineOSVersion);
    free(mi);
}


drmaa2_r drmaa2_rsession_request_reservation(const drmaa2_rsession rs, const drmaa2_rtemplate rt)
{
    long long template_id = save_rtemplate(DB_NAME, rt);
    long long id = save_reservation(DB_NAME, rs->name, template_id); 

    drmaa2_r r = (drmaa2_r)malloc(sizeof(drmaa2_r_s));
    char *cid;
    asprintf(&cid, "%lld\n", id);
    r->id = cid; //already allocated
    r->session_name = strdup(rs->name);

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
    jobs = get_session_jobs(DB_NAME, jobs, js->name);

    return jobs;
}


drmaa2_j drmaa2_jsession_run_job(const drmaa2_jsession js, const drmaa2_jtemplate jt)
{
    //TODO: complete template evaluation
    int i;
    if ((jt->jobCategory != DRMAA2_UNSET_STRING) && 
        !string_array_contains(jobcategories, JOBCATEGORIES_LENGTH, jt->jobCategory))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
        drmaa2_lasterror_text_v = "Given job category is not supported.";
        return NULL;
    }

    long long template_id = save_jtemplate(DB_NAME, jt);

    long long id = save_job(DB_NAME, js->name, template_id); 

    pid_t childpid;

    if ((childpid = fork()) == -1)
    {
        perror("fork failed\n");
        exit(1);
    }
    else if (childpid == 0)
        {
            // child
            char *id_c;
            asprintf(&id_c, "%lld", id);
            char *args[] = {"./wrapper", DB_NAME, id_c, NULL};
            execv(args[0], args);
        }
        else
        {
            // parent
            drmaa2_j j = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
            char *cid;
            asprintf(&cid, "%lld\n", id);
            j->id = cid; //already allocated
            j->session_name = strdup(js->name);
            return j;
        }
}


char *drmaa2_j_get_id(const drmaa2_j j)
{
    // returns copy since application should call drmaa2_string_free()
    return strdup(j->id);
}


drmaa2_jinfo drmaa2_j_get_info(const drmaa2_j j)
{
    drmaa2_jinfo ji = drmaa2_jinfo_create();
    ji->jobId = strdup(j->id);
    
    ji = get_job_info(DB_NAME, ji); // exitStatus, terminating signal and *_time are set
    
    //TODO set: jobSubState, allocatedMachines, i->submissionMachine, jobOwner, queueName 
    return ji;
}


drmaa2_error drmaa2_j_wait_terminated(const drmaa2_j j, const time_t timeout)
{
    DRMAA2_DEBUG_PRINT("wait for job with id: %s\n", j->id);
    int status = -1;
    while (status == -1)
    {
        status = drmaa2_get_job_status(DB_NAME, j);
        sleep(1);
    }
    
    return DRMAA2_SUCCESS;
}


drmaa2_r_list drmaa2_msession_get_all_reservations(const drmaa2_msession ms)
{
    drmaa2_r_list reservations = drmaa2_list_create(DRMAA2_RESERVATIONLIST, DRMAA2_UNSET_CALLBACK);
    reservations = get_reservations(DB_NAME, reservations);
    return reservations;
}


drmaa2_j_list drmaa2_msession_get_all_jobs(const drmaa2_msession ms, const drmaa2_jinfo filter)
{
    drmaa2_j_list jobs = drmaa2_list_create(DRMAA2_JOBLIST, DRMAA2_UNSET_CALLBACK);
    jobs = get_jobs(DB_NAME, jobs, filter);
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

void drmaa2_version_free(drmaa2_version v)
{
    if (v)
    {
        free(v->major);
        free(v->minor);
        free(v);
    }
};

drmaa2_bool drmaa2_supports(const drmaa2_capability c)
{
    return DRMAA2_FALSE;
}


char *drmaa2_generate_unique_name(char* prefix)
{
    //TODO: use uuid
    srand(time(NULL));
    int r = rand();
    char *name;
    if (asprintf(&name, "%s%i", prefix, r) == -1)
    {
        printf("BAD ALLOCATION\n");
        drmaa2_lasterror_v = DRMAA2_OUT_OF_RESOURCE;
        drmaa2_lasterror_text_v = "Could not allocate enough memory.";
        return NULL;
    };
    return name;
}


//TODO: avoid code duplication
drmaa2_jsession drmaa2_create_jsession(const char * session_name, const char * contact)
{
    drmaa2_jsession js = (drmaa2_jsession)malloc(sizeof(drmaa2_jsession_s));

    if (session_name == DRMAA2_UNSET_STRING)
    {
        char *name = drmaa2_generate_unique_name("jseesion");
        if (name == NULL)
            return NULL;
        js->name = name;
    }
    else
        js->name = strdup(session_name);

    assert(session_name != DRMAA2_UNSET_STRING);
    js->contact = (contact != NULL) ? strdup(contact) : DRMAA2_UNSET_STRING;

    if (save_jsession(DB_NAME, contact, session_name) != 0)
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
        drmaa2_lasterror_text_v = "Could not store session information.";
        return NULL;
    }

    return js;
}


drmaa2_rsession drmaa2_create_rsession(const char * session_name, const char * contact)
{
    drmaa2_rsession rs = (drmaa2_rsession)malloc(sizeof(drmaa2_rsession_s));

    if (session_name == DRMAA2_UNSET_STRING)
    {
        char *name = drmaa2_generate_unique_name("jseesion");
        if (name == NULL)
            return NULL;
        rs->name = name;
    }
    else
        rs->name = strdup(session_name);

    assert(session_name != DRMAA2_UNSET_STRING);
    rs->contact = (contact != NULL) ? strdup(contact) : DRMAA2_UNSET_STRING;

    if (save_rsession(DB_NAME, contact, session_name) != 0)
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
        drmaa2_lasterror_text_v = "Could not store session information.";
        return NULL;
    }

    return rs;
}


drmaa2_jsession drmaa2_open_jsession(const char * session_name)
{
    if (session_name != DRMAA2_UNSET_STRING)
    {
        drmaa2_jsession js = get_jsession(DB_NAME, session_name);
        if (js)
            return js;
    }

    // empty session_name given || no match
    drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
    drmaa2_lasterror_text_v = "No session with the given name.";
    return NULL;
}


drmaa2_rsession drmaa2_open_rsession(const char * session_name)
{
    if (session_name != DRMAA2_UNSET_STRING)
    {
        drmaa2_rsession rs = get_rsession(DB_NAME, session_name);
        if (rs)
            return rs;
    }

    // empty session_name given || no match
    drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
    drmaa2_lasterror_text_v = "No session with the given name.";
    return NULL;
}


drmaa2_msession drmaa2_open_msession(const char * session_name)
{
    drmaa2_msession ms = (drmaa2_msession)malloc(sizeof(drmaa2_msession_s));
    ms->name = session_name ? strdup(session_name) : NULL;
    return ms;
}


drmaa2_error drmaa2_close_jsession(drmaa2_jsession js)
{
    free((char *)js->contact);
    free((char *)js->name);
    free(js);
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_close_rsession(drmaa2_rsession rs)
{
    free((char *)rs->contact);
    free((char *)rs->name);
    free(rs);
    return DRMAA2_SUCCESS;
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
    if (session_name == NULL)
        return DRMAA2_INVALID_ARGUMENT;

    int status = delete_jsession(DB_NAME, session_name);
    return status;
}


drmaa2_error drmaa2_destroy_rsession(const char * session_name)
{
    if (session_name == NULL)
        return DRMAA2_INVALID_ARGUMENT;

    int status = delete_rsession(DB_NAME, session_name);
        return status;
}


drmaa2_string_list drmaa2_get_jsession_names(void)
{
    drmaa2_string_list session_names = drmaa2_list_create(DRMAA2_STRINGLIST, (drmaa2_list_entryfree)drmaa2_string_free);
    session_names = get_jsession_names(DB_NAME, session_names);
    return session_names;
}


drmaa2_string_list drmaa2_get_rsession_names(void)
{
    drmaa2_string_list session_names = drmaa2_list_create(DRMAA2_STRINGLIST, (drmaa2_list_entryfree)drmaa2_string_free);
    session_names = get_rsession_names(DB_NAME, session_names);
    return session_names;
}

