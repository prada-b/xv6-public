// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };

int
main(void)
{
  int pid, wpid;

  if(xv6_open("console", O_RDWR) < 0){
    xv6_mknod("console", 1, 1);
    xv6_open("console", O_RDWR);
  }
  xv6_dup(0);  // stdout
  xv6_dup(0);  // stderr

  for(;;){
    printf(1, "init: starting sh\n");
    pid = xv6_fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      xv6_exit();
    }
    if(pid == 0){
      xv6_exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      xv6_exit();
    }
    while((wpid=xv6_wait()) >= 0 && wpid != pid)
      printf(1, "zombie!\n");
  }
}
