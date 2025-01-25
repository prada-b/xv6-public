// Create a zombie process that
// must be reparented at exit.

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  if(xv6_fork() > 0)
    xv6_sleep(5);  // Let child exit before parent.
  xv6_exit();
}
