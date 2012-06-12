#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "drmaa2-debug.h"


void usage()
{
	printf("Usage: ./wrapper DATABASE JOB_ID\n");
}



static int cmd_callback(char **ptr, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    char *command = strdup(argv[0]);
    *ptr = command;
    return 0;
}

char *get_command(char *db_name, long long row_id)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(db_name, &db);
    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        usage();
        exit(1);
    }


    char *stmt = sqlite3_mprintf("SELECT remoteCommand FROM jobs, job_templates \
    	WHERE jobs.rowid = %lld AND job_templates.rowid = jobs.template_id", row_id);
    DEBUG_PRINT("%s\n", stmt);

    char *command = NULL;
    rc = -1;
    while (rc != SQLITE_OK)
    {
        rc = sqlite3_exec(db, stmt, cmd_callback, &command, &zErrMsg);
        if( rc!=SQLITE_OK )
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            sleep(1);
        }
    }
    sqlite3_free(stmt);

    sqlite3_close(db);
    return command;
}


int drmaa2_save_pid(char *db_name, long long row_id, pid_t pid)
{
	sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(db_name, &db);
    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        usage();
        exit(1);
    }


    char *stmt = sqlite3_mprintf("BEGIN IMMEDIATE; UPDATE jobs SET pid = %d WHERE rowid = %lld; END;", pid, row_id);
    DEBUG_PRINT("%s\n", stmt);

    rc = -1;
    while (rc != SQLITE_OK)
    {
        rc = sqlite3_exec(db, stmt, NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK )
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            sleep(1);
        }
    }

    sqlite3_free(stmt);
    sqlite3_close(db);
    return rc;
}


int drmaa2_save_exit_status(char *db_name, long long row_id, int status)
{
	sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(db_name, &db);
    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        usage();
        exit(1);
    }

    char *stmt = sqlite3_mprintf("BEGIN IMMEDIATE; UPDATE jobs SET exit_status = %d WHERE rowid = %lld; END;", status, row_id);
    DEBUG_PRINT("%s\n", stmt);

    while (1)
    {
    	rc = sqlite3_exec(db, stmt, NULL, NULL, &zErrMsg);
    	if (rc == SQLITE_OK)
    		break;
    	sleep(1);
    }
    	
    sqlite3_free(stmt);

    if( rc!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
    return rc;
}



int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		usage();
		exit(1);
	}

	long long row_id = atoll(argv[2]);
	if (row_id < 1)
	{
		fprintf(stderr, "No valid row_id: %s\n", argv[2]);
		usage();
		exit(1);
	}

	char *cmd = get_command(argv[1], row_id);
	pid_t childpid;

	if ((childpid = fork()) == -1)
    {
        perror("fork failed\n");
        exit(1);
    }
    
    if (childpid == 0)
    {
    	//child
    	char *args[] = {cmd, NULL};
    	execv(args[0], args);

    }
    else
    {
    	// parent
    	drmaa2_save_pid(argv[1], row_id, childpid);

    	pid_t child;
    	int status;
		child = waitpid(childpid, &status, 0);

		drmaa2_save_exit_status(argv[1], row_id, status);

		if (WIFEXITED(status))
	    {
	        //j->info->exitStatus = WEXITSTATUS(status);
	        //j->info->finishTime = time(NULL);

	        DRMAA2_DEBUG_PRINT("Process terminated normally by a call to _exit(2) or exit(3).\n");
	        DRMAA2_DEBUG_PRINT("%d  - evaluates to the low-order 8 bits of the argument passed to _exit(2) or exit(3) by the child.\n", WEXITSTATUS(status));
	    }
	    if (WIFSIGNALED(status))
	    {
	        DRMAA2_DEBUG_PRINT("Process terminated due to receipt of a signal.\n");
	        DRMAA2_DEBUG_PRINT("%d  - evaluates to the number of the signal that caused the termination of the process.\n", WTERMSIG(status));
	        DRMAA2_DEBUG_PRINT("%d  - evaluates as true if the termination of the process was accompanied by the creation of a core \
	             file containing an image of the process when the signal was received.\n", WCOREDUMP(status));
	    }
	    if (WIFSTOPPED(status))
	    {
	        DRMAA2_DEBUG_PRINT("Process has not terminated, but has stopped and can be restarted.  This macro can be true only if the wait call \
	             specified the WUNTRACED option or if the child process is being traced (see ptrace(2)).\n");
	        DRMAA2_DEBUG_PRINT("%d  - evaluates to the number of the signal that caused the process to stop.\n", WSTOPSIG(status));
	    }
    }


}