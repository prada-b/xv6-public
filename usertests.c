#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

char buf[8192];
char name[3];
char *echoargv[] = { "echo", "ALL", "TESTS", "PASSED", 0 };
int stdout = 1;

// does chdir() call iput(p->cwd) in a transaction?
void
iputtest(void)
{
  printf(stdout, "iput test\n");

  if(xv6_mkdir("iputdir") < 0){
    printf(stdout, "mkdir failed\n");
    xv6_exit();
  }
  if(xv6_chdir("iputdir") < 0){
    printf(stdout, "chdir iputdir failed\n");
    xv6_exit();
  }
  if(xv6_unlink("../iputdir") < 0){
    printf(stdout, "unlink ../iputdir failed\n");
    xv6_exit();
  }
  if(xv6_chdir("/") < 0){
    printf(stdout, "chdir / failed\n");
    xv6_exit();
  }
  printf(stdout, "iput test ok\n");
}

// does xv6_exit() call iput(p->cwd) in a transaction?
void
exitiputtest(void)
{
  int pid;

  printf(stdout, "exitiput test\n");

  pid = xv6_fork();
  if(pid < 0){
    printf(stdout, "fork failed\n");
    xv6_exit();
  }
  if(pid == 0){
    if(xv6_mkdir("iputdir") < 0){
      printf(stdout, "mkdir failed\n");
      xv6_exit();
    }
    if(xv6_chdir("iputdir") < 0){
      printf(stdout, "child chdir failed\n");
      xv6_exit();
    }
    if(xv6_unlink("../iputdir") < 0){
      printf(stdout, "unlink ../iputdir failed\n");
      xv6_exit();
    }
    xv6_exit();
  }
  xv6_wait();
  printf(stdout, "exitiput test ok\n");
}

// does the error path in open() for attempt to write a
// directory call iput() in a transaction?
// needs a hacked kernel that pauses just after the namei()
// call in sys_open():
//    if((ip = namei(path)) == 0)
//      return -1;
//    {
//      int i;
//      for(i = 0; i < 10000; i++)
//        yield();
//    }
void
openiputtest(void)
{
  int pid;

  printf(stdout, "openiput test\n");
  if(xv6_mkdir("oidir") < 0){
    printf(stdout, "mkdir oidir failed\n");
    xv6_exit();
  }
  pid = xv6_fork();
  if(pid < 0){
    printf(stdout, "fork failed\n");
    xv6_exit();
  }
  if(pid == 0){
    int fd = xv6_open("oidir", O_RDWR);
    if(fd >= 0){
      printf(stdout, "open directory for write succeeded\n");
      xv6_exit();
    }
    xv6_exit();
  }
  xv6_sleep(1);
  if(xv6_unlink("oidir") != 0){
    printf(stdout, "unlink failed\n");
    xv6_exit();
  }
  xv6_wait();
  printf(stdout, "openiput test ok\n");
}

// simple file system tests

void
opentest(void)
{
  int fd;

  printf(stdout, "open test\n");
  fd = xv6_open("echo", 0);
  if(fd < 0){
    printf(stdout, "open echo failed!\n");
    xv6_exit();
  }
  xv6_close(fd);
  fd = xv6_open("doesnotexist", 0);
  if(fd >= 0){
    printf(stdout, "open doesnotexist succeeded!\n");
    xv6_exit();
  }
  printf(stdout, "open test ok\n");
}

void
writetest(void)
{
  int fd;
  int i;

  printf(stdout, "small file test\n");
  fd = xv6_open("small", O_CREATE|O_RDWR);
  if(fd >= 0){
    printf(stdout, "creat small succeeded; ok\n");
  } else {
    printf(stdout, "error: creat small failed!\n");
    xv6_exit();
  }
  for(i = 0; i < 100; i++){
    if(xv6_write(fd, "aaaaaaaaaa", 10) != 10){
      printf(stdout, "error: write aa %d new file failed\n", i);
      xv6_exit();
    }
    if(xv6_write(fd, "bbbbbbbbbb", 10) != 10){
      printf(stdout, "error: write bb %d new file failed\n", i);
      xv6_exit();
    }
  }
  printf(stdout, "writes ok\n");
  xv6_close(fd);
  fd = xv6_open("small", O_RDONLY);
  if(fd >= 0){
    printf(stdout, "open small succeeded ok\n");
  } else {
    printf(stdout, "error: open small failed!\n");
    xv6_exit();
  }
  i = xv6_read(fd, buf, 2000);
  if(i == 2000){
    printf(stdout, "read succeeded ok\n");
  } else {
    printf(stdout, "read failed\n");
    xv6_exit();
  }
  xv6_close(fd);

  if(xv6_unlink("small") < 0){
    printf(stdout, "unlink small failed\n");
    xv6_exit();
  }
  printf(stdout, "small file test ok\n");
}

void
writetest1(void)
{
  int i, fd, n;

  printf(stdout, "big files test\n");

  fd = xv6_open("big", O_CREATE|O_RDWR);
  if(fd < 0){
    printf(stdout, "error: creat big failed!\n");
    xv6_exit();
  }

  for(i = 0; i < MAXFILE; i++){
    ((int*)buf)[0] = i;
    if(xv6_write(fd, buf, 512) != 512){
      printf(stdout, "error: write big file failed\n", i);
      xv6_exit();
    }
  }

  xv6_close(fd);

  fd = xv6_open("big", O_RDONLY);
  if(fd < 0){
    printf(stdout, "error: open big failed!\n");
    xv6_exit();
  }

  n = 0;
  for(;;){
    i = xv6_read(fd, buf, 512);
    if(i == 0){
      if(n == MAXFILE - 1){
        printf(stdout, "read only %d blocks from big", n);
        xv6_exit();
      }
      break;
    } else if(i != 512){
      printf(stdout, "read failed %d\n", i);
      xv6_exit();
    }
    if(((int*)buf)[0] != n){
      printf(stdout, "read content of block %d is %d\n",
             n, ((int*)buf)[0]);
      xv6_exit();
    }
    n++;
  }
  xv6_close(fd);
  if(xv6_unlink("big") < 0){
    printf(stdout, "unlink big failed\n");
    xv6_exit();
  }
  printf(stdout, "big files ok\n");
}

void
createtest(void)
{
  int i, fd;

  printf(stdout, "many creates, followed by unlink test\n");

  name[0] = 'a';
  name[2] = '\0';
  for(i = 0; i < 52; i++){
    name[1] = '0' + i;
    fd = xv6_open(name, O_CREATE|O_RDWR);
    xv6_close(fd);
  }
  name[0] = 'a';
  name[2] = '\0';
  for(i = 0; i < 52; i++){
    name[1] = '0' + i;
    xv6_unlink(name);
  }
  printf(stdout, "many creates, followed by unlink; ok\n");
}

void dirtest(void)
{
  printf(stdout, "mkdir test\n");

  if(xv6_mkdir("dir0") < 0){
    printf(stdout, "mkdir failed\n");
    xv6_exit();
  }

  if(xv6_chdir("dir0") < 0){
    printf(stdout, "chdir dir0 failed\n");
    xv6_exit();
  }

  if(xv6_chdir("..") < 0){
    printf(stdout, "chdir .. failed\n");
    xv6_exit();
  }

  if(xv6_unlink("dir0") < 0){
    printf(stdout, "unlink dir0 failed\n");
    xv6_exit();
  }
  printf(stdout, "mkdir test ok\n");
}

void
exectest(void)
{
  printf(stdout, "exec test\n");
  if(xv6_exec("echo", echoargv) < 0){
    printf(stdout, "exec echo failed\n");
    xv6_exit();
  }
}

// simple fork and pipe read/write

void
pipe1(void)
{
  int fds[2], pid;
  int seq, i, n, cc, total;

  if(xv6_pipe(fds) != 0){
    printf(1, "pipe() failed\n");
    xv6_exit();
  }
  pid = xv6_fork();
  seq = 0;
  if(pid == 0){
    xv6_close(fds[0]);
    for(n = 0; n < 5; n++){
      for(i = 0; i < 1033; i++)
        buf[i] = seq++;
      if(xv6_write(fds[1], buf, 1033) != 1033){
        printf(1, "pipe1 oops 1\n");
        xv6_exit();
      }
    }
    xv6_exit();
  } else if(pid > 0){
    xv6_close(fds[1]);
    total = 0;
    cc = 1;
    while((n = xv6_read(fds[0], buf, cc)) > 0){
      for(i = 0; i < n; i++){
        if((buf[i] & 0xff) != (seq++ & 0xff)){
          printf(1, "pipe1 oops 2\n");
          return;
        }
      }
      total += n;
      cc = cc * 2;
      if(cc > sizeof(buf))
        cc = sizeof(buf);
    }
    if(total != 5 * 1033){
      printf(1, "pipe1 oops 3 total %d\n", total);
      xv6_exit();
    }
    xv6_close(fds[0]);
    xv6_wait();
  } else {
    printf(1, "fork() failed\n");
    xv6_exit();
  }
  printf(1, "pipe1 ok\n");
}

// meant to be run w/ at most two CPUs
void
preempt(void)
{
  int pid1, pid2, pid3;
  int pfds[2];

  printf(1, "preempt: ");
  pid1 = xv6_fork();
  if(pid1 == 0)
    for(;;)
      ;

  pid2 = xv6_fork();
  if(pid2 == 0)
    for(;;)
      ;

  xv6_pipe(pfds);
  pid3 = xv6_fork();
  if(pid3 == 0){
    xv6_close(pfds[0]);
    if(xv6_write(pfds[1], "x", 1) != 1)
      printf(1, "preempt write error");
    xv6_close(pfds[1]);
    for(;;)
      ;
  }

  xv6_close(pfds[1]);
  if(xv6_read(pfds[0], buf, sizeof(buf)) != 1){
    printf(1, "preempt read error");
    return;
  }
  xv6_close(pfds[0]);
  printf(1, "kill... ");
  xv6_kill(pid1);
  xv6_kill(pid2);
  xv6_kill(pid3);
  printf(1, "wait... ");
  xv6_wait();
  xv6_wait();
  xv6_wait();
  printf(1, "preempt ok\n");
}

// try to find any races between xv6_exit and wait
void
exitwait(void)
{
  int i, pid;

  for(i = 0; i < 100; i++){
    pid = xv6_fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      return;
    }
    if(pid){
      if(xv6_wait() != pid){
        printf(1, "wait wrong pid\n");
        return;
      }
    } else {
      xv6_exit();
    }
  }
  printf(1, "exitwait ok\n");
}

void
mem(void)
{
  void *m1, *m2;
  int pid, ppid;

  printf(1, "mem test\n");
  ppid = xv6_getpid();
  if((pid = xv6_fork()) == 0){
    m1 = 0;
    while((m2 = malloc(10001)) != 0){
      *(char**)m2 = m1;
      m1 = m2;
    }
    while(m1){
      m2 = *(char**)m1;
      free(m1);
      m1 = m2;
    }
    m1 = malloc(1024*20);
    if(m1 == 0){
      printf(1, "couldn't allocate mem?!!\n");
      xv6_kill(ppid);
      xv6_exit();
    }
    free(m1);
    printf(1, "mem ok\n");
    xv6_exit();
  } else {
    xv6_wait();
  }
}

// More file system tests

// two processes write to the same file descriptor
// is the offset shared? does inode locking work?
void
sharedfd(void)
{
  int fd, pid, i, n, nc, np;
  char buf[10];

  printf(1, "sharedfd test\n");

  xv6_unlink("sharedfd");
  fd = xv6_open("sharedfd", O_CREATE|O_RDWR);
  if(fd < 0){
    printf(1, "fstests: cannot open sharedfd for writing");
    return;
  }
  pid = xv6_fork();
  memset(buf, pid==0?'c':'p', sizeof(buf));
  for(i = 0; i < 1000; i++){
    if(xv6_write(fd, buf, sizeof(buf)) != sizeof(buf)){
      printf(1, "fstests: write sharedfd failed\n");
      break;
    }
  }
  if(pid == 0)
    xv6_exit();
  else
    xv6_wait();
  xv6_close(fd);
  fd = xv6_open("sharedfd", 0);
  if(fd < 0){
    printf(1, "fstests: cannot open sharedfd for reading\n");
    return;
  }
  nc = np = 0;
  while((n = xv6_read(fd, buf, sizeof(buf))) > 0){
    for(i = 0; i < sizeof(buf); i++){
      if(buf[i] == 'c')
        nc++;
      if(buf[i] == 'p')
        np++;
    }
  }
  xv6_close(fd);
  xv6_unlink("sharedfd");
  if(nc == 10000 && np == 10000){
    printf(1, "sharedfd ok\n");
  } else {
    printf(1, "sharedfd oops %d %d\n", nc, np);
    xv6_exit();
  }
}

// four processes write different files at the same
// time, to test block allocation.
void
fourfiles(void)
{
  int fd, pid, i, j, n, total, pi;
  char *names[] = { "f0", "f1", "f2", "f3" };
  char *fname;

  printf(1, "fourfiles test\n");

  for(pi = 0; pi < 4; pi++){
    fname = names[pi];
    xv6_unlink(fname);

    pid = xv6_fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      xv6_exit();
    }

    if(pid == 0){
      fd = xv6_open(fname, O_CREATE | O_RDWR);
      if(fd < 0){
        printf(1, "create failed\n");
        xv6_exit();
      }

      memset(buf, '0'+pi, 512);
      for(i = 0; i < 12; i++){
        if((n = xv6_write(fd, buf, 500)) != 500){
          printf(1, "write failed %d\n", n);
          xv6_exit();
        }
      }
      xv6_exit();
    }
  }

  for(pi = 0; pi < 4; pi++){
    xv6_wait();
  }

  for(i = 0; i < 2; i++){
    fname = names[i];
    fd = xv6_open(fname, 0);
    total = 0;
    while((n = xv6_read(fd, buf, sizeof(buf))) > 0){
      for(j = 0; j < n; j++){
        if(buf[j] != '0'+i){
          printf(1, "wrong char\n");
          xv6_exit();
        }
      }
      total += n;
    }
    xv6_close(fd);
    if(total != 12*500){
      printf(1, "wrong length %d\n", total);
      xv6_exit();
    }
    xv6_unlink(fname);
  }

  printf(1, "fourfiles ok\n");
}

// four processes create and delete different files in same directory
void
createdelete(void)
{
  enum { N = 20 };
  int pid, i, fd, pi;
  char name[32];

  printf(1, "createdelete test\n");

  for(pi = 0; pi < 4; pi++){
    pid = xv6_fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      xv6_exit();
    }

    if(pid == 0){
      name[0] = 'p' + pi;
      name[2] = '\0';
      for(i = 0; i < N; i++){
        name[1] = '0' + i;
        fd = xv6_open(name, O_CREATE | O_RDWR);
        if(fd < 0){
          printf(1, "create failed\n");
          xv6_exit();
        }
        xv6_close(fd);
        if(i > 0 && (i % 2 ) == 0){
          name[1] = '0' + (i / 2);
          if(xv6_unlink(name) < 0){
            printf(1, "unlink failed\n");
            xv6_exit();
          }
        }
      }
      xv6_exit();
    }
  }

  for(pi = 0; pi < 4; pi++){
    xv6_wait();
  }

  name[0] = name[1] = name[2] = 0;
  for(i = 0; i < N; i++){
    for(pi = 0; pi < 4; pi++){
      name[0] = 'p' + pi;
      name[1] = '0' + i;
      fd = xv6_open(name, 0);
      if((i == 0 || i >= N/2) && fd < 0){
        printf(1, "oops createdelete %s didn't exist\n", name);
        xv6_exit();
      } else if((i >= 1 && i < N/2) && fd >= 0){
        printf(1, "oops createdelete %s did exist\n", name);
        xv6_exit();
      }
      if(fd >= 0)
        xv6_close(fd);
    }
  }

  for(i = 0; i < N; i++){
    for(pi = 0; pi < 4; pi++){
      name[0] = 'p' + i;
      name[1] = '0' + i;
      xv6_unlink(name);
    }
  }

  printf(1, "createdelete ok\n");
}

// can I unlink a file and still read it?
void
unlinkread(void)
{
  int fd, fd1;

  printf(1, "unlinkread test\n");
  fd = xv6_open("unlinkread", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "create unlinkread failed\n");
    xv6_exit();
  }
  xv6_write(fd, "hello", 5);
  xv6_close(fd);

  fd = xv6_open("unlinkread", O_RDWR);
  if(fd < 0){
    printf(1, "open unlinkread failed\n");
    xv6_exit();
  }
  if(xv6_unlink("unlinkread") != 0){
    printf(1, "unlink unlinkread failed\n");
    xv6_exit();
  }

  fd1 = xv6_open("unlinkread", O_CREATE | O_RDWR);
  xv6_write(fd1, "yyy", 3);
  xv6_close(fd1);

  if(xv6_read(fd, buf, sizeof(buf)) != 5){
    printf(1, "unlinkread read failed");
    xv6_exit();
  }
  if(buf[0] != 'h'){
    printf(1, "unlinkread wrong data\n");
    xv6_exit();
  }
  if(xv6_write(fd, buf, 10) != 10){
    printf(1, "unlinkread write failed\n");
    xv6_exit();
  }
  xv6_close(fd);
  xv6_unlink("unlinkread");
  printf(1, "unlinkread ok\n");
}

void
linktest(void)
{
  int fd;

  printf(1, "linktest\n");

  xv6_unlink("lf1");
  xv6_unlink("lf2");

  fd = xv6_open("lf1", O_CREATE|O_RDWR);
  if(fd < 0){
    printf(1, "create lf1 failed\n");
    xv6_exit();
  }
  if(xv6_write(fd, "hello", 5) != 5){
    printf(1, "write lf1 failed\n");
    xv6_exit();
  }
  xv6_close(fd);

  if(xv6_link("lf1", "lf2") < 0){
    printf(1, "link lf1 lf2 failed\n");
    xv6_exit();
  }
  xv6_unlink("lf1");

  if(xv6_open("lf1", 0) >= 0){
    printf(1, "unlinked lf1 but it is still there!\n");
    xv6_exit();
  }

  fd = xv6_open("lf2", 0);
  if(fd < 0){
    printf(1, "open lf2 failed\n");
    xv6_exit();
  }
  if(xv6_read(fd, buf, sizeof(buf)) != 5){
    printf(1, "read lf2 failed\n");
    xv6_exit();
  }
  xv6_close(fd);

  if(xv6_link("lf2", "lf2") >= 0){
    printf(1, "link lf2 lf2 succeeded! oops\n");
    xv6_exit();
  }

  xv6_unlink("lf2");
  if(xv6_link("lf2", "lf1") >= 0){
    printf(1, "link non-existant succeeded! oops\n");
    xv6_exit();
  }

  if(xv6_link(".", "lf1") >= 0){
    printf(1, "link . lf1 succeeded! oops\n");
    xv6_exit();
  }

  printf(1, "linktest ok\n");
}

// test concurrent create/link/unlink of the same file
void
concreate(void)
{
  char file[3];
  int i, pid, n, fd;
  char fa[40];
  struct {
    ushort inum;
    char name[14];
  } de;

  printf(1, "concreate test\n");
  file[0] = 'C';
  file[2] = '\0';
  for(i = 0; i < 40; i++){
    file[1] = '0' + i;
    xv6_unlink(file);
    pid = xv6_fork();
    if(pid && (i % 3) == 1){
      xv6_link("C0", file);
    } else if(pid == 0 && (i % 5) == 1){
      xv6_link("C0", file);
    } else {
      fd = xv6_open(file, O_CREATE | O_RDWR);
      if(fd < 0){
        printf(1, "concreate create %s failed\n", file);
        xv6_exit();
      }
      xv6_close(fd);
    }
    if(pid == 0)
      xv6_exit();
    else
      xv6_wait();
  }

  memset(fa, 0, sizeof(fa));
  fd = xv6_open(".", 0);
  n = 0;
  while(xv6_read(fd, &de, sizeof(de)) > 0){
    if(de.inum == 0)
      continue;
    if(de.name[0] == 'C' && de.name[2] == '\0'){
      i = de.name[1] - '0';
      if(i < 0 || i >= sizeof(fa)){
        printf(1, "concreate weird file %s\n", de.name);
        xv6_exit();
      }
      if(fa[i]){
        printf(1, "concreate duplicate file %s\n", de.name);
        xv6_exit();
      }
      fa[i] = 1;
      n++;
    }
  }
  xv6_close(fd);

  if(n != 40){
    printf(1, "concreate not enough files in directory listing\n");
    xv6_exit();
  }

  for(i = 0; i < 40; i++){
    file[1] = '0' + i;
    pid = xv6_fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      xv6_exit();
    }
    if(((i % 3) == 0 && pid == 0) ||
       ((i % 3) == 1 && pid != 0)){
      xv6_close(xv6_open(file, 0));
      xv6_close(xv6_open(file, 0));
      xv6_close(xv6_open(file, 0));
      xv6_close(xv6_open(file, 0));
    } else {
      xv6_unlink(file);
      xv6_unlink(file);
      xv6_unlink(file);
      xv6_unlink(file);
    }
    if(pid == 0)
      xv6_exit();
    else
      xv6_wait();
  }

  printf(1, "concreate ok\n");
}

// another concurrent link/unlink/create test,
// to look for deadlocks.
void
linkunlink()
{
  int pid, i;

  printf(1, "linkunlink test\n");

  xv6_unlink("x");
  pid = xv6_fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    xv6_exit();
  }

  unsigned int x = (pid ? 1 : 97);
  for(i = 0; i < 100; i++){
    x = x * 1103515245 + 12345;
    if((x % 3) == 0){
      xv6_close(xv6_open("x", O_RDWR | O_CREATE));
    } else if((x % 3) == 1){
      xv6_link("cat", "x");
    } else {
      xv6_unlink("x");
    }
  }

  if(pid)
    xv6_wait();
  else
    xv6_exit();

  printf(1, "linkunlink ok\n");
}

// directory that uses indirect blocks
void
bigdir(void)
{
  int i, fd;
  char name[10];

  printf(1, "bigdir test\n");
  xv6_unlink("bd");

  fd = xv6_open("bd", O_CREATE);
  if(fd < 0){
    printf(1, "bigdir create failed\n");
    xv6_exit();
  }
  xv6_close(fd);

  for(i = 0; i < 500; i++){
    name[0] = 'x';
    name[1] = '0' + (i / 64);
    name[2] = '0' + (i % 64);
    name[3] = '\0';
    if(xv6_link("bd", name) != 0){
      printf(1, "bigdir link failed\n");
      xv6_exit();
    }
  }

  xv6_unlink("bd");
  for(i = 0; i < 500; i++){
    name[0] = 'x';
    name[1] = '0' + (i / 64);
    name[2] = '0' + (i % 64);
    name[3] = '\0';
    if(xv6_unlink(name) != 0){
      printf(1, "bigdir unlink failed");
      xv6_exit();
    }
  }

  printf(1, "bigdir ok\n");
}

void
subdir(void)
{
  int fd, cc;

  printf(1, "subdir test\n");

  xv6_unlink("ff");
  if(xv6_mkdir("dd") != 0){
    printf(1, "subdir mkdir dd failed\n");
    xv6_exit();
  }

  fd = xv6_open("dd/ff", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "create dd/ff failed\n");
    xv6_exit();
  }
  xv6_write(fd, "ff", 2);
  xv6_close(fd);

  if(xv6_unlink("dd") >= 0){
    printf(1, "unlink dd (non-empty dir) succeeded!\n");
    xv6_exit();
  }

  if(xv6_mkdir("/dd/dd") != 0){
    printf(1, "subdir mkdir dd/dd failed\n");
    xv6_exit();
  }

  fd = xv6_open("dd/dd/ff", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "create dd/dd/ff failed\n");
    xv6_exit();
  }
  xv6_write(fd, "FF", 2);
  xv6_close(fd);

  fd = xv6_open("dd/dd/../ff", 0);
  if(fd < 0){
    printf(1, "open dd/dd/../ff failed\n");
    xv6_exit();
  }
  cc = xv6_read(fd, buf, sizeof(buf));
  if(cc != 2 || buf[0] != 'f'){
    printf(1, "dd/dd/../ff wrong content\n");
    xv6_exit();
  }
  xv6_close(fd);

  if(xv6_link("dd/dd/ff", "dd/dd/ffff") != 0){
    printf(1, "link dd/dd/ff dd/dd/ffff failed\n");
    xv6_exit();
  }

  if(xv6_unlink("dd/dd/ff") != 0){
    printf(1, "unlink dd/dd/ff failed\n");
    xv6_exit();
  }
  if(xv6_open("dd/dd/ff", O_RDONLY) >= 0){
    printf(1, "open (unlinked) dd/dd/ff succeeded\n");
    xv6_exit();
  }

  if(xv6_chdir("dd") != 0){
    printf(1, "chdir dd failed\n");
    xv6_exit();
  }
  if(xv6_chdir("dd/../../dd") != 0){
    printf(1, "chdir dd/../../dd failed\n");
    xv6_exit();
  }
  if(xv6_chdir("dd/../../../dd") != 0){
    printf(1, "chdir dd/../../dd failed\n");
    xv6_exit();
  }
  if(xv6_chdir("./..") != 0){
    printf(1, "chdir ./.. failed\n");
    xv6_exit();
  }

  fd = xv6_open("dd/dd/ffff", 0);
  if(fd < 0){
    printf(1, "open dd/dd/ffff failed\n");
    xv6_exit();
  }
  if(xv6_read(fd, buf, sizeof(buf)) != 2){
    printf(1, "read dd/dd/ffff wrong len\n");
    xv6_exit();
  }
  xv6_close(fd);

  if(xv6_open("dd/dd/ff", O_RDONLY) >= 0){
    printf(1, "open (unlinked) dd/dd/ff succeeded!\n");
    xv6_exit();
  }

  if(xv6_open("dd/ff/ff", O_CREATE|O_RDWR) >= 0){
    printf(1, "create dd/ff/ff succeeded!\n");
    xv6_exit();
  }
  if(xv6_open("dd/xx/ff", O_CREATE|O_RDWR) >= 0){
    printf(1, "create dd/xx/ff succeeded!\n");
    xv6_exit();
  }
  if(xv6_open("dd", O_CREATE) >= 0){
    printf(1, "create dd succeeded!\n");
    xv6_exit();
  }
  if(xv6_open("dd", O_RDWR) >= 0){
    printf(1, "open dd rdwr succeeded!\n");
    xv6_exit();
  }
  if(xv6_open("dd", O_WRONLY) >= 0){
    printf(1, "open dd wronly succeeded!\n");
    xv6_exit();
  }
  if(xv6_link("dd/ff/ff", "dd/dd/xx") == 0){
    printf(1, "link dd/ff/ff dd/dd/xx succeeded!\n");
    xv6_exit();
  }
  if(xv6_link("dd/xx/ff", "dd/dd/xx") == 0){
    printf(1, "link dd/xx/ff dd/dd/xx succeeded!\n");
    xv6_exit();
  }
  if(xv6_link("dd/ff", "dd/dd/ffff") == 0){
    printf(1, "link dd/ff dd/dd/ffff succeeded!\n");
    xv6_exit();
  }
  if(xv6_mkdir("dd/ff/ff") == 0){
    printf(1, "mkdir dd/ff/ff succeeded!\n");
    xv6_exit();
  }
  if(xv6_mkdir("dd/xx/ff") == 0){
    printf(1, "mkdir dd/xx/ff succeeded!\n");
    xv6_exit();
  }
  if(xv6_mkdir("dd/dd/ffff") == 0){
    printf(1, "mkdir dd/dd/ffff succeeded!\n");
    xv6_exit();
  }
  if(xv6_unlink("dd/xx/ff") == 0){
    printf(1, "unlink dd/xx/ff succeeded!\n");
    xv6_exit();
  }
  if(xv6_unlink("dd/ff/ff") == 0){
    printf(1, "unlink dd/ff/ff succeeded!\n");
    xv6_exit();
  }
  if(xv6_chdir("dd/ff") == 0){
    printf(1, "chdir dd/ff succeeded!\n");
    xv6_exit();
  }
  if(xv6_chdir("dd/xx") == 0){
    printf(1, "chdir dd/xx succeeded!\n");
    xv6_exit();
  }

  if(xv6_unlink("dd/dd/ffff") != 0){
    printf(1, "unlink dd/dd/ff failed\n");
    xv6_exit();
  }
  if(xv6_unlink("dd/ff") != 0){
    printf(1, "unlink dd/ff failed\n");
    xv6_exit();
  }
  if(xv6_unlink("dd") == 0){
    printf(1, "unlink non-empty dd succeeded!\n");
    xv6_exit();
  }
  if(xv6_unlink("dd/dd") < 0){
    printf(1, "unlink dd/dd failed\n");
    xv6_exit();
  }
  if(xv6_unlink("dd") < 0){
    printf(1, "unlink dd failed\n");
    xv6_exit();
  }

  printf(1, "subdir ok\n");
}

// test writes that are larger than the log.
void
bigwrite(void)
{
  int fd, sz;

  printf(1, "bigwrite test\n");

  xv6_unlink("bigwrite");
  for(sz = 499; sz < 12*512; sz += 471){
    fd = xv6_open("bigwrite", O_CREATE | O_RDWR);
    if(fd < 0){
      printf(1, "cannot create bigwrite\n");
      xv6_exit();
    }
    int i;
    for(i = 0; i < 2; i++){
      int cc = xv6_write(fd, buf, sz);
      if(cc != sz){
        printf(1, "write(%d) ret %d\n", sz, cc);
        xv6_exit();
      }
    }
    xv6_close(fd);
    xv6_unlink("bigwrite");
  }

  printf(1, "bigwrite ok\n");
}

void
bigfile(void)
{
  int fd, i, total, cc;

  printf(1, "bigfile test\n");

  xv6_unlink("bigfile");
  fd = xv6_open("bigfile", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(1, "cannot create bigfile");
    xv6_exit();
  }
  for(i = 0; i < 20; i++){
    memset(buf, i, 600);
    if(xv6_write(fd, buf, 600) != 600){
      printf(1, "write bigfile failed\n");
      xv6_exit();
    }
  }
  xv6_close(fd);

  fd = xv6_open("bigfile", 0);
  if(fd < 0){
    printf(1, "cannot open bigfile\n");
    xv6_exit();
  }
  total = 0;
  for(i = 0; ; i++){
    cc = xv6_read(fd, buf, 300);
    if(cc < 0){
      printf(1, "read bigfile failed\n");
      xv6_exit();
    }
    if(cc == 0)
      break;
    if(cc != 300){
      printf(1, "short read bigfile\n");
      xv6_exit();
    }
    if(buf[0] != i/2 || buf[299] != i/2){
      printf(1, "read bigfile wrong data\n");
      xv6_exit();
    }
    total += cc;
  }
  xv6_close(fd);
  if(total != 20*600){
    printf(1, "read bigfile wrong total\n");
    xv6_exit();
  }
  xv6_unlink("bigfile");

  printf(1, "bigfile test ok\n");
}

void
fourteen(void)
{
  int fd;

  // DIRSIZ is 14.
  printf(1, "fourteen test\n");

  if(xv6_mkdir("12345678901234") != 0){
    printf(1, "mkdir 12345678901234 failed\n");
    xv6_exit();
  }
  if(xv6_mkdir("12345678901234/123456789012345") != 0){
    printf(1, "mkdir 12345678901234/123456789012345 failed\n");
    xv6_exit();
  }
  fd = xv6_open("123456789012345/123456789012345/123456789012345", O_CREATE);
  if(fd < 0){
    printf(1, "create 123456789012345/123456789012345/123456789012345 failed\n");
    xv6_exit();
  }
  xv6_close(fd);
  fd = xv6_open("12345678901234/12345678901234/12345678901234", 0);
  if(fd < 0){
    printf(1, "open 12345678901234/12345678901234/12345678901234 failed\n");
    xv6_exit();
  }
  xv6_close(fd);

  if(xv6_mkdir("12345678901234/12345678901234") == 0){
    printf(1, "mkdir 12345678901234/12345678901234 succeeded!\n");
    xv6_exit();
  }
  if(xv6_mkdir("123456789012345/12345678901234") == 0){
    printf(1, "mkdir 12345678901234/123456789012345 succeeded!\n");
    xv6_exit();
  }

  printf(1, "fourteen ok\n");
}

void
rmdot(void)
{
  printf(1, "rmdot test\n");
  if(xv6_mkdir("dots") != 0){
    printf(1, "mkdir dots failed\n");
    xv6_exit();
  }
  if(xv6_chdir("dots") != 0){
    printf(1, "chdir dots failed\n");
    xv6_exit();
  }
  if(xv6_unlink(".") == 0){
    printf(1, "rm . worked!\n");
    xv6_exit();
  }
  if(xv6_unlink("..") == 0){
    printf(1, "rm .. worked!\n");
    xv6_exit();
  }
  if(xv6_chdir("/") != 0){
    printf(1, "chdir / failed\n");
    xv6_exit();
  }
  if(xv6_unlink("dots/.") == 0){
    printf(1, "unlink dots/. worked!\n");
    xv6_exit();
  }
  if(xv6_unlink("dots/..") == 0){
    printf(1, "unlink dots/.. worked!\n");
    xv6_exit();
  }
  if(xv6_unlink("dots") != 0){
    printf(1, "unlink dots failed!\n");
    xv6_exit();
  }
  printf(1, "rmdot ok\n");
}

void
dirfile(void)
{
  int fd;

  printf(1, "dir vs file\n");

  fd = xv6_open("dirfile", O_CREATE);
  if(fd < 0){
    printf(1, "create dirfile failed\n");
    xv6_exit();
  }
  xv6_close(fd);
  if(xv6_chdir("dirfile") == 0){
    printf(1, "chdir dirfile succeeded!\n");
    xv6_exit();
  }
  fd = xv6_open("dirfile/xx", 0);
  if(fd >= 0){
    printf(1, "create dirfile/xx succeeded!\n");
    xv6_exit();
  }
  fd = xv6_open("dirfile/xx", O_CREATE);
  if(fd >= 0){
    printf(1, "create dirfile/xx succeeded!\n");
    xv6_exit();
  }
  if(xv6_mkdir("dirfile/xx") == 0){
    printf(1, "mkdir dirfile/xx succeeded!\n");
    xv6_exit();
  }
  if(xv6_unlink("dirfile/xx") == 0){
    printf(1, "unlink dirfile/xx succeeded!\n");
    xv6_exit();
  }
  if(xv6_link("README", "dirfile/xx") == 0){
    printf(1, "link to dirfile/xx succeeded!\n");
    xv6_exit();
  }
  if(xv6_unlink("dirfile") != 0){
    printf(1, "unlink dirfile failed!\n");
    xv6_exit();
  }

  fd = xv6_open(".", O_RDWR);
  if(fd >= 0){
    printf(1, "open . for writing succeeded!\n");
    xv6_exit();
  }
  fd = xv6_open(".", 0);
  if(xv6_write(fd, "x", 1) > 0){
    printf(1, "write . succeeded!\n");
    xv6_exit();
  }
  xv6_close(fd);

  printf(1, "dir vs file OK\n");
}

// test that iput() is called at the end of _namei()
void
iref(void)
{
  int i, fd;

  printf(1, "empty file name\n");

  // the 50 is NINODE
  for(i = 0; i < 50 + 1; i++){
    if(xv6_mkdir("irefd") != 0){
      printf(1, "mkdir irefd failed\n");
      xv6_exit();
    }
    if(xv6_chdir("irefd") != 0){
      printf(1, "chdir irefd failed\n");
      xv6_exit();
    }

    xv6_mkdir("");
    xv6_link("README", "");
    fd = xv6_open("", O_CREATE);
    if(fd >= 0)
      xv6_close(fd);
    fd = xv6_open("xx", O_CREATE);
    if(fd >= 0)
      xv6_close(fd);
    xv6_unlink("xx");
  }

  xv6_chdir("/");
  printf(1, "empty file name OK\n");
}

// test that fork fails gracefully
// the forktest binary also does this, but it runs out of proc entries first.
// inside the bigger usertests binary, we run out of memory first.
void
forktest(void)
{
  int n, pid;

  printf(1, "fork test\n");

  for(n=0; n<1000; n++){
    pid = xv6_fork();
    if(pid < 0)
      break;
    if(pid == 0)
      xv6_exit();
  }

  if(n == 1000){
    printf(1, "fork claimed to work 1000 times!\n");
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

void
sbrktest(void)
{
  int fds[2], pid, pids[10], ppid;
  char *a, *b, *c, *lastaddr, *oldbrk, *p, scratch;
  uint amt;

  printf(stdout, "sbrk test\n");
  oldbrk = xv6_sbrk(0);

  // can one sbrk() less than a page?
  a = xv6_sbrk(0);
  int i;
  for(i = 0; i < 5000; i++){
    b = xv6_sbrk(1);
    if(b != a){
      printf(stdout, "sbrk test failed %d %x %x\n", i, a, b);
      xv6_exit();
    }
    *b = 1;
    a = b + 1;
  }
  pid = xv6_fork();
  if(pid < 0){
    printf(stdout, "sbrk test fork failed\n");
    xv6_exit();
  }
  c = xv6_sbrk(1);
  c = xv6_sbrk(1);
  if(c != a + 1){
    printf(stdout, "sbrk test failed post-fork\n");
    xv6_exit();
  }
  if(pid == 0)
    xv6_exit();
  xv6_wait();

  // can one grow address space to something big?
#define BIG (100*1024*1024)
  a = xv6_sbrk(0);
  amt = (BIG) - (uint)a;
  p = xv6_sbrk(amt);
  if (p != a) {
    printf(stdout, "sbrk test failed to grow big address space; enough phys mem?\n");
    xv6_exit();
  }
  lastaddr = (char*) (BIG-1);
  *lastaddr = 99;

  // can one de-allocate?
  a = xv6_sbrk(0);
  c = xv6_sbrk(-4096);
  if(c == (char*)0xffffffff){
    printf(stdout, "sbrk could not deallocate\n");
    xv6_exit();
  }
  c = xv6_sbrk(0);
  if(c != a - 4096){
    printf(stdout, "sbrk deallocation produced wrong address, a %x c %x\n", a, c);
    xv6_exit();
  }

  // can one re-allocate that page?
  a = xv6_sbrk(0);
  c = xv6_sbrk(4096);
  if(c != a || xv6_sbrk(0) != a + 4096){
    printf(stdout, "sbrk re-allocation failed, a %x c %x\n", a, c);
    xv6_exit();
  }
  if(*lastaddr == 99){
    // should be zero
    printf(stdout, "sbrk de-allocation didn't really deallocate\n");
    xv6_exit();
  }

  a = xv6_sbrk(0);
  c = xv6_sbrk(-(xv6_sbrk(0) - oldbrk));
  if(c != a){
    printf(stdout, "sbrk downsize failed, a %x c %x\n", a, c);
    xv6_exit();
  }

  // can we read the kernel's memory?
  for(a = (char*)(KERNBASE); a < (char*) (KERNBASE+2000000); a += 50000){
    ppid = xv6_getpid();
    pid = xv6_fork();
    if(pid < 0){
      printf(stdout, "fork failed\n");
      xv6_exit();
    }
    if(pid == 0){
      printf(stdout, "oops could read %x = %x\n", a, *a);
      xv6_kill(ppid);
      xv6_exit();
    }
    xv6_wait();
  }

  // if we run the system out of memory, does it clean up the last
  // failed allocation?
  if(xv6_pipe(fds) != 0){
    printf(1, "pipe() failed\n");
    xv6_exit();
  }
  for(i = 0; i < sizeof(pids)/sizeof(pids[0]); i++){
    if((pids[i] = xv6_fork()) == 0){
      // allocate a lot of memory
      xv6_sbrk(BIG - (uint)xv6_sbrk(0));
      xv6_write(fds[1], "x", 1);
      // sit around until killed
      for(;;) xv6_sleep(1000);
    }
    if(pids[i] != -1)
      xv6_read(fds[0], &scratch, 1);
  }
  // if those failed allocations freed up the pages they did allocate,
  // we'll be able to allocate here
  c = xv6_sbrk(4096);
  for(i = 0; i < sizeof(pids)/sizeof(pids[0]); i++){
    if(pids[i] == -1)
      continue;
    xv6_kill(pids[i]);
    xv6_wait();
  }
  if(c == (char*)0xffffffff){
    printf(stdout, "failed sbrk leaked memory\n");
    xv6_exit();
  }

  if(xv6_sbrk(0) > oldbrk)
    xv6_sbrk(-(xv6_sbrk(0) - oldbrk));

  printf(stdout, "sbrk test OK\n");
}

void
validateint(int *p)
{
  int res;
  asm("mov %%esp, %%ebx\n\t"
      "mov %3, %%esp\n\t"
      "int %2\n\t"
      "mov %%ebx, %%esp" :
      "=a" (res) :
      "a" (SYS_sleep), "n" (T_SYSCALL), "c" (p) :
      "ebx");
}

void
validatetest(void)
{
  int hi, pid;
  uint p;

  printf(stdout, "validate test\n");
  hi = 1100*1024;

  for(p = 0; p <= (uint)hi; p += 4096){
    if((pid = xv6_fork()) == 0){
      // try to crash the kernel by passing in a badly placed integer
      validateint((int*)p);
      xv6_exit();
    }
    xv6_sleep(0);
    xv6_sleep(0);
    xv6_kill(pid);
    xv6_wait();

    // try to crash the kernel by passing in a bad string pointer
    if(xv6_link("nosuchfile", (char*)p) != -1){
      printf(stdout, "link should not succeed\n");
      xv6_exit();
    }
  }

  printf(stdout, "validate ok\n");
}

// does unintialized data start out zero?
char uninit[10000];
void
bsstest(void)
{
  int i;

  printf(stdout, "bss test\n");
  for(i = 0; i < sizeof(uninit); i++){
    if(uninit[i] != '\0'){
      printf(stdout, "bss test failed\n");
      xv6_exit();
    }
  }
  printf(stdout, "bss test ok\n");
}

// does exec return an error if the arguments
// are larger than a page? or does it write
// below the stack and wreck the instructions/data?
void
bigargtest(void)
{
  int pid, fd;

  xv6_unlink("bigarg-ok");
  pid = xv6_fork();
  if(pid == 0){
    static char *args[MAXARG];
    int i;
    for(i = 0; i < MAXARG-1; i++)
      args[i] = "bigargs test: failed\n                                                                                                                                                                                                       ";
    args[MAXARG-1] = 0;
    printf(stdout, "bigarg test\n");
    xv6_exec("echo", args);
    printf(stdout, "bigarg test ok\n");
    fd = xv6_open("bigarg-ok", O_CREATE);
    xv6_close(fd);
    xv6_exit();
  } else if(pid < 0){
    printf(stdout, "bigargtest: fork failed\n");
    xv6_exit();
  }
  xv6_wait();
  fd = xv6_open("bigarg-ok", 0);
  if(fd < 0){
    printf(stdout, "bigarg test failed!\n");
    xv6_exit();
  }
  xv6_close(fd);
  xv6_unlink("bigarg-ok");
}

// what happens when the file system runs out of blocks?
// answer: balloc panics, so this test is not useful.
void
fsfull()
{
  int nfiles;
  int fsblocks = 0;

  printf(1, "fsfull test\n");

  for(nfiles = 0; ; nfiles++){
    char name[64];
    name[0] = 'f';
    name[1] = '0' + nfiles / 1000;
    name[2] = '0' + (nfiles % 1000) / 100;
    name[3] = '0' + (nfiles % 100) / 10;
    name[4] = '0' + (nfiles % 10);
    name[5] = '\0';
    printf(1, "writing %s\n", name);
    int fd = xv6_open(name, O_CREATE|O_RDWR);
    if(fd < 0){
      printf(1, "open %s failed\n", name);
      break;
    }
    int total = 0;
    while(1){
      int cc = xv6_write(fd, buf, 512);
      if(cc < 512)
        break;
      total += cc;
      fsblocks++;
    }
    printf(1, "wrote %d bytes\n", total);
    xv6_close(fd);
    if(total == 0)
      break;
  }

  while(nfiles >= 0){
    char name[64];
    name[0] = 'f';
    name[1] = '0' + nfiles / 1000;
    name[2] = '0' + (nfiles % 1000) / 100;
    name[3] = '0' + (nfiles % 100) / 10;
    name[4] = '0' + (nfiles % 10);
    name[5] = '\0';
    xv6_unlink(name);
    nfiles--;
  }

  printf(1, "fsfull test finished\n");
}

void
uio()
{
  #define RTC_ADDR 0x70
  #define RTC_DATA 0x71

  ushort port = 0;
  uchar val = 0;
  int pid;

  printf(1, "uio test\n");
  pid = xv6_fork();
  if(pid == 0){
    port = RTC_ADDR;
    val = 0x09;  /* year */
    /* http://wiki.osdev.org/Inline_Assembly/Examples */
    asm volatile("outb %0,%1"::"a"(val), "d" (port));
    port = RTC_DATA;
    asm volatile("inb %1,%0" : "=a" (val) : "d" (port));
    printf(1, "uio: uio succeeded; test FAILED\n");
    xv6_exit();
  } else if(pid < 0){
    printf (1, "fork failed\n");
    xv6_exit();
  }
  xv6_wait();
  printf(1, "uio test done\n");
}

void argptest()
{
  int fd;
  fd = xv6_open("init", O_RDONLY);
  if (fd < 0) {
    printf(2, "open failed\n");
    xv6_exit();
  }
  xv6_read(fd, xv6_sbrk(0) - 1, -1);
  xv6_close(fd);
  printf(1, "arg test passed\n");
}

unsigned long randstate = 1;
unsigned int
rand()
{
  randstate = randstate * 1664525 + 1013904223;
  return randstate;
}

int
main(int argc, char *argv[])
{
  printf(1, "usertests starting\n");

  if(xv6_open("usertests.ran", 0) >= 0){
    printf(1, "already ran user tests -- rebuild fs.img\n");
    xv6_exit();
  }
  xv6_close(xv6_open("usertests.ran", O_CREATE));

  argptest();
  createdelete();
  linkunlink();
  concreate();
  fourfiles();
  sharedfd();

  bigargtest();
  bigwrite();
  bigargtest();
  bsstest();
  sbrktest();
  validatetest();

  opentest();
  writetest();
  writetest1();
  createtest();

  openiputtest();
  exitiputtest();
  iputtest();

  mem();
  pipe1();
  preempt();
  exitwait();

  rmdot();
  fourteen();
  bigfile();
  subdir();
  linktest();
  unlinkread();
  dirfile();
  iref();
  forktest();
  bigdir(); // slow

  uio();

  exectest();

  xv6_exit();
}
