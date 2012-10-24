#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "drmaa2-debug.h"
#include "drmaa2-persistence.h"


void usage()
{
	printf("Usage: ./wrapper DATABASE JOB_ID\n");
}



int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		usage();
		exit(1);
	}

	long long row_id = atoll(argv[1]);
	if (row_id < 1)
	{
		fprintf(stderr, "No valid row_id: %s\n", argv[1]);
		usage();
		exit(1);
	}

	char *cmd = get_command(row_id);
    if (cmd == NULL)
    {
        fprintf(stderr, "Error: Could not read job command\n");
        exit(1);
    }

	pid_t childpid;

	if ((childpid = fork()) == -1)
    {
        perror("fork failed\n");
        exit(1);
    }
    
    if (childpid == 0)
    {
    	//child
    	setsid();	// create new process group so that jobs containing multiple processes can be killed
    	save_state_id(row_id, DRMAA2_RUNNING);
    	char *args[] = {cmd, NULL};
    	execv(args[0], args);
    	return 0;			// dead code, just to avoid GCC warning
    }
    else
    {
    	// parent
    	drmaa2_save_pid(row_id, childpid);

    	pid_t child;
    	int status;
		child = waitpid(childpid, &status, 0);

		drmaa2_save_exit_status(row_id, status);

		if (WIFEXITED(status))
	    {
	    	save_state_id(row_id, DRMAA2_DONE);
	        DRMAA2_DEBUG_PRINT("Process terminated normally by a call to _exit(2) or exit(3).\n");
	        DRMAA2_DEBUG_PRINT("%d  - evaluates to the low-order 8 bits of the argument passed to _exit(2) or exit(3) by the child.\n", WEXITSTATUS(status));
	    }
	    if (WIFSIGNALED(status))
	    {
	    	save_state_id(row_id, DRMAA2_FAILED);
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
	    return 0;
    }

}