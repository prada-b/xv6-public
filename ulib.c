#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p != '\0' && *p == *q)
    p++, q++;
  return *(unsigned char *)p - *(unsigned char *)q;
}

int
strcmp_casefold(const char *p, const char *q)
{
  while (*p != '\0' && *q != '\0' && (*p | 32) == (*q | 32)){
    p++, q++;
  }
  return *(unsigned char *)p - *(unsigned char *)q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void
strswp(char **a1, char **a2)
{
  char *temp = *a1;
  *a1 = *a2;
  *a2 = temp;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

bool
readline(int fd, char *line, int max_n)
{
  int i;
  bool reading = true;
  char c = '\0';

  i = 0;
  while(true){
    // Read single char
    // Break if...
    if (read(fd, &c, 1) < 1) { reading = false; break; } // EOF
    if (i == max_n-1)        { break; }             // Buffer almost full
    if (c == '\n')           { break; }             // EOL
    
    // Update buffer
    line[i] = c;
    i++;
  }
  // Null-terminate
  line[i] = '\0';

  // Are there lines remaining to read?
  return reading;
}
