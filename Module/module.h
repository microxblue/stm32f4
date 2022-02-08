#ifndef __MODULE_H__
#define __MODULE_H__

#include <stdint.h>
#include <stdbool.h>

#include <stm32f4xx.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_adc.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_usart.h>

#define SYSTEM_CORE_CLOCK   168000000

#define __MODULE__

#include <module_api.h>

#define  printf            gCommonApi->Printf
#define  getchar           gCommonApi->GetChar
#define  haschar           gCommonApi->HasChar
#define  putchar           gCommonApi->PutChar
#define  delay_ms          gCommonApi->DelayMs
#define  delay_us          gCommonApi->DelayUs
#define  usb_get_rx_buf    gCommonApi->UsbGetRxBuf
#define  usb_free_rx_buf   gCommonApi->UsbFreeRxBuf
#define  usb_get_tx_buf    gCommonApi->UsbGetTxBuf
#define  usb_add_tx_buf    gCommonApi->UsbAddTxBuf

#define  ZERO_BSS() { \
                           extern unsigned int __bss_start__, __bss_end__; \
                           int *p = (int *)&__bss_start__; \
                           while (p < (int *)&__bss_end__) *p++ = 0; }

#endif
