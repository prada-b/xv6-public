#include "types.h"
#include "stat.h"
#include "user.h"

char buf[512];

void
cat(int fd)
{
  int n;

  while((n = xv6_read(fd, buf, sizeof(buf))) > 0) {
    if (xv6_write(1, buf, n) != n) {
      printf(1, "cat: write error\n");
      xv6_exit();
    }
  }
  if(n < 0){
    printf(1, "cat: read error\n");
    xv6_exit();
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;

  if(argc <= 1){
    cat(0);
    xv6_exit();
  }

  for(i = 1; i < argc; i++){
    if((fd = xv6_open(argv[i], 0)) < 0){
      printf(1, "cat: cannot open %s\n", argv[i]);
      xv6_exit();
    }
    cat(fd);
    xv6_close(fd);
  }
  xv6_exit();
}
