#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

char*
fmtname(char *path, short type)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ){
    return p;
  }
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  // Mark if directory
  if(type == T_DIR){
    memset(buf+strlen(p), '/', 1);
  }
  return buf;
}

void
print_dirent(char *path, struct stat st, bool show_hidden)
{
  char *path_name = fmtname(path, st.type);
  if((path_name[0] == '.') && !show_hidden){
    return;
  }
  printf(1, "%s %d %d %d\n", path_name, st.type, st.ino, st.size);
}

void
ls(char *path, bool show_hidden)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = xv6_open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(xv6_fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    xv6_close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    print_dirent(path, st, show_hidden);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(xv6_read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      print_dirent(buf, st, show_hidden);
    }
    break;
  }
  xv6_close(fd);
}

int
main(int argc, char *argv[])
{
  int i;
  bool show_hidden = false;

  for(i=1; i<argc; i++){
    if(argv[i][0] == '-'){
      switch(argv[i][1]){
      case 'a':
        show_hidden = true;
      break;
      default:
        printf(1, "undefined argument %c\n", argv[i][1]);
        break;
      }
    }
    else{
      break;
    }
  }

  if(i == argc){
    ls(".", show_hidden);
    xv6_exit();
  }
  for(; i<argc; i++)
    ls(argv[i], show_hidden);
  xv6_exit();
}
