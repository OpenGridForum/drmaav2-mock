#include "drmaa2.h"
#include "drmaa2-list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>

#include "drmaa2-specific.h"
#include "drmaa2-persistence.h"
#include "drmaa2-debug.h"


// internal global error variables
int drmaa2_lasterror_v          = DRMAA2_SUCCESS;
char *drmaa2_lasterror_text_v   = NULL;


// callback is global
drmaa2_callback current_drmaa2_callback = NULL;


void call_state_chage_notification(drmaa2_j j, drmaa2_jstate state) {
    drmaa2_notification n = (drmaa2_notification)malloc(sizeof(drmaa2_notification_s));
    n->event = DRMAA2_NEW_STATE;
    n->jobId = strdup(j->id);
    n->sessionName = strdup(j->session_name);
    n->jobState = state;
    current_drmaa2_callback(&n);
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


void drmaa2_slotinfo_free(drmaa2_slotinfo * siRef)
{
    drmaa2_slotinfo si = *siRef;
    if (si != NULL) {
        drmaa2_string_free(&(si->machineName));
        free(si);
        *siRef = NULL;
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


void drmaa2_notification_free(drmaa2_notification *nRef)
{
    drmaa2_notification n = *nRef;
    if (n != NULL) {
        drmaa2_string_free(&(n->jobId));
        drmaa2_string_free(&(n->sessionName));
        free(n);
        *nRef = NULL;
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


void drmaa2_version_free(drmaa2_version * vRef)
{
    if (*vRef != NULL) {
        drmaa2_string_free(&((*vRef)->major));
        drmaa2_string_free(&((*vRef)->minor));
        free(*vRef);
        *vRef=NULL;
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


// drmaa2 reflective interface is implemented by drmaa2-specific.c


// drmaa2 interface types and corresponding free functions are implemented by drmaa2-specific


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


drmaa2_error drmaa2_r_terminate (drmaa2_r r) {
    return DRMAA2_SUCCESS;
}


drmaa2_string drmaa2_jarray_get_id(const drmaa2_jarray ja)
{
    return strdup(ja->id);
}


drmaa2_j_list drmaa2_jarray_get_jobs(const drmaa2_jarray ja)
{
    drmaa2_j_list jl = get_jobs_of_jarray(ja);
    return jl;
}


drmaa2_string drmaa2_jarray_get_session_name(const drmaa2_jarray ja)
{
    return strdup(ja->session_name);
}


drmaa2_jtemplate drmaa2_jarray_get_job_template(const drmaa2_jarray ja)
{
    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    return drmaa2_get_jobarray_template(jt, ja->id);
}


// helper method to avoid code duplication
drmaa2_error drmaa2_jarray_change_state(drmaa2_jarray ja, drmaa2_error (*chage_state_method)(drmaa2_j))
{
    drmaa2_j_list jl = drmaa2_jarray_get_jobs(ja);
    size_t i;
    for (i = 0; i < drmaa2_list_size(jl); i++)
    {
        chage_state_method((drmaa2_j)drmaa2_list_get(jl, i));
    }
    drmaa2_list_free(&jl);
    return DRMAA2_SUCCESS;
}

drmaa2_error drmaa2_jarray_suspend (drmaa2_jarray ja)
{
    return drmaa2_jarray_change_state(ja, drmaa2_j_suspend);
}

drmaa2_error drmaa2_jarray_resume (drmaa2_jarray ja)
{
    return drmaa2_jarray_change_state(ja, drmaa2_j_resume);
}

drmaa2_error drmaa2_jarray_hold (drmaa2_jarray ja)
{
    return drmaa2_jarray_change_state(ja, drmaa2_j_hold);
}

drmaa2_error drmaa2_jarray_release (drmaa2_jarray ja)
{
    return drmaa2_jarray_change_state(ja, drmaa2_j_release);
}

drmaa2_error drmaa2_jarray_terminate (drmaa2_jarray ja)
{
    return drmaa2_jarray_change_state(ja, drmaa2_j_terminate);    
}


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
    return add_supported_job_categories(jc);
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


drmaa2_jarray drmaa2_jsession_get_job_array(const drmaa2_jsession js, const drmaa2_string jobarrayId)
{
    if (!drmaa2_jsession_is_valid(js->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Job session is invalid.";
        return NULL;
    }

    drmaa2_jarray ja = NULL;
    if (jarray_exists(js->name, jobarrayId))
    {
        ja = (drmaa2_jarray)malloc(sizeof(drmaa2_jarray));
        ja->id = strdup(jobarrayId);
        ja->session_name = strdup(js->name);
    }
    else
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
        drmaa2_lasterror_text_v = "Session does not / no longer contain the according job array.";
    }
    return ja;
}


drmaa2_j drmaa2_jsession_run_job(const drmaa2_jsession js, const drmaa2_jtemplate jt)
{
    if (!drmaa2_jsession_is_valid(js->name))
    {
        drmaa2_lasterror_v = DRMAA2_INVALID_SESSION;
        drmaa2_lasterror_text_v = "Job session is invalid.";
        return NULL;
    }

    if (jt->jobCategory != DRMAA2_UNSET_STRING) {
        drmaa2_string_list sl = drmaa2_jsession_get_job_categories(js);
        drmaa2_bool supported = DRMAA2_FALSE;
        size_t i;
        for (i = 0; i < drmaa2_list_size(sl); i++) {
            if (strcmp(jt->jobCategory, drmaa2_list_get(sl, i)) == 0) {
                supported = DRMAA2_TRUE;
                break;
            }
        }
        if (supported == DRMAA2_FALSE) {
            drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
            drmaa2_lasterror_text_v = "Given job category is not supported.";
            return NULL;
        }
    }

    if (!drmaa2_supports(DRMAA2_JT_EMAIL) && (jt->email != DRMAA2_UNSET_LIST
            || jt->emailOnStarted != DRMAA2_UNSET_BOOL || jt->emailOnTerminated != DRMAA2_UNSET_BOOL)) {
        drmaa2_lasterror_v = DRMAA2_UNSUPPORTED_ATTRIBUTE;
        drmaa2_lasterror_text_v = "Email is not supported.";
        return NULL;
    }

    if (!drmaa2_supports(DRMAA2_JT_MAXSLOTS) && jt->maxSlots != DRMAA2_UNSET_NUM) {
        drmaa2_lasterror_v = DRMAA2_UNSUPPORTED_ATTRIBUTE;
        drmaa2_lasterror_text_v = "MaxSlot is not supported.";
        return NULL;
    }

    if (!drmaa2_supports(DRMAA2_JT_DEADLINE) && jt->deadlineTime != DRMAA2_UNSET_TIME) {
        drmaa2_lasterror_v = DRMAA2_UNSUPPORTED_ATTRIBUTE;
        drmaa2_lasterror_text_v = "DeadLineTime is not supported.";
        return NULL;
    }

    if (!drmaa2_supports(DRMAA2_JT_STAGING) && (jt->stageInFiles != DRMAA2_UNSET_DICT ||
            jt->stageOutFiles != DRMAA2_UNSET_DICT)) {
        drmaa2_lasterror_v = DRMAA2_UNSUPPORTED_ATTRIBUTE;
        drmaa2_lasterror_text_v = "Staging is not supported.";
        return NULL;
    }

    if (!drmaa2_supports(DRMAA2_JT_ACCOUNTINGID) && jt->accountingId != DRMAA2_UNSET_STRING) {
        drmaa2_lasterror_v = DRMAA2_UNSUPPORTED_ATTRIBUTE;
        drmaa2_lasterror_text_v = "AccountingID is not supported.";
        return NULL;
    }

    // further evaluation can be done here

    long long template_id = save_jtemplate(jt, js->name);
    long long job_id = save_job(js->name, template_id); 
    return submit_job_to_DRMS(js, job_id, jt);
}


drmaa2_jarray drmaa2_jsession_run_bulk_jobs(const drmaa2_jsession js, const drmaa2_jtemplate jt, 
    unsigned long begin_index, unsigned long end_index, unsigned long step, unsigned long max_parallel)
{
    if (end_index < begin_index) {
        drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
        drmaa2_lasterror_text_v = "The beginIndex value must be less than or equal to endIndex.";
        return NULL;
    } else if (begin_index < 1) {
        drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
        drmaa2_lasterror_text_v = "Only positive index numbers are allowed for the beginIndex.";
        return NULL;
    }
    unsigned long index = begin_index;
    drmaa2_j j;

    drmaa2_string_list sl = drmaa2_list_create(DRMAA2_STRINGLIST, (drmaa2_list_entryfree)drmaa2_string_free);

    while (index <= end_index) {
        printf("index: %lu\n", index);

        //TODO: index placeholders

        j = drmaa2_jsession_run_job(js, jt);
        drmaa2_list_add(sl, strdup(j->id));
        drmaa2_j_free(&j);
        index += step;
    }

    long long template_id = save_jtemplate(jt, js->name);

    long long id = save_jarray(js->name, template_id, sl);
    drmaa2_list_free(&sl);
    drmaa2_jarray ja = (drmaa2_jarray)malloc(sizeof(drmaa2_jarray_s));
    char *cid;
    asprintf(&cid, "%lld\n", id);
    ja->id = cid; //already allocated
    ja->session_name = strdup(js->name);
    return ja;
}


drmaa2_j drmaa2_jsession_wait_any_started (const drmaa2_jsession js, const drmaa2_j_list l, 
    const time_t timeout)
{
    drmaa2_j started_j = NULL;
    drmaa2_j current_j = NULL;
    drmaa2_jstate state;
    size_t i;
    while (started_j == NULL) {
        for (i = 0; i < drmaa2_list_size(l); i++) {
            current_j = (drmaa2_j)drmaa2_list_get(l, i);
            state = get_state(current_j);
            if (state != DRMAA2_QUEUED && state != DRMAA2_QUEUED_HELD) {
                started_j = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
                started_j->session_name = strdup(js->name);
                started_j->id = strdup(current_j->id);
                break;
            }
        }
        if (started_j != NULL) {
            break;
        } else if (timeout == DRMAA2_ZERO_TIME || timeout <= time(NULL)) {
            drmaa2_lasterror_v = DRMAA2_TIMEOUT;
            drmaa2_lasterror_text_v = "A timeout occured while waiting for job termination.";
            break;
        }
        sleep(1);
    }
    return started_j;
}


drmaa2_j drmaa2_jsession_wait_any_terminated (const drmaa2_jsession js, 
    const drmaa2_j_list l, const time_t timeout)
{
    drmaa2_j terminated_j = NULL;
    drmaa2_j current_j = NULL;
    drmaa2_jstate state;
    size_t i;
    while (terminated_j == NULL) {
        for (i = 0; i < drmaa2_list_size(l); i++) {
            current_j = (drmaa2_j)drmaa2_list_get(l, i);
            state = get_state(current_j);
            if (state == DRMAA2_DONE || state == DRMAA2_FAILED) {
                terminated_j = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
                terminated_j->session_name = strdup(js->name);
                terminated_j->id = strdup(current_j->id);
                break;
            }
        }
        if (terminated_j != NULL) {
            break;
        } else if (timeout == DRMAA2_ZERO_TIME || timeout <= time(NULL)) {
            drmaa2_lasterror_v = DRMAA2_TIMEOUT;
            drmaa2_lasterror_text_v = "A timeout occured while waiting for job termination.";
            break;
        }
        sleep(1);
    }
    return terminated_j;
}



drmaa2_string drmaa2_j_get_id(const drmaa2_j j)
{
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
    drmaa2_jstate old_state = get_state(j);
    if (old_state == DRMAA2_RUNNING) {
        save_state(j, DRMAA2_SUSPENDED);
        if (current_drmaa2_callback != NULL)
            call_state_chage_notification(j, DRMAA2_SUSPENDED);
        return DRMAA2_SUCCESS;
    }
    else {
        drmaa2_lasterror_v = DRMAA2_INVALID_STATE;
        drmaa2_lasterror_text_v   = "Current job state does not allow to suspend the job.";
        return DRMAA2_INVALID_STATE;
    }
}


drmaa2_error drmaa2_j_resume(drmaa2_j j)
{
    drmaa2_jstate old_state = get_state(j);
    if (old_state == DRMAA2_SUSPENDED) {
        save_state(j, DRMAA2_RUNNING);
        if (current_drmaa2_callback != NULL)
            call_state_chage_notification(j, DRMAA2_RUNNING);
        return DRMAA2_SUCCESS;
    }
    else {
        drmaa2_lasterror_v = DRMAA2_INVALID_STATE;
        drmaa2_lasterror_text_v   = "Current job state does not allow to resume the job.";
        return DRMAA2_INVALID_STATE;
    }
}


drmaa2_error drmaa2_j_hold(drmaa2_j j)
{
    drmaa2_jstate old_state = get_state(j);
    if (old_state == DRMAA2_QUEUED) {
        save_state(j, DRMAA2_QUEUED_HELD);
        if (current_drmaa2_callback != NULL)
            call_state_chage_notification(j, DRMAA2_QUEUED_HELD);
        return DRMAA2_SUCCESS;
    }
    else if (old_state == DRMAA2_REQUEUED) {
        save_state(j, DRMAA2_REQUEUED_HELD);
        if (current_drmaa2_callback != NULL)
            call_state_chage_notification(j, DRMAA2_REQUEUED);
        return DRMAA2_SUCCESS;
    }
    else {
        drmaa2_lasterror_v = DRMAA2_INVALID_STATE;
        drmaa2_lasterror_text_v   = "Current job state does not allow to hold the job.";
        return DRMAA2_INVALID_STATE;
    }
}


drmaa2_error drmaa2_j_release(drmaa2_j j)
{
    drmaa2_jstate old_state = get_state(j);
    if (old_state == DRMAA2_QUEUED_HELD) {
        save_state(j, DRMAA2_QUEUED);
        if (current_drmaa2_callback != NULL)
            call_state_chage_notification(j, DRMAA2_QUEUED);
        return DRMAA2_SUCCESS;
    }
    else if (old_state == DRMAA2_REQUEUED_HELD) {
        save_state(j, DRMAA2_REQUEUED);
        if (current_drmaa2_callback != NULL)
            call_state_chage_notification(j, DRMAA2_REQUEUED);
        return DRMAA2_SUCCESS;
    }
    else {
        drmaa2_lasterror_v = DRMAA2_INVALID_STATE;
        drmaa2_lasterror_text_v   = "Current job state does not allow to release the job.";
        return DRMAA2_INVALID_STATE;
    }
}


drmaa2_error drmaa2_j_terminate(drmaa2_j j)
{
    pid_t jpid = get_job_pid(j);
    pid_t job_gpid =  getpgid(jpid);
    DRMAA2_DEBUG_PRINT("Kill process group %d.\n", jpid);
    if (killpg(job_gpid, SIGTERM)) {
        DRMAA2_DEBUG_PRINT("ERROR: %s\n", strerror(errno));
    }

    return DRMAA2_SUCCESS;
}


drmaa2_jstate drmaa2_j_get_state(const drmaa2_j j, drmaa2_string * substate)
{
    *substate = NULL;
    return get_state(j);
}



drmaa2_jinfo drmaa2_j_get_info(const drmaa2_j j)
{
    drmaa2_jinfo ji = drmaa2_jinfo_create();
    ji->jobId = strdup(j->id);
    
    ji = get_job_info(ji); // exitStatus, terminating signal and *_time are set
    
    //TODO set: jobSubState, allocatedMachines, i->submissionMachine, jobOwner, queueName 
    return ji;
}


drmaa2_error drmaa2_j_wait_started (const drmaa2_j j, const time_t timeout)
{
    drmaa2_jstate state;
    drmaa2_error return_status = DRMAA2_SUCCESS;
    while (1) {
        state = get_state(j);
        if (state != DRMAA2_QUEUED && state != DRMAA2_QUEUED_HELD) {
            break;
        } else if (timeout == DRMAA2_ZERO_TIME || (timeout != DRMAA2_INFINITE_TIME && timeout <= time(NULL))) {
            drmaa2_lasterror_v = return_status = DRMAA2_TIMEOUT;
            drmaa2_lasterror_text_v = "A timeout occured while waiting for a job start.";
            break;
        }
        sleep(1);
    }
    return return_status;
}


drmaa2_error drmaa2_j_wait_terminated(const drmaa2_j j, const time_t timeout)
{

    DRMAA2_DEBUG_PRINT("wait for job with id: %s\n", j->id);
    drmaa2_jstate state;
    drmaa2_error return_status = DRMAA2_SUCCESS;
    while (1) {
        state = get_state(j);
        if (state == DRMAA2_DONE || state == DRMAA2_FAILED) {
            break;
        } else if (timeout == DRMAA2_ZERO_TIME || (timeout != DRMAA2_INFINITE_TIME && timeout <= time(NULL))) {
            drmaa2_lasterror_v = return_status = DRMAA2_TIMEOUT;
            drmaa2_lasterror_text_v = "A timeout occured while waiting for a job start.";
            break;
        }
        sleep(1);
    }

    return return_status;
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


drmaa2_error drmaa2_register_event_notification(const drmaa2_callback callback)
{
    current_drmaa2_callback = callback;
    return DRMAA2_SUCCESS;
}

