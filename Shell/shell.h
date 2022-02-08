#ifndef  _SHELL_H_
#define  _SHELL_H_

#include "memory.h"

#define   MAX_LINE_LEN      SHELL_CMD_SIZE

#define   CMD_HASH(x,y) (((x)<<8)| (y))

unsigned char get_ser_cmdline (char *cmdline);
unsigned char get_mem_cmdline (char *cmdline);
unsigned char get_usb_cmdline (char *cmdline);
void parse_cmdline (char *str);
void print_banner (void);
void debug_loop (void);
void irom_prog (void);

#endif