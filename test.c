
#include "drmaa2.h"

int main ()
{
  drmaa2_jobsession_h js;
  drmaa2_job_template_t jt;
  drmaa2_job_h j;

  js=drmaa2_create_jobsession("testsession", DRMAA2_UNSET_STRING);
  jt=drmaa2_job_template_create();
  jt->remoteCommand="/bin/date";
  j=drmaa2_js_run_job(js, jt);
  drmaa2_j_wait_terminated(j, DRMAA2_INFINITE_TIME);
  drmaa2_job_template_free(jt);
  drmaa2_destroy_jobsession(js);
  return 0;
}

