#include "drmaa2-specific.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "drmaa2-debug.h"


extern int drmaa2_lasterror_v;
extern char *drmaa2_lasterror_text_v;

extern drmaa2_callback current_drmaa2_callback;


// DRMAA2 reflective interface 

drmaa2_string_list drmaa2_jtemplate_impl_spec(void) {
    return drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
}

drmaa2_string_list drmaa2_jinfo_impl_spec(void) {
    return drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
}

drmaa2_string_list drmaa2_rtemplate_impl_spec(void) {
    return drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
}

drmaa2_string_list drmaa2_rinfo_impl_spec(void) {
    return drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
}

drmaa2_string_list drmaa2_queueinfo_impl_spec(void) {
    return drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
}

drmaa2_string_list drmaa2_machineinfo_impl_spec(void) {
    return drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
}

drmaa2_string_list drmaa2_notification_impl_spec(void) {
    return drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
}


drmaa2_string drmaa2_get_instance_value(const void * instance, const char * name) {
    drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
    drmaa2_lasterror_text_v = "drmaa2-mock has no implementation-specific attributes";
    return NULL;
}

drmaa2_string drmaa2_describe_attribute(const void * instance, const char * name) {
    drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
    drmaa2_lasterror_text_v = "drmaa2-mock has no implementation-specific attributes";
    return NULL;
}

drmaa2_error drmaa2_set_instance_value(void * instance, const char * name, const char * value) {
    drmaa2_lasterror_v = DRMAA2_INVALID_ARGUMENT;
    drmaa2_lasterror_text_v = "drmaa2-mock has no implementation-specific attributes";
    return DRMAA2_INVALID_ARGUMENT;
}



// free functions of implementation-specific drmaa inface types

void drmaa2_jsession_free(drmaa2_jsession * jsRef) {
    if (*jsRef != NULL) {
        drmaa2_string_free(&((*jsRef)->contact));
        drmaa2_string_free(&((*jsRef)->name));
        free(*jsRef);
        *jsRef = NULL;
    }
}

void drmaa2_rsession_free(drmaa2_rsession * rsRef) {
    if (*rsRef != NULL) {
        drmaa2_string_free(&((*rsRef)->contact));
        drmaa2_string_free(&((*rsRef)->name));
        free(*rsRef);
        *rsRef = NULL;
    }
}

void drmaa2_msession_free(drmaa2_msession * msRef) {
    if (*msRef != NULL) {
        drmaa2_string_free(&((*msRef)->name));
        free(*msRef);
        *msRef = NULL;
    }
}

void drmaa2_j_free(drmaa2_j * jRef) {
    if (*jRef != NULL) {
        drmaa2_string_free(&((*jRef)->id));
        drmaa2_string_free(&((*jRef)->session_name));
        free(*jRef);
        *jRef=NULL;
    }
}

void drmaa2_jarray_free(drmaa2_jarray * jaRef) {
    if (*jaRef != NULL) {
        drmaa2_string_free(&((*jaRef)->id));
        drmaa2_string_free(&((*jaRef)->session_name));
        free(*jaRef);
        *jaRef=NULL;
    }
}

void drmaa2_r_free(drmaa2_r * rRef) {
    if (*rRef != NULL) {
        drmaa2_string_free(&((*rRef)->id));
        drmaa2_string_free(&((*rRef)->session_name));
        free(*rRef);
        *rRef=NULL;
    }
}








drmaa2_string_list add_supported_job_categories(drmaa2_string_list jl) {
#ifdef __GNUC__
    if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 2) {
        drmaa2_list_add(jl, strdup("OpenMP")); // GCC 4.2 implements OpenMP 2.5
    }
#endif
    return jl;
}


void start_and_monitor_job(drmaa2_j j, drmaa2_jtemplate jt) {
    pid_t job_pid;

    if ((job_pid = fork()) == -1) {
        perror("fork failed\n");
        exit(1);
    }

    long long row_id = atoll(j->id);
    
    if (job_pid == 0) {
        //child - represents the running job
        setsid();   // create new process group so that jobs containing multiple processes can be killed
        save_state_id(row_id, DRMAA2_RUNNING);
        if (current_drmaa2_callback != NULL)
            call_state_chage_notification(j, DRMAA2_RUNNING);
        char *args[] = {jt->remoteCommand, NULL};
        execv(args[0], args);
        return;           // dead code, just to avoid GCC warning
    }
    else {
        // parent - monitors the running job
        drmaa2_save_pid(row_id, job_pid);

        pid_t child;
        int status;
        child = waitpid(job_pid, &status, 0);
        drmaa2_save_exit_status(row_id, WEXITSTATUS(status));

        if (WIFEXITED(status)) {
            save_state_id(row_id, DRMAA2_DONE);
            if (current_drmaa2_callback != NULL)
                call_state_chage_notification(j, DRMAA2_DONE);
            DRMAA2_DEBUG_PRINT("Process terminated normally by a call to _exit(2) or exit(3).\n");
            DRMAA2_DEBUG_PRINT("%d  - evaluates to the low-order 8 bits of the argument passed to _exit(2) or exit(3) by the child.\n", WEXITSTATUS(status));
        }
        if (WIFSIGNALED(status)) {
            save_state_id(row_id, DRMAA2_FAILED);
            if (current_drmaa2_callback != NULL)
                call_state_chage_notification(j, DRMAA2_FAILED);
            DRMAA2_DEBUG_PRINT("Process terminated due to receipt of a signal.\n");
            DRMAA2_DEBUG_PRINT("%d  - evaluates to the number of the signal that caused the termination of the process.\n", WTERMSIG(status));
            DRMAA2_DEBUG_PRINT("%d  - evaluates as true if the termination of the process was accompanied by the creation of a core \
                 file containing an image of the process when the signal was received.\n", WCOREDUMP(status));
        }
        if (WIFSTOPPED(status)) {
            DRMAA2_DEBUG_PRINT("Process has not terminated, but has stopped and can be restarted.  This macro can be true only if the wait call \
                 specified the WUNTRACED option or if the child process is being traced (see ptrace(2)).\n");
            DRMAA2_DEBUG_PRINT("%d  - evaluates to the number of the signal that caused the process to stop.\n", WSTOPSIG(status));
        }
        exit(0);
    }

}


drmaa2_j submit_job_to_DRMS(drmaa2_jsession js, long long job_id, drmaa2_jtemplate jt) {
    pid_t childpid;

    if ((childpid = fork()) == -1) {
        perror("fork failed\n");
        exit(1);
    }
    else {
        char *job_id_c;
        asprintf(&job_id_c, "%lld", job_id);
        drmaa2_j j = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
        j->id = job_id_c; //already allocated
        j->session_name = strdup(js->name);
        save_state_id(job_id, DRMAA2_QUEUED);

        
        if (childpid == 0) {
            // child
            start_and_monitor_job(j, jt); 
            //char *args[] = {"./wrapper", job_id_c, NULL};      //old version - will be removed with later commit
            //execv(args[0], args);                              //old version - will be removed with later commit
            return NULL;        // dead code, just to avoid a GCC warning about control end reach
        }
        else {
            // parent
            sleep(1); //TODO: ensure that job is started
            return j;
        }
    }
}



drmaa2_string drmaa2_get_drms_name(void) {
    return NULL;
}

drmaa2_version drmaa2_get_drms_version(void) {
    return NULL;
}

drmaa2_string drmaa2_get_drmaa_name(void) {
    return strdup("drmaa2-mock");
}

drmaa2_version drmaa2_get_drmaa_version(void) {
    drmaa2_version version = (drmaa2_version)malloc(sizeof(drmaa2_version_s));
    version->major = strdup("0");
    version->minor = strdup("1");
    return version;
}


drmaa2_bool drmaa2_supports(const drmaa2_capability c) {
    switch(c) {
        case DRMAA2_ADVANCE_RESERVATION:    return DRMAA2_TRUE;
        case DRMAA2_RESERVE_SLOTS:          return DRMAA2_FALSE;
        case DRMAA2_CALLBACK:               return DRMAA2_TRUE;
        case DRMAA2_BULK_JOBS_MAXPARALLEL:  return DRMAA2_FALSE;
        case DRMAA2_JT_EMAIL:               return DRMAA2_FALSE;
        case DRMAA2_JT_STAGING:             return DRMAA2_TRUE;
        case DRMAA2_JT_DEADLINE:            return DRMAA2_TRUE;
        case DRMAA2_JT_MAXSLOTS:            return DRMAA2_FALSE;
        case DRMAA2_JT_ACCOUNTINGID:        return DRMAA2_FALSE;
        case DRMAA2_RT_STARTNOW:            return DRMAA2_TRUE;
        case DRMAA2_RT_DURATION:            return DRMAA2_TRUE;
        case DRMAA2_RT_MACHINEOS:           return DRMAA2_FALSE;
        case DRMAA2_RT_MACHINEARCH:         return DRMAA2_FALSE;
        default:                            return DRMAA2_FALSE; 
    }
}


