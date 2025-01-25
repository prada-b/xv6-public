#include "types.h"
#include "stat.h"
#include "user.h"

char buf[512];

void
wc(int fd, char *name)
{
  int i, n;
  int l, w, c, inword;

  l = w = c = 0;
  inword = 0;
  while((n = xv6_read(fd, buf, sizeof(buf))) > 0){
    for(i=0; i<n; i++){
      c++;
      if(buf[i] == '\n')
        l++;
      if(strchr(" \r\t\n\v", buf[i]))
        inword = 0;
      else if(!inword){
        w++;
        inword = 1;
      }
    }
  }
  if(n < 0){
    printf(1, "wc: read error\n");
    xv6_exit();
  }
  printf(1, "%d %d %d %s\n", l, w, c, name);
}

int
main(int argc, char *argv[])
{
  int fd, i;

  if(argc <= 1){
    wc(0, "");
    xv6_exit();
  }

  for(i = 1; i < argc; i++){
    if((fd = xv6_open(argv[i], 0)) < 0){
      printf(1, "wc: cannot open %s\n", argv[i]);
      xv6_exit();
    }
    wc(fd, argv[i]);
    xv6_close(fd);
  }
  xv6_exit();
}
