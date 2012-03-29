#include "drmaa2.h"
#include <stdio.h>

int main ()
{
  drmaa2_jsession_h js;
  drmaa2_jtemplate jt;
  drmaa2_j_h j;

  js = drmaa2_create_jobsession("testsession", DRMAA2_UNSET_STRING);
  jt = drmaa2_job_template_create();
  jt->remoteCommand = "/bin/date";
  j = drmaa2_jsession_run_job(js, jt);
  drmaa2_j_wait_terminated(j, DRMAA2_INFINITE_TIME);
  drmaa2_jtemplate_free(jt);
  drmaa2_destroy_jobsession(js);
  return 0;
}

