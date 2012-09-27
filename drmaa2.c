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




void drmaa2_string_free(drmaa2_string * sRef)
{
    if (*sRef != NULL) {        
        free(*sRef);
        *sRef = NULL;
    }
}


drmaa2_error drmaa2_lasterror(void)
{
    return drmaa2_lasterror_v;
}


drmaa2_string drmaa2_lasterror_text(void)
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


void drmaa2_jinfo_free(drmaa2_jinfo * jiRef)
{
    drmaa2_jinfo ji = *jiRef;
    if (ji != NULL) {
        drmaa2_string_free(&(ji->jobId));
        drmaa2_string_free(&(ji->terminatingSignal));
        drmaa2_string_free(&(ji->annotation));
        drmaa2_string_free(&(ji->jobSubState));
        drmaa2_list_free  (&(ji->allocatedMachines));
        drmaa2_string_free(&(ji->submissionMachine));
        drmaa2_string_free(&(ji->jobOwner));
        drmaa2_string_free(&(ji->queueName));
        free(ji);
        *jiRef = NULL;
    }
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


void drmaa2_rinfo_free(drmaa2_rinfo * riRef)
{
    drmaa2_rinfo ri = *riRef;
    if (ri != NULL) {
        drmaa2_string_free(&(ri->reservationId));
        drmaa2_string_free(&(ri->reservationName));
        drmaa2_list_free  (&(ri->usersACL));
        drmaa2_list_free  (&(ri->reservedMachines));
        free(ri);
        *riRef = NULL;
    }
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


void drmaa2_jtemplate_free(drmaa2_jtemplate * jtRef)
{
    drmaa2_jtemplate jt = *jtRef;
    if (jt != NULL) {
        drmaa2_string_free(&(jt->remoteCommand));
        drmaa2_list_free(&(jt->args));          
        drmaa2_dict_free(&(jt->jobEnvironment));
        drmaa2_string_free(&(jt->workingDirectory));        
        drmaa2_string_free(&(jt->jobCategory));            
        drmaa2_list_free(&(jt->email));
        drmaa2_string_free(&(jt->jobName));              
        drmaa2_string_free(&(jt->inputPath));
        drmaa2_string_free(&(jt->outputPath));
        drmaa2_string_free(&(jt->errorPath));
        drmaa2_string_free(&(jt->reservationId));
        drmaa2_string_free(&(jt->queueName));
        drmaa2_list_free(&(jt->candidateMachines));
        drmaa2_dict_free(&(jt->stageInFiles)); 
        drmaa2_dict_free(&(jt->stageOutFiles));
        drmaa2_dict_free(&(jt->resourceLimits));
        drmaa2_string_free(&(jt->accountingId));
        free(jt);
        *jtRef = NULL;
    }
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


void drmaa2_rtemplate_free(drmaa2_rtemplate * rtRef)
{
    drmaa2_rtemplate rt = *rtRef;
    if (rt != NULL) {
        drmaa2_string_free(&(rt->reservationName));          
        drmaa2_string_free(&(rt->jobCategory));
        drmaa2_list_free(&(rt->usersACL));            
        drmaa2_list_free(&(rt->candidateMachines));
        free(rt);
        *rtRef = NULL;
    }
}


void drmaa2_queueinfo_free(drmaa2_queueinfo * qiRef)
{
    drmaa2_queueinfo qi = *qiRef;
    if (qi != NULL) {
        drmaa2_string_free(&(qi->name));          
        free(qi);
        *qiRef = NULL;
    }
}


void drmaa2_machineinfo_free(drmaa2_machineinfo * miRef)
{
    if (*miRef != NULL) {
        drmaa2_string_free (&((*miRef)->name));
        drmaa2_version_free(&((*miRef)->machineOSVersion));
        free(*miRef);
        *miRef = NULL;
    }
}


void drmaa2_jsession_free(drmaa2_jsession * jsRef)
{
    if (*jsRef != NULL) {
        drmaa2_string_free(&((*jsRef)->contact));
        drmaa2_string_free(&((*jsRef)->name));
        free(*jsRef);
        *jsRef = NULL;
    }
}

void drmaa2_rsession_free(drmaa2_rsession * rsRef)
{
    if (*rsRef != NULL) {
        drmaa2_string_free(&((*rsRef)->contact));
        drmaa2_string_free(&((*rsRef)->name));
        free(*rsRef);
        *rsRef = NULL;
    }
}

void drmaa2_msession_free(drmaa2_msession * msRef)
{
    if (*msRef != NULL) {
        drmaa2_string_free(&((*msRef)->name));
        free(*msRef);
        *msRef = NULL;
    }
}

void drmaa2_j_free(drmaa2_j * jRef)
{
    if (*jRef != NULL) {
        drmaa2_string_free(&((*jRef)->id));
        drmaa2_string_free(&((*jRef)->session_name));
        free(*jRef);
        *jRef=NULL;
    }
}

void drmaa2_jarray_free(drmaa2_jarray * jaRef)
{

}

void drmaa2_r_free(drmaa2_r * rRef)
{
    if (*rRef != NULL) {
        drmaa2_string_free(&((*rRef)->id));
        drmaa2_string_free(&((*rRef)->session_name));
        free(*rRef);
        *rRef=NULL;
    }
}


drmaa2_string drmaa2_rsession_get_contact(const drmaa2_rsession rs)
{
    if (!drmaa2_rsession_is_valid(rs->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Reservation session is invalid.";
        return NULL;
    }
    
    if (rs->contact) return strdup(rs->contact);
            return DRMAA2_UNSET_STRING;
}


drmaa2_string drmaa2_rsession_get_session_name(const drmaa2_rsession rs)
{
    if (!drmaa2_rsession_is_valid(rs->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Reservation session is invalid.";
        return NULL;
    }

    return strdup(rs->name);
}


drmaa2_r drmaa2_rsession_get_reservation(const drmaa2_rsession rs, const drmaa2_string reservationId)
{
    if (!drmaa2_rsession_is_valid(rs->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Reservation session is invalid.";
        return NULL;
    }

    drmaa2_r r = drmaa2_get_reservation(reservationId);
    if (r == NULL)
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
        drmaa2_lasterror_text_v = "Reservation ID is invalid.";
    }
    return r;    
}


drmaa2_r drmaa2_rsession_request_reservation(const drmaa2_rsession rs, const drmaa2_rtemplate rt)
{
    if (!drmaa2_rsession_is_valid(rs->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Reservation session is invalid.";
        return NULL;
    }

    char *reservation_name = rt->reservationName ? rt->reservationName : "reservation"; 
    long long template_id = save_rtemplate(rt, rs->name);
    long long id = save_reservation(rs->name, template_id, reservation_name); 

    drmaa2_r r = (drmaa2_r)malloc(sizeof(drmaa2_r_s));
    char *cid;
    asprintf(&cid, "%lld", id);
    r->id = cid; //already allocated
    r->session_name = strdup(rs->name);

    return r;
}


drmaa2_r_list drmaa2_rsession_get_reservations(const drmaa2_rsession rs)
{
        if (!drmaa2_rsession_is_valid(rs->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Reservation session is invalid.";
        return NULL;
    }


    drmaa2_r_list reservations = drmaa2_list_create(DRMAA2_RESERVATIONLIST, (drmaa2_list_entryfree)drmaa2_r_free);
    reservations = drmaa2_get_session_reservations(reservations, rs->name);

    return reservations;
}


drmaa2_string drmaa2_r_get_id(const drmaa2_r r)
{
    //returns copy since application should call drmaa2_string_free()
    return strdup(r->id);
}


drmaa2_string drmaa2_r_get_session_name(const drmaa2_r r)
{
    return strdup(r->session_name);
}


drmaa2_rtemplate  drmaa2_r_get_reservation_template(const drmaa2_r r)
{
    drmaa2_rtemplate rt = drmaa2_rtemplate_create();
    rt = drmaa2_get_rtemplate(rt, r->id);
    return rt;
}


drmaa2_rinfo drmaa2_r_get_info(const drmaa2_r r)
{
    drmaa2_rinfo ri = drmaa2_rinfo_create();
    ri->reservationId = strdup(r->id);
    ri = drmaa2_get_rinfo(ri);
    return ri; 
}


//TODO: implement
//drmaa2_error      drmaa2_r_terminate                (drmaa2_r r);


drmaa2_string drmaa2_jsession_get_contact(const drmaa2_jsession js)
{
    if (!drmaa2_jsession_is_valid(js->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Job session is invalid.";
        return NULL;
    }
    
    if (js->contact) return strdup(js->contact);
            return DRMAA2_UNSET_STRING;
}


drmaa2_string drmaa2_jsession_get_session_name(const drmaa2_jsession js)
{
    if (!drmaa2_jsession_is_valid(js->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Job session is invalid.";
        return NULL;
    }

    return strdup(js->name);
}


drmaa2_string_list drmaa2_jsession_get_job_categories(const drmaa2_jsession js)
{
    if (!drmaa2_jsession_is_valid(js->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Job session is invalid.";
        return NULL;
    }

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
    if (!drmaa2_jsession_is_valid(js->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Job session is invalid.";
        return NULL;
    }

    drmaa2_j_list jobs = drmaa2_list_create(DRMAA2_JOBLIST, (drmaa2_list_entryfree)drmaa2_j_free);
    jobs = get_session_jobs(jobs, js->name);

    return jobs;
}


drmaa2_j drmaa2_jsession_run_job(const drmaa2_jsession js, const drmaa2_jtemplate jt)
{
    if (!drmaa2_jsession_is_valid(js->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Job session is invalid.";
        return NULL;
    }

    //TODO: complete template evaluation
    if ((jt->jobCategory != DRMAA2_UNSET_STRING) && 
        !string_array_contains(jobcategories, JOBCATEGORIES_LENGTH, jt->jobCategory))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
        drmaa2_lasterror_text_v = "Given job category is not supported.";
        return NULL;
    }

    long long template_id = save_jtemplate(jt, js->name);

    long long id = save_job(js->name, template_id); 

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
            char *args[] = {"./wrapper", id_c, NULL};
            execv(args[0], args);
            return NULL;        // dead code, just to avoid a GCC warning about control end reach
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


drmaa2_string drmaa2_j_get_id(const drmaa2_j j)
{
    // returns copy since application should call drmaa2_string_free()
    return strdup(j->id);
}


drmaa2_string drmaa2_j_get_session_name(const drmaa2_j j)
{
    return strdup(j->session_name);
}


drmaa2_jtemplate drmaa2_j_get_jt(const drmaa2_j j)
{
    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    jt = drmaa2_get_job_template(jt, j->id);
    return jt;
}


drmaa2_error drmaa2_j_suspend(drmaa2_j j)
{
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_j_resume(drmaa2_j j)
{
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_j_hold(drmaa2_j j)
{
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_j_release(drmaa2_j j)
{
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_j_terminate(drmaa2_j j)
{
    return DRMAA2_SUCCESS;
}


drmaa2_jstate drmaa2_j_get_state(const drmaa2_j j, drmaa2_string * substate)
{
    //TODO: implement
}


drmaa2_jinfo drmaa2_j_get_info(const drmaa2_j j)
{
    drmaa2_jinfo ji = drmaa2_jinfo_create();
    ji->jobId = strdup(j->id);
    
    ji = get_job_info(ji); // exitStatus, terminating signal and *_time are set
    
    //TODO set: jobSubState, allocatedMachines, i->submissionMachine, jobOwner, queueName 
    return ji;
}


drmaa2_error drmaa2_j_wait_terminated(const drmaa2_j j, const time_t timeout)
{
    DRMAA2_DEBUG_PRINT("wait for job with id: %s\n", j->id);
    int status = -1;
    while (status == -1)
    {
        status = drmaa2_get_job_status(j);
        sleep(1);
    }
    
    return DRMAA2_SUCCESS;
}


drmaa2_r_list drmaa2_msession_get_all_reservations(const drmaa2_msession ms)
{
    drmaa2_r_list reservations = drmaa2_list_create(DRMAA2_RESERVATIONLIST, (drmaa2_list_entryfree)drmaa2_r_free);
    reservations = get_reservations(reservations);
    return reservations;
}


drmaa2_j_list drmaa2_msession_get_all_jobs(const drmaa2_msession ms, const drmaa2_jinfo filter)
{
    drmaa2_j_list jobs = drmaa2_list_create(DRMAA2_JOBLIST, (drmaa2_list_entryfree)drmaa2_j_free);
    jobs = get_jobs(jobs, filter);
    return jobs;
}


drmaa2_queueinfo_list drmaa2_msession_get_all_queues(const drmaa2_msession ms, const drmaa2_string_list names)
{
    drmaa2_queueinfo_list ql = drmaa2_list_create(DRMAA2_QUEUEINFOLIST, (drmaa2_list_entryfree)drmaa2_queueinfo_free);
    if (names == DRMAA2_UNSET_LIST) {
        // return all queue info instances
        drmaa2_queueinfo qi = (drmaa2_queueinfo)malloc(sizeof(drmaa2_queueinfo_s));
        qi->name = strdup("default");
        drmaa2_list_add(ql, qi);
    }
    else {
        int i;
        for (i = 0; i < drmaa2_list_size(names); i++) {
            if (!strcmp(drmaa2_list_get(names, i), "default")) {
                drmaa2_queueinfo qi = (drmaa2_queueinfo)malloc(sizeof(drmaa2_queueinfo_s));
                qi->name = strdup("default");
                drmaa2_list_add(ql, qi);
            }
        }
    }
    return ql;
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


drmaa2_string drmaa2_get_drms_name(void)
{
    return NULL;
}

drmaa2_version drmaa2_get_drms_version(void)
{
    return NULL;
}

drmaa2_string drmaa2_get_drmaa_name(void)
{
    // returns copy since application should call drmaa2_string_free()
    return strdup("drmaa2-mock");
}

drmaa2_version drmaa2_get_drmaa_version(void)
{
    drmaa2_version version = (drmaa2_version)malloc(sizeof(drmaa2_version_s));
    version->major = strdup("0");
    version->minor = strdup("1");
    return version;
}

void drmaa2_version_free(drmaa2_version * vRef)
{
    if (*vRef != NULL) {
        drmaa2_string_free(&((*vRef)->major));
        drmaa2_string_free(&((*vRef)->minor));
        free(*vRef);
        *vRef=NULL;
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
        char *name = drmaa2_generate_unique_name("jsession");
        if (name == NULL)
            return NULL;
        js->name = name;
    }
    else
        js->name = strdup(session_name);

    assert(session_name != DRMAA2_UNSET_STRING);
    js->contact = (contact != NULL) ? strdup(contact) : DRMAA2_UNSET_STRING;

    if (save_jsession(contact, session_name) != 0)
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
        char *name = drmaa2_generate_unique_name("rsession");
        if (name == NULL)
            return NULL;
        rs->name = name;
    }
    else
        rs->name = strdup(session_name);

    assert(session_name != DRMAA2_UNSET_STRING);
    rs->contact = (contact != NULL) ? strdup(contact) : DRMAA2_UNSET_STRING;

    if (save_rsession(contact, session_name) != 0)
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
        drmaa2_jsession js = get_jsession(session_name);
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
        drmaa2_rsession rs = get_rsession(session_name);
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
    if (drmaa2_jsession_is_valid(js->name))
        return DRMAA2_SUCCESS;
    else
        return DRMAA2_INVALID_SESSION;
}


drmaa2_error drmaa2_close_rsession(drmaa2_rsession rs)
{
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_close_msession(drmaa2_msession ms)
{
    return DRMAA2_SUCCESS;
}


drmaa2_error drmaa2_destroy_jsession(const char * session_name)
{
    if (session_name == NULL)
        return DRMAA2_INVALID_ARGUMENT;

    int status = delete_jsession(session_name);
    return status;
}


drmaa2_error drmaa2_destroy_rsession(const char * session_name)
{
    if (session_name == NULL)
        return DRMAA2_INVALID_ARGUMENT;

    int status = delete_rsession(session_name);
        return status;
}


drmaa2_string_list drmaa2_get_jsession_names(void)
{
    drmaa2_string_list session_names = drmaa2_list_create(DRMAA2_STRINGLIST, (drmaa2_list_entryfree)drmaa2_string_free);
    session_names = get_jsession_names(session_names);
    return session_names;
}


drmaa2_string_list drmaa2_get_rsession_names(void)
{
    drmaa2_string_list session_names = drmaa2_list_create(DRMAA2_STRINGLIST, (drmaa2_list_entryfree)drmaa2_string_free);
    session_names = get_rsession_names(session_names);
    return session_names;
}

