#ifndef _USB_API_H_
#define _USB_API_H_

#include <stdint.h>
#include <stdbool.h>

#define EP_MAX_SIZE     64

uint8_t *usb_get_rx_buf(uint16_t *len);
void     usb_free_rx_buf();
uint8_t *usb_get_tx_buf(uint16_t  len);
void     usb_add_tx_buf(uint16_t  len);

bool     usb_haschar ();
int      usb_getchar ();
void     usb_putchar (int c);
void     usb_puts (char *s);
bool     usb_isfull ();

bool     is_usb_connected ();
bool     is_usb_console_ready ();

void     usb_init ();
void     usb_start ();

#endif
