#ifndef  _COMMON_H_
#define  _COMMON_H_

#include <stdbool.h>
#include <stdint.h>

#define  CMD_PACKET_LEN    12
#define  ARRAY_SIZE(Array) (sizeof (Array) / sizeof ((Array)[0]))

#ifndef  __MODULE__
void     putsc (char ch);
char     hassc ();
char     getsc ();
int      puts (const char *s);
#define  haschar   hassc
#define  putchar   putsc
#define  getchar   getsc
#endif

typedef  unsigned char     BYTE;
typedef  unsigned short    WORD;
typedef  unsigned int     DWORD;

typedef  unsigned char    UINT8;
typedef  unsigned short  UINT16;
typedef  unsigned int    UINT32;

typedef  char              CHAR;
typedef  char              INT8;
typedef  short            INT16;
typedef  int              INT32;
typedef  unsigned int    size_t;

#define  NULL            ((void *)0)

void         *memcpy(void *d, void *s, size_t n);
void         *memset(void *s, int c, size_t n);
int           memcmp(void *s1, void *s2, size_t n);
int           strncmpi(void *s1, void *s2, size_t n);
char         *strcpy(char *dest,const char *src);
char         *strcat(char *dest, const char *src);
int           strcmp(const char * cs,const char * ct);
int           strncmp(const char * cs,const char * ct, int count);
int           strlen(const char * s);
int           toupper(int ch);
int           tolower(int ch);
char         *skipchar (char *str, char ch);
char         *findchar (char *str, char ch);
unsigned long xtoi (char *str);


int           printf (const char *fmt, ...);
void          delay_us (uint32_t us);
void          delay_ms (uint32_t ms);

#endif
