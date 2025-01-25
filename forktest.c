// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "types.h"
#include "stat.h"
#include "user.h"

#define N  1000

void
printf(int fd, const char *s, ...)
{
  xv6_write(fd, s, strlen(s));
}

void
forktest(void)
{
  int n, pid;

  printf(1, "fork test\n");

  for(n=0; n<N; n++){
    pid = xv6_fork();
    if(pid < 0)
      break;
    if(pid == 0)
      xv6_exit();
  }

  if(n == N){
    printf(1, "fork claimed to work N times!\n", N);
    xv6_exit();
  }

  for(; n > 0; n--){
    if(xv6_wait() < 0){
      printf(1, "wait stopped early\n");
      xv6_exit();
    }
  }

  if(xv6_wait() != -1){
    printf(1, "wait got too many\n");
    xv6_exit();
  }

  printf(1, "fork test OK\n");
}

int
main(void)
{
  forktest();
  xv6_exit();
}
