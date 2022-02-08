#include "board.h"
#include "usbapi.h"

#if USB_CONSOLE

void putsc (char ch)
{
  usb_putchar(ch);
}

char hassc ()
{
  return usb_haschar();
}

char getsc ()
{
  while (!usb_haschar());
  return usb_getchar();
}

#else

void putsc (char ch)
{
  while (!(UART_CONSOLE->SR & USART_FLAG_TXE));
  UART_CONSOLE->DR = ch;
}

char hassc ()
{
  return UART_CONSOLE->SR & USART_FLAG_RXNE;
}

char getsc ()
{
  while (!hassc());
  return UART_CONSOLE->DR;
}

#endif


int puts(const char *s)
{
  while (*s) putsc(*s++);
  return 0;
}
