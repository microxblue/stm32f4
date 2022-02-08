#ifndef  _USB_DEV_H_
#define  _USB_DEV_H_

#include <common.h>
#include <usb.h>
#include <usbapi.h>
#include <ringbuf.h>

#define MAX_RX_PKT_NUM  4
#define MAX_TX_PKT_NUM  6

#define MX_EP0_SIZE    0x40
#define MX_CON_IN_EP   0x81
#define MX_CON_OUT_EP  0x01
#define MX_CON_SZ      0x40
#define MX_DATA_OUT_EP 0x02
#define MX_DATA_IN_EP  0x83
#define MX_DATA_SZ     0x40

/* Declaration of the report descriptor */
struct mx_config {
  struct usb_config_descriptor        config;
  struct usb_interface_descriptor     comm;
  struct usb_endpoint_descriptor      comm_epout;
  struct usb_endpoint_descriptor      comm_epin;
  struct usb_interface_descriptor     data;
  struct usb_endpoint_descriptor      data_eprx;
  struct usb_endpoint_descriptor      data_eptx;
} __attribute__((packed));

typedef struct {
  uint8_t  buf[MAX_RX_PKT_NUM][EP_MAX_SIZE];
  uint8_t  len[MAX_RX_PKT_NUM];
  uint8_t  head;
  uint8_t  tail;
  uint8_t  count;
} USB_RX_BUF;

typedef struct {
  uint8_t  buf[MAX_TX_PKT_NUM][EP_MAX_SIZE];
  uint8_t  len[MAX_TX_PKT_NUM];
  uint8_t  head;
  uint8_t  tail;
  uint8_t  count;
} USB_TX_BUF;

#endif
