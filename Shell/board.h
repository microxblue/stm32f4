#ifndef  _BOARD_H_
#define  _BOARD_H_

// 0: UART console, 1: USB console
#define  USB_CONSOLE       1

#define  UART_CONSOLE      UART4

#if USB_CONSOLE
  #define  IS_CONSOLE_READY()  is_usb_console_ready()
#else
  #define  IS_CONSOLE_READY()  (1)
#endif

#include <stm32f4xx.h>
#include <stm32f4xx_tim.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_flash.h>

#include "module_api.h"

#endif