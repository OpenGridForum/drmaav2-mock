/* DRMAAv2 C Binding
 * Authors: Andre Merzky, Mariusz Mamonski, Daniel Gruber, Peter Tröger, Roger Brobst
 *
 * Open issues are marked with FIXME
 *
 * Design decisions:
 *  - IDL module scope is realized by "DRMAA2" prefixing of names / identifiers
 *  - If the input type gives all information, the parameter and method name are shortened
 *  - IDL-spec data types get an "_t" postfix to their name
 *  - IDL-spec struct types get an "_s" postfix to their name
 *  - DRMAA2 (interface) instance types get an "_h" postfix to express their opaque handle character
 *  - Methods do not have a postfix
 *  - Functions directly return valid values, or an UNSET value to indicate a problem
 *  - Procedures always return drmaa2_error_t
 *  - Two new additional function for fetching error number and error message of the last call 
 *    (thread-local storage to be mandadated in written spec textxs)
 *  - All functions returning VOID in IDL are mapped to boolean return type 
 *  - Generic support functions for list data types
 *      - keeps list operations portable between different DRMAA implementations
 *      - Original DRMAA error codes are good enough to support them, no new ones needed
 *      - All lists created must be free'd by the application, even if they come from the implementation itself
 *  - IDL Boolean maps to int, plus drmaa2_boolean_t for TRUE / FALSE value definition
 *  - IDL long maps to long long (despite exitStatus, which is naturally an INT)
 *  - All numerical types are signed, in order to support -1 as UNSET-value for numerical types
 *  - For better readability, structs and enums are typedef'ed.
 *  - Application-created structs get additional support methods to support the demanded UNSET initialization.
 *  - AbsoluteTime and TimeAmount directly map to time_t. The "%z" formatter gives the RFC822 stringification support demanded by the IDL spec.
 *  - "any" type for job sub-state is mapped to char* as generic portable approach.
 *  - FIXME: Exceptions are modeled as enum, since extensibility is not allowed (see public comment for root spec).
 *  - DrmaaReflective lists are mapped to global functions, and not to const's, since multiple implementation backends might intended
 *  - One UNSET macro per type, to make the UNSET semantics explicit; favoured macro approach to reason uppercase naming (instead of const) 
 *  - Example for implementation-specific struct extension approach being recommended by the language binding:
 *        typedef struct gridengine_job_template_s 
 *        {
 *            [attributes from drmaa2_job_template_s] ...
 *            int gridengine_specific_attr;
 *        } gridengine_job_template_s;
 *        typedef gridengine_job_template_s * gridengine_job_template_t; (allows pointer casting in application and implementation)
 *
 * - FIXME: The IDL spec demands support for getting a printable version of a job template. 
 * - If a method parameter is not changed during execution, it is marked as const.
 */ 

#include <time.h>

/************************** Enumerations ***********************/
typedef enum drmaa2_job_state_t
{
  DRMAA2_UNDETERMINED                =  0,
  DRMAA2_QUEUED                      =  1,
  DRMAA2_QUEUED_HELD                 =  2,
  DRMAA2_RUNNING                     =  3,
  DRMAA2_SUSPENDED                   =  4,
  DRMAA2_REQUEUED                    =  5,
  DRMAA2_REQUEUED_HELD               =  6,
  DRMAA2_DONE                        =  7,
  DRMAA2_FAILED                      =  8
} drmaa2_job_state_t;

typedef enum drmaa2_operating_system_t
{
  DRMAA2_OTHER_OS                    =  0,
  DRMAA2_AIX                         =  1,
  DRMAA2_BSD                         =  2,
  DRMAA2_LINUX                       =  3,
  DRMAA2_HPUX                        =  4,
  DRMAA2_IRIX                        =  5,
  DRMAA2_MACOS                       =  6,
  DRMAA2_SUNOS                       =  7,
  DRMAA2_TRUE64                      =  8,
  DRMAA2_UNIXWARE                    =  9,
  DRMAA2_WIN                         = 10,
  DRMAA2_WINNT                       = 11
} drmaa2_operating_system_t;

typedef enum drmaa2_cpu_architecture_t
{
  DRMAA2_OTHER_CPU                   =  0,
  DRMAA2_ALPHA                       =  1,
  DRMAA2_ARM                         =  2,
  DRMAA2_CELL                        =  3,
  DRMAA2_PARISC                      =  4,
  DRMAA2_X86                         =  5,
  DRMAA2_X64                         =  6,
  DRMAA2_IA64                        =  7,
  DRMAA2_MIPS                        =  8,
  DRMAA2_PPC                         =  9,
  DRMAA2_PPC64                       = 10,
  DRMAA2_SPARC                       = 11,
  DRMAA2_SPARC64                     = 12
} drmaa2_cpu_architecture_t;

typedef enum drmaa2_resource_limit_type_t
{
  DRMAA2_CORE_FILE_SIZE              =  0,
  DRMAA2_CPU_TIME                    =  1,
  DRMAA2_DATA_SEG_SIZE               =  2,
  DRMAA2_FILE_SIZE                   =  3,
  DRMAA2_OPEN_FILES                  =  4,
  DRMAA2_STACK_SIZE                  =  5,
  DRMAA2_VIRTUAL_MEMORY              =  6,
  DRMAA2_WALLCLOCK_TIME              =  7
} drmaa2_resource_limit_type_t;

typedef enum drmaa2_jt_placeholder_t
{
  DRMAA2_HOME_DIRECTORY              =  0,
  DRMAA2_WORKING_DIRECTORY           =  1,
  DRMAA2_PARAMETRIC_INDEX            =  2
} drmaa2_jt_placeholder_t;

typedef enum drmaa2_event_t
{
  DRMAA2_NEW_STATE                   =  0,
  DRMAA2_MIGRATED                    =  1,
  DRMAA2_ATTRIBUTE_CHANGE            =  2
} drmaa2_event_t;

typedef enum drmaa2_capability_t
{
  DRMAA2_ADVANCE_RESERVATION         =  0,
  DRMAA2_RESERVE_SLOTS               =  1,
  DRMAA2_CALLBACK                    =  2,
  DRMAA2_BULK_JOBS_MAXPARALLEL       =  3,
  DRMAA2_JT_EMAIL                    =  4,
  DRMAA2_JT_STAGING                  =  5,
  DRMAA2_JT_DEADLINE                 =  6,
  DRMAA2_JT_MAXSLOTS                 =  7,
  DRMAA2_JT_ACCOUNTINGID             =  8,
  DRMAA2_RT_STARTNOW                 =  9,
  DRMAA2_RT_DURATION                 = 10,
  DRMAA2_RT_MACHINEOS                = 11,
  DRMAA2_RT_MACHINEARCH              = 12
} drmaa2_capability_t;

typedef enum drmaa2_boolean_t
{
  DRMAA2_FALSE                       =  0,
  DRMAA2_TRUE                        =  1
} drmaa2_boolean_t;

/************************** Exceptions ***********************/


typedef enum drmaa2_error_t {
  DRMAA2_SUCCESS                     =  0,
  DRMAA2_ERROR_DENIED_BY_DRMS        =  1,
  DRMAA2_ERROR_DRM_COMMUNICATION     =  2,
  DRMAA2_ERROR_TRY_LATER             =  3,
  DRMAA2_ERROR_SESSION_MANAGEMENT    =  4,
  DRMAA2_ERROR_TIMEOUT               =  5,
  DRMAA2_ERROR_INTERNAL              =  6,
  DRMAA2_ERROR_INVALID_ARGUMENT      =  7,
  DRMAA2_ERROR_INVALID_SESSION       =  8,
  DRMAA2_ERROR_INVALID_STATE         =  9,
  DRMAA2_ERROR_OUT_OF_RESOURCE       = 10,
  DRMAA2_ERROR_UNSUPPORTED_ATTRIBUTE = 11,
  DRMAA2_ERROR_UNSUPPORTED_OPERATION = 12,
  DRMAA2_ERROR_LASTNUM               = 13
} drmaa2_error_t;

// returns the last error in a thread-safe manner
drmaa2_error_t drmaa2_lasterror(void);

// returns the textual description of the last error in a thread-safe manner
// the application is obligated to free the string buffer (may be dynamic)
char * drmaa2_lasterror_text(void);


/************************** List type definitions ***********************/

typedef void *        drmaa2_list_t;
typedef drmaa2_list_t drmaa2_string_list_t;
typedef drmaa2_list_t drmaa2_job_list_t;
typedef drmaa2_list_t drmaa2_queueinfo_list_t;
typedef drmaa2_list_t drmaa2_machineinfo_list_t;
typedef drmaa2_list_t drmaa2_slotinfo_list_t;
typedef drmaa2_list_t drmaa2_reservation_list_t;

typedef enum drmaa2_listtype
{
  DRMAA2_STRING_LIST,        // both OrderedStringList and StringList
  DRMAA2_JOB_LIST,
  DRMAA2_QUEUEINFO_LIST,
  DRMAA2_MACHINEINFO_LIST,
  DRMAA2_SLOTINFO_LIST,
  DRMAA2_RESERVATION_LIST
} drmaa2_listtype_t;


// Creates a new list, for the specified type of items
// Returns pointer to list, or NULL
drmaa2_list_t drmaa2_list_create  (const drmaa2_listtype_t t);

// Deletes the list, also deletes the members
// Returns success indication 
drmaa2_error_t drmaa2_list_free   (drmaa2_list_t l); 

// FIXME: clarify if iterator state is thread local or not

// Returns the next/previous item in the list, 
// or NULL if the end/begin is reached
void *         drmaa2_list_next   (drmaa2_list_t l);
void *         drmaa2_list_prev   (drmaa2_list_t l);

// Resets the iterator of the list to the first/last item
// Returns success indication 
drmaa2_error_t drmaa2_list_first  (drmaa2_list_t l);
drmaa2_error_t drmaa2_list_last   (drmaa2_list_t l);

// Adds a new item at the end of the list
// Returns success indication 
drmaa2_error_t drmaa2_list_add    (drmaa2_list_t l, void * value);

// Removes the most recent item returned by the iterator, 
// or the first if the iterator wasn't used so far
// Returns success indication 
// FIXME: what happens after 'add()' - is the iterator 
// moved to the added element?
drmaa2_error_t drmaa2_list_remove (drmaa2_list_t l);

// FIXME: it would still be useful to get a size, even 
// if it is not used for indexing.  Otherwise counting
// is very cumbersome...
// returns '0' on empty list (not an error)
int            drmaa2_list_size   (const drmaa2_list_t l);


/************************** Dictionary type definitions ***********************/

typedef struct drmaa2_dictentry_t
{
  char * key;
  char * value;
} drmaa2_dictentry_t;

typedef void * drmaa2_dict_t;

// Creates a new dictionary
// Returns pointer to dict, or NULL
drmaa2_dict_t        drmaa2_dict_create (void);

// Deletes the dictionary, also deletes the members
// Returns success indication 
drmaa2_error_t       drmaa2_dict_free   (drmaa2_dict_t d); 

drmaa2_string_list_t drmaa2_dict_list   (const drmaa2_dict_t d);                         // return list of set keys
drmaa2_boolean_t     drmaa2_dict_has    (const drmaa2_dict_t d, const char * key);
char *               drmaa2_dict_get    (const drmaa2_dict_t d, const char * key);             // fails if !has(key)
drmaa2_error_t       drmaa2_dict_del    (      drmaa2_dict_t d, const char * key);             // fails if !has(key)
drmaa2_error_t       drmaa2_dict_set    (      drmaa2_dict_t d, const char * key, const char * val); // add   if !has(key)

/************************** Time Defs   ***********************/

#define  DRMAA2_ZERO_TIME       ((time_t)  0)
#define  DRMAA2_INFINITE_TIME   ((time_t) -1)
#define  DRMAA2_NOW             ((time_t) -2)


/************************** UNSET values  ***********************/

// FIXME: Ignores the recommendation of the IDL spec
#define  DRMAA2_UNSET_BOOLEAN   DRMAA2_FALSE
#define  DRMAA2_UNSET_STRING    NULL   // FIXME: Ignores the recommendation of the IDL spec
#define  DRMAA2_UNSET_NUM       -1     // FIXME: Ignores the recommendation of the IDL spec
#define  DRMAA2_UNSET_ENUM      -1
#define  DRMAA2_UNSET_LIST      NULL
#define  DRMAA2_UNSET_DICT      NULL
#define  DRMAA2_UNSET_TIME      ((time_t) -3)
#define  DRMAA2_UNSET_CALLBACK  NULL	// for un-registering notification callback functions


/************************** Value Types ***********************/

// FIXME: IDL spec mandates pass-by-value so far, which we ignore with the pointer representation
// of structs in the signatures

typedef struct drmaa2_job_info_s
{
  char *                      jobId;
  int                         exitStatus;        
  char *                      terminatingSignal;
  char *                      annotation;
  drmaa2_job_state_t          jobState;
  char *                      jobSubState;
  drmaa2_string_list_t        allocatedMachines;
  char *                      submissionMachine;
  char *                      jobOwner;
  long long                   slots;
  char *                      queueName;
  time_t                      wallclockTime;
  long long                   cpuTime;
  time_t                      submissionTime;
  time_t                      dispatchTime;
  time_t                      finishTime;
} drmaa2_job_info_s;
typedef drmaa2_job_info_s * drmaa2_job_info_t;

drmaa2_job_info_t drmaa2_job_info_create (void);
drmaa2_error_t    drmaa2_job_info_free   (drmaa2_job_info_t ji);

typedef struct drmaa2_slot_info_s
{
  char *                      machineName; 
  long long                   slots;
} drmaa2_slot_info_s;
typedef drmaa2_slot_info_s * drmaa2_slot_info_t;
// Structure is only result, so no create / free functions are needed for the application

typedef struct drmaa2_reservation_info_s
{
  char *                      reservationId;
  char *                      reservationName;
  time_t                      reservedStartTime;
  time_t                      reservedEndTime;
  drmaa2_string_list_t        usersACL;
  long long                   reservedSlots;
  drmaa2_slotinfo_list_t      reservedMachines;
} drmaa2_reservation_info_s;
typedef drmaa2_reservation_info_s * drmaa2_reservation_info_t;
// Structure is only result, so no create / free functions are needed for the application

typedef struct drmaa2_job_template_s
{
  char *                      remoteCommand;          
  drmaa2_string_list_t        args;          
  drmaa2_boolean_t            submitAsHold;
  drmaa2_boolean_t            rerunnable;
  drmaa2_dict_t               jobEnvironment;        
  char *                      workingDirectory;        
  char *                      jobCategory;            
  drmaa2_string_list_t        email;            
  drmaa2_boolean_t            emailOnStarted;
  drmaa2_boolean_t            emailOnTerminated;
  char *                      jobName;              
  char *                      inputPath;            
  char *                      outputPath;            
  char *                      errorPath;            
  drmaa2_boolean_t            joinFiles;
  char *                      reservationId;          
  char *                      queueName;            
  long long                   minSlots;              
  long long                   maxSlots;              
  long long                   priority;              
  drmaa2_string_list_t        candidateMachines;  
  long long                   minPhysMemory;            
  drmaa2_operating_system_t   machineOS;        
  drmaa2_cpu_architecture_t   machineArch;      
  time_t                      startTime;          
  time_t                      deadlineTime;        
  drmaa2_dict_t               stageInFiles;        
  drmaa2_dict_t               stageOutFiles;        
  drmaa2_dict_t               resourceLimits;      
  char *                      accountingId;          
} drmaa2_job_template_s;
typedef drmaa2_job_template_s * drmaa2_job_template_t;

drmaa2_job_template_t  drmaa2_job_template_create (void);
drmaa2_error_t         drmaa2_job_template_free   (drmaa2_job_template_t jt);

typedef struct drmaa2_reservation_template_s
{
  char *                       reservationName;          
  time_t                       startTime;          
  time_t                       endTime;          
  time_t                       duration;          
  long long                    minSlots;              
  long long                    maxSlots;
  char *                       jobCategory;
  drmaa2_string_list_t         usersACL;            
  drmaa2_string_list_t         candidateMachines;  
  long long                    minPhysMemory;            
  drmaa2_operating_system_t    machineOS;        
  drmaa2_cpu_architecture_t    machineArch;      
} drmaa2_reservation_template_s;
typedef drmaa2_reservation_template_s * drmaa2_reservation_template_t;

drmaa2_reservation_template_t  drmaa2_reservation_template_create (void);
drmaa2_error_t                 drmaa2_reservation_template_free   (drmaa2_reservation_template_t rt);

// Structure is only result, so no create / free functions are needed for the application
typedef struct drmaa2_notification_s
{
  drmaa2_event_t               event;
  char *                       jobId;
  char *                       sessionName;
  drmaa2_job_state_t           jobState;
} drmaa2_notification_s;
typedef drmaa2_notification_s * drmaa2_notification_t;

// Structure is only result, so no create / free functions are needed for the application
typedef struct drmaa2_queue_info_s
{
  char *                       name;
} drmaa2_queue_info_s;
typedef drmaa2_queue_info_s * drmaa2_queue_info_t;

// Structure is only result, so no create / free functions are needed for the application
typedef struct drmaa2_version_s
{
  char *                       major; 
  char *                       minor;
} drmaa2_version_s;
typedef drmaa2_version_s * drmaa2_version_t;

// Structure is only result, so no create / free functions are needed for the application
typedef struct drmaa2_machine_info_s
{
  char *                       name;  
  drmaa2_boolean_t             available;    
  long long                    sockets;      
  long long                    coresPerSocket;
  long long                    threadsPerCore;  
  float                        load;  
  long long                    physMemory;
  long long                    virtMemory;    
  drmaa2_operating_system_t    machineOS;  
  drmaa2_version_t             machineOSVersion;
  drmaa2_cpu_architecture_t    machineArch;
} drmaa2_machine_info_s;
typedef drmaa2_machine_info_s * drmaa2_machine_info_t;


/************************** Interfaces ***********************/

// DrmaaReflective
drmaa2_string_list_t drmaa2_job_template_impl_spec         (void);
drmaa2_string_list_t drmaa2_job_info_impl_spec             (void);
drmaa2_string_list_t drmaa2_reservation_template_impl_spec (void);
drmaa2_string_list_t drmaa2_reservation_info_impl_spec     (void);
drmaa2_string_list_t drmaa2_queue_info_impl_spec           (void);
drmaa2_string_list_t drmaa2_machine_info_impl_spec         (void);
drmaa2_string_list_t drmaa2_notification_impl_spec         (void);

char *               drmaa2_get_instance_value (const void * instance, const char * name);
char *               drmaa2_describe_attribute (const void * instance, const char * name);
drmaa2_error_t       drmaa2_set_instance_value (      void * instance, const char * name, const char * value);

// DrmaaCallback
typedef void (*drmaa2_callback_t)(drmaa2_notification_t * notification);

// handle's for interface instances
typedef void * drmaa2_jobsession_h;
typedef void * drmaa2_job_h;
typedef void * drmaa2_reservationsession_h;
typedef void * drmaa2_reservation_h;
typedef void * drmaa2_jobarray_h;
typedef void * drmaa2_monitoringsession_h;


// ReservationSession
char *                         drmaa2_rs_get_contact              (const drmaa2_reservationsession_h rs);
char *                         drmaa2_rs_get_session_name         (const drmaa2_reservationsession_h rs); 
drmaa2_reservation_h           drmaa2_rs_get_reservation          (const drmaa2_reservationsession_h rs, const char * reservation_id);
drmaa2_reservation_h           drmaa2_rs_request_reservation      (const drmaa2_reservationsession_h rs, const drmaa2_reservation_template_t rt);
drmaa2_reservation_list_t      drmaa2_rs_get_reservations         (const drmaa2_reservationsession_h rs);

// Reservation
char *                         drmaa2_r_get_id                    (const drmaa2_reservation_h r);
char *                         drmaa2_r_get_session_name          (const drmaa2_reservation_h r);
drmaa2_reservation_template_t  drmaa2_r_get_reservation_template  (const drmaa2_reservation_h r);
drmaa2_reservation_info_t      drmaa2_r_get_info                  (const drmaa2_reservation_h r);
drmaa2_error_t                 drmaa2_r_terminate                 (drmaa2_reservation_h r);

// JobArray 
char *                         drmaa2_ja_get_id                   (const drmaa2_jobarray_h ja);
drmaa2_job_list_t              drmaa2_ja_get_jobs                 (const drmaa2_jobarray_h ja); 
char *                         drmaa2_ja_get_session_name         (const drmaa2_jobarray_h ja);
drmaa2_job_template_t          drmaa2_ja_get_job_template         (const drmaa2_jobarray_h ja); 
drmaa2_error_t                 drmaa2_ja_suspend                  (drmaa2_jobarray_h ja); 
drmaa2_error_t                 drmaa2_ja_resume                   (drmaa2_jobarray_h ja); 
drmaa2_error_t                 drmaa2_ja_hold                     (drmaa2_jobarray_h ja); 
drmaa2_error_t                 drmaa2_ja_release                  (drmaa2_jobarray_h ja); 
drmaa2_error_t                 drmaa2_ja_terminate                (drmaa2_jobarray_h ja); 

// JobSession 
char *                         drmaa2_js_get_contact              (const drmaa2_jobsession_h js); 
char *                         drmaa2_js_get_session_name         (const drmaa2_jobsession_h js);
drmaa2_string_list_t           drmaa2_js_get_job_categories       (const drmaa2_jobsession_h js); 
drmaa2_job_list_t              drmaa2_js_get_jobs                 (const drmaa2_jobsession_h js, const drmaa2_job_info_t filter);
drmaa2_jobarray_h              drmaa2_js_get_job_array            (const drmaa2_jobsession_h js, const char * jobarray_id);
drmaa2_job_h                   drmaa2_js_run_job                  (const drmaa2_jobsession_h js, const drmaa2_job_template_t jt);
// FIXME: IDL spec demands the language binding to specify the largest allowed end_index.
drmaa2_jobarray_h              drmaa2_js_run_bulk_jobs            (const drmaa2_jobsession_h js, const drmaa2_job_template_t jt, 
                                                                   unsigned long begin_index, unsigned long end_index,
                                                                   unsigned long step,        unsigned long max_parallel);
drmaa2_job_h                   drmaa2_js_wait_any_started         (const drmaa2_jobsession_h js, const drmaa2_job_list_t l, const time_t timeout);
drmaa2_job_h                   drmaa2_js_wait_any_terminated      (const drmaa2_jobsession_h js, const drmaa2_job_list_t l, const time_t timeout);

// Job
char *                         drmaa2_j_get_id                     (const drmaa2_job_h j);
char *                         drmaa2_j_get_session_name           (const drmaa2_job_h j);
drmaa2_job_template_t          drmaa2_j_get_job_template           (const drmaa2_job_h j);
drmaa2_error_t                 drmaa2_j_suspend                    (drmaa2_job_h j); 
drmaa2_error_t                 drmaa2_j_resume                     (drmaa2_job_h j); 
drmaa2_error_t                 drmaa2_j_hold                       (drmaa2_job_h j); 
drmaa2_error_t                 drmaa2_j_release                    (drmaa2_job_h j); 
drmaa2_error_t                 drmaa2_j_terminate                  (drmaa2_job_h j); 
drmaa2_job_state_t             drmaa2_j_get_state                  (const drmaa2_job_h j, char ** substate); // FIXME: how long is substate list?
drmaa2_job_info_t              drmaa2_j_get_info                   (const drmaa2_job_h j);
drmaa2_job_h                   drmaa2_j_wait_started               (const drmaa2_job_h j, const time_t timeout); 
drmaa2_job_h                   drmaa2_j_wait_terminated            (const drmaa2_job_h j, const time_t timeout); 

// MonitoringSession
drmaa2_reservation_list_t      drmaa2_ms_get_all_reservations      (const drmaa2_monitoringsession_h ms);
drmaa2_job_list_t              drmaa2_ms_get_all_jobs              (const drmaa2_monitoringsession_h ms, const drmaa2_job_info_t filter);
drmaa2_queueinfo_list_t        drmaa2_ms_get_all_queues            (const drmaa2_monitoringsession_h ms, const drmaa2_string_list_t names);
drmaa2_machineinfo_list_t      drmaa2_ms_get_all_machines          (const drmaa2_monitoringsession_h ms, const drmaa2_string_list_t names);


// SessionManager
char *                         drmaa2_get_drms_name                (void);
drmaa2_version_t               drmaa2_get_drms_version             (void);
char *                         drmaa2_get_drmaa_name               (void);
drmaa2_version_t               drmaa2_get_drmaa_version            (void);
drmaa2_boolean_t               drmaa2_supports                     (const drmaa2_capability_t c);
drmaa2_jobsession_h            drmaa2_create_jobsession            (const char * session_name, const char * contact);
drmaa2_reservationsession_h    drmaa2_create_reservationsession    (const char * session_name, const char * contact);
drmaa2_jobsession_h            drmaa2_open_jobsession              (const char * session_name);
drmaa2_reservationsession_h    drmaa2_open_reservationsession      (const char * session_name);
drmaa2_monitoringsession_h     drmaa2_open_monitoringsession       (const char * session_name);
drmaa2_error_t                 drmaa2_close_jobsession             (drmaa2_jobsession_h js);
drmaa2_error_t                 drmaa2_close_reservationsession     (drmaa2_reservationsession_h rs);
drmaa2_error_t                 drmaa2_close_monitoringsession      (drmaa2_monitoringsession_h ms);
drmaa2_error_t                 drmaa2_destroy_jobsession           (const char * session_name);
drmaa2_error_t                 drmaa2_destroy_reservationsession   (const char * session_name);
drmaa2_string_list_t           drmaa2_get_jobsession_names         (void);
drmaa2_string_list_t           drmaa2_get_reservationsession_names (void);
drmaa2_error_t                 drmaa2_register_event_notification  (const drmaa2_callback_t callback);

