#include "drmaa2.h"
#include <stdio.h>

int main ()
{
  drmaa2_jsession_h js;
  drmaa2_jtemplate jt;
  drmaa2_j_h j;
 
  drmaa2_version v=drmaa2_get_drmaa_version();
  printf("This is %s version %s.%s\n", drmaa2_get_drmaa_name(), v->major, v->minor);
  js = drmaa2_create_jsession("testsession", DRMAA2_UNSET_STRING);
  jt = drmaa2_jtemplate_create();
  jt->remoteCommand = "/bin/date";
  j = drmaa2_jsession_run_job(js, jt);
  drmaa2_j_wait_terminated(j, DRMAA2_INFINITE_TIME);
  drmaa2_jtemplate_free(jt);
  drmaa2_destroy_jsession("testsession");
  return 0;
}

