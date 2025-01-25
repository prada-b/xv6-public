struct stat;
struct rtcdate;

// system calls
int xv6_fork(void);
int xv6_exit(void) __attribute__((noreturn));
int xv6_wait(void);
int xv6_pipe(int*);
int xv6_write(int, const void*, int);
int xv6_read(int, void*, int);
int xv6_close(int);
int xv6_kill(int);
int xv6_exec(char*, char**);
int xv6_open(const char*, int);
int xv6_mknod(const char*, short, short);
int xv6_unlink(const char*);
int xv6_fstat(int fd, struct stat*);
int xv6_link(const char*, const char*);
int xv6_mkdir(const char*);
int xv6_chdir(const char*);
int xv6_dup(int);
int xv6_getpid(void);
char* xv6_sbrk(int);
int xv6_sleep(int);
int xv6_uptime(void);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
