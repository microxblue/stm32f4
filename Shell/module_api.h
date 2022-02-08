#ifndef _MODULE_API_H_
#define _MODULE_API_H_

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

#define   KB(x)             (x<<10)

#define   SYS_FLASH_BASE    (0x08000000)
#define   SYS_SRAM_BASE     (0x20000000)
#define   SYS_SRAM_SIZE     KB(128)
#define   SYS_SRAM_END      (SYS_SRAM_BASE + SYS_SRAM_SIZE)

#define   COMMON_API_BASE   (SYS_SRAM_END - KB(1))
#define   ISR_VECT_BASE     (COMMON_API_BASE + 0x0200)
#define   ISR_VECT_SIZE     (0x188)
#define   USER_REG_BASE     (COMMON_API_BASE + 0x01D0)
#define   MOD_PTR1          (COMMON_API_BASE + 0x01D8)
#define   MOD_PTR2          (COMMON_API_BASE + 0x01DC)
#define   SHELL_CMD_BASE    (COMMON_API_BASE + 0x01E0)
#define   SHELL_CMD_SIZE    (0x020)

typedef int      (*API_MODENTRY)();
typedef int      (*API_PRINTF)(const char *, ...);
typedef char     (*API_HASCHAR)();
typedef char     (*API_GETCHAR)();
typedef void     (*API_PUTCHAR)(char c);
typedef void     (*API_DELAY_MS)(uint32_t delay);
typedef void     (*API_DELAY_US)(uint32_t delay);
typedef void     (*API_GPIOINIT)(void *GPIOx, void *GPIO_Init);
typedef uint32_t (*API_GETTICK)(void);
typedef float    (*API_FLOAT_ONE)(float a);
typedef float    (*API_FLOAT_TWO)(float a, float b);
typedef uint8_t *(*API_USB_GET_RX_BUF)(unsigned short *len);
typedef void     (*API_USB_FREE_RX_BUF)();
typedef uint8_t *(*API_USB_GET_TX_BUF)(unsigned short len);
typedef void     (*API_USB_ADD_TX_BUF)(unsigned short len);

#define STRUCT_OFF(s,f)   (UINT32)(&((s *)0)->f)

#define  CMD_LEN    4
#define  CMD_NUM    4

#pragma pack(1)

typedef struct {
  // offset 0x000
  union {
    unsigned char  param_c;
    unsigned short param_s;
    unsigned long  param_l;
  } params[4];

  // offset 0x010
  unsigned char        paramcnt;
  unsigned char        reserved[3];
  unsigned int         status[2];

  // offset 0x20
  API_HASCHAR          HasChar;
  API_GETCHAR          GetChar;
  API_PUTCHAR          PutChar;
  API_PRINTF           Printf;
  API_DELAY_MS         DelayMs;
  API_DELAY_US         DelayUs;
  API_GETTICK          GetTick;

  API_USB_GET_RX_BUF   UsbGetRxBuf;
  API_USB_FREE_RX_BUF  UsbFreeRxBuf;
  API_USB_GET_TX_BUF   UsbGetTxBuf;
  API_USB_ADD_TX_BUF   UsbAddTxBuf;

  // Must not exceed offset 0x300
  // Since it is used as  ISRBASE
} COMMON_API;
#pragma pack()

extern COMMON_API    *gCommonApi;
extern API_MODENTRY   gModEntry;

#endif


