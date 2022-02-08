#include <common.h>
#include <stm32.h>
#include "usbmcu.h"
#include "usbdev.h"

static    USB_OTG_GlobalTypeDef * const OTG  = (void*)(USB_OTG_FS_PERIPH_BASE + USB_OTG_GLOBAL_BASE);

#define   CON_IN_SOF_INVERVAL   0xF

usbd_device        m_usb_dev;
uint32_t           m_usb_critical_nesting;

static bool        m_usb_connected;
static bool        m_usb_console_ready;

static uint32_t    m_usb_sof_count = 0;
static ringbuf_t   m_usb_con_rx_rb;
static ringbuf_t   m_usb_con_tx_rb;

static uint32_t    m_ubuf[0x20]  LD_CCMRAM;

static uint8_t     m_fifo_tx[EP_MAX_SIZE]  LD_CCMRAM;
static uint8_t     m_fifo_rx[EP_MAX_SIZE]  LD_CCMRAM;

static  uint8_t    m_usb_con_rx_buf[EP_MAX_SIZE]     LD_CCMRAM;
static  uint8_t    m_usb_con_tx_buf[EP_MAX_SIZE*4]   LD_CCMRAM;

USB_RX_BUF  m_usb_rx_buf  LD_CCMRAM;
USB_TX_BUF  m_usb_tx_buf  LD_CCMRAM;

void usb_buf_init (void);

/* Device descriptor */
static const struct usb_device_descriptor device_desc = {
  .bLength            = sizeof (struct usb_device_descriptor),
  .bDescriptorType    = USB_DTYPE_DEVICE,
  .bcdUSB             = VERSION_BCD (2, 0, 0),
  .bDeviceClass       = 0xEF,
  .bDeviceSubClass    = 0x02,
  .bDeviceProtocol    = 0x01,
  .bMaxPacketSize0    = MX_EP0_SIZE,
  .idVendor           = 0x0686,
  .idProduct          = 0x1023,
  .bcdDevice          = VERSION_BCD (2, 0, 0),
  .iManufacturer      = 1,
  .iProduct           = 2,
  .iSerialNumber      = 3,
  .bNumConfigurations = 1,
};

/* Device configuration descriptor */
static const struct mx_config config_desc = {
  .config = {
    .bLength                = sizeof (struct usb_config_descriptor),
    .bDescriptorType        = USB_DTYPE_CONFIGURATION,
    .wTotalLength           = sizeof (struct mx_config),
    .bNumInterfaces         = 2,
    .bConfigurationValue    = 1,
    .iConfiguration         = NO_DESCRIPTOR,
    .bmAttributes           = USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPOWERED,
    .bMaxPower              = USB_CFG_POWER_MA (100),
  },

  .comm = {
    .bLength                = sizeof (struct usb_interface_descriptor),
    .bDescriptorType        = USB_DTYPE_INTERFACE,
    .bInterfaceNumber       = 0,
    .bAlternateSetting      = 0,
    .bNumEndpoints          = 2,
    .bInterfaceClass        = 0xFF,
    .bInterfaceSubClass     = 0xFF,
    .bInterfaceProtocol     = 0xFF,
    .iInterface             = 0x00,
  },

  .comm_epin = {
    .bLength                = sizeof (struct usb_endpoint_descriptor),
    .bDescriptorType        = USB_DTYPE_ENDPOINT,
    .bEndpointAddress       = MX_CON_IN_EP,
    .bmAttributes           = USB_EPTYPE_BULK,
    .wMaxPacketSize         = MX_CON_SZ,
    .bInterval              = 0x00,
  },

  .comm_epout = {
    .bLength                = sizeof (struct usb_endpoint_descriptor),
    .bDescriptorType        = USB_DTYPE_ENDPOINT,
    .bEndpointAddress       = MX_CON_OUT_EP,
    .bmAttributes           = USB_EPTYPE_BULK,
    .wMaxPacketSize         = MX_CON_SZ,
    .bInterval              = 0x00,
  },

  .data = {
    .bLength                = sizeof (struct usb_interface_descriptor),
    .bDescriptorType        = USB_DTYPE_INTERFACE,
    .bInterfaceNumber       = 1,
    .bAlternateSetting      = 0,
    .bNumEndpoints          = 2,
    .bInterfaceClass        = 0xFF,
    .bInterfaceSubClass     = 0xFF,
    .bInterfaceProtocol     = 0xFF,
    .iInterface             = 0x00,
  },

  .data_eprx = {
    .bLength                = sizeof (struct usb_endpoint_descriptor),
    .bDescriptorType        = USB_DTYPE_ENDPOINT,
    .bEndpointAddress       = MX_DATA_IN_EP,
    .bmAttributes           = USB_EPTYPE_BULK,
    .wMaxPacketSize         = MX_DATA_SZ,
    .bInterval              = 0x00,
  },

  .data_eptx = {
    .bLength                = sizeof (struct usb_endpoint_descriptor),
    .bDescriptorType        = USB_DTYPE_ENDPOINT,
    .bEndpointAddress       = MX_DATA_OUT_EP,
    .bmAttributes           = USB_EPTYPE_BULK,
    .wMaxPacketSize         = MX_DATA_SZ,
    .bInterval              = 0x00,
  },

};

static const struct usb_string_descriptor lang_desc     = USB_ARRAY_DESC (USB_LANGID_ENG_US);
static const struct usb_string_descriptor manuf_desc_en = USB_STRING_DESC ("MX");
static const struct usb_string_descriptor prod_desc_en  = USB_STRING_DESC ("STM32 DEV");
static const struct usb_string_descriptor sn_desc_en    = USB_STRING_DESC ("STM32F407");
static const struct usb_string_descriptor *const dtable[] = {
  &lang_desc,
  &manuf_desc_en,
  &prod_desc_en,
  &sn_desc_en
};

static void mx_reset (usbd_device *dev, uint8_t event, uint8_t ep)
{
  m_usb_sof_count     = 0;
  m_usb_connected     = false;
  m_usb_console_ready = false;
}

static usbd_respond mx_getdesc (usbd_ctlreq *req, void **address, uint16_t *length)
{
  const uint8_t dtype = req->wValue >> 8;
  const uint8_t dnumber = req->wValue & 0xFF;
  const void *desc;
  uint16_t len = 0;

  switch (dtype) {
  case USB_DTYPE_DEVICE:
    desc = &device_desc;
    if (!m_usb_connected) {
      m_usb_connected = true;
    }
    break;
  case USB_DTYPE_CONFIGURATION:
    desc = &config_desc;
    len = sizeof (config_desc);
    break;
  case USB_DTYPE_STRING:
    if (dnumber < 4) {
      desc = dtable[dnumber];
    } else {
      return usbd_fail;
    }
    break;
  default:
    return usbd_fail;
  }
  if (len == 0) {
    len = ((struct usb_header_descriptor *)desc)->bLength;
  }
  *address = (void *)desc;
  *length = len;
  return usbd_ack;
}

static
usbd_respond mx_control (usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback)
{
  if (((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) == (USB_REQ_INTERFACE | USB_REQ_CLASS)
      && req->wIndex == 0 ) {
    switch (req->bRequest) {
    default:
      return usbd_fail;
    }
  }
  return usbd_fail;
}

static
void mx_data_in (usbd_device *dev, uint8_t event, uint8_t ep)
{
  uint8_t  *buf;
  uint8_t   head;
  uint16_t  len;
  int32_t   plen;

  // send data out to host
  if (m_usb_tx_buf.count) {
    head = m_usb_tx_buf.head;
    len  = m_usb_tx_buf.len[head];
    if (usbd_ep_write (dev, ep, (void *)-1, len) == 0) {
      buf  = m_usb_tx_buf.buf[head];
      plen = usbd_ep_write (dev, ep, buf, len);
      if (plen >= 0) {
        m_usb_tx_buf.len[head] = 0xFF;
        if (++head >= MAX_TX_PKT_NUM) {
          head = 0;
        }
        m_usb_tx_buf.head = head;
        m_usb_tx_buf.count--;
      }
    }
  }
}

void usb_data_in (void)
{
  mx_data_in (&m_usb_dev, 0, MX_DATA_IN_EP);
}

static
void mx_data_out (usbd_device *dev, uint8_t event, uint8_t ep)
{
  uint8_t   tail;
  int32_t   plen;
  uint32_t  irqmsk;

  // received data from host
  irqmsk = OTG->GINTMSK;
  if (m_usb_rx_buf.count >= MAX_RX_PKT_NUM) {
    // no space, disable RXFLVL interrupt to avoid interrupt storms;
    OTG->GINTMSK = irqmsk & ~USB_OTG_GINTMSK_RXFLVLM;
    return;
  }

  if (!(irqmsk & USB_OTG_GINTMSK_RXFLVLM)) {
    OTG->GINTMSK = irqmsk | USB_OTG_GINTMSK_RXFLVLM;
  }

  tail = m_usb_rx_buf.tail;
  plen = usbd_ep_read (dev, ep, m_usb_rx_buf.buf[tail], EP_MAX_SIZE);
  if (plen >= 0) {
    m_usb_rx_buf.len[tail] = (uint8_t)plen;
    if (++tail >= MAX_RX_PKT_NUM) {
      tail = 0;
    }
    m_usb_rx_buf.tail = tail;
    m_usb_rx_buf.count++;
  }
}

static
void mx_con_in (usbd_device *dev, uint8_t event, uint8_t ep)
{
  uint16_t  dlen;

  // send buffer to host
  dlen = rb_get_count (&m_usb_con_tx_rb);
  if (dlen == 0) {
    return;
  }

  if (dlen > EP_MAX_SIZE) {
    dlen = EP_MAX_SIZE;
  }

  if (usbd_ep_write (dev, ep, (void *)-1, dlen) == 0) {
    rb_gets (&m_usb_con_tx_rb, m_fifo_tx, dlen);
    usbd_ep_write (dev, ep, m_fifo_tx, dlen);
  }
}


static
void mx_con_out (usbd_device *dev, uint8_t event, uint8_t ep)
{
  int32_t   plen;
  uint16_t  dlen;

  if (!m_usb_console_ready) {
    m_usb_console_ready = true;
  }

  // recv buffer from host
  dlen = rb_get_free_count (&m_usb_con_tx_rb);
  if (dlen > 0) {
    plen = usbd_ep_read (dev, ep, m_fifo_rx, dlen);
    if (plen > 0) {
      rb_puts (&m_usb_con_rx_rb, m_fifo_rx, plen);
    }
  }
}

static void mx_sof (usbd_device *dev, uint8_t event, uint8_t ep)
{
  m_usb_sof_count++;
  if (!m_usb_connected)
    return;

  mx_con_in  (&m_usb_dev, 0, MX_CON_IN_EP);
  mx_data_in (&m_usb_dev, 0, MX_DATA_IN_EP);
}

static usbd_respond mx_setconf (usbd_device *dev, uint8_t cfg)
{
  switch (cfg) {
  case 0:
    /* deconfiguring device */
    usbd_ep_deconfig (dev, MX_CON_IN_EP);
    usbd_ep_deconfig (dev, MX_CON_OUT_EP);
    usbd_reg_endpoint (dev, MX_CON_IN_EP, 0);
    usbd_reg_endpoint (dev, MX_CON_OUT_EP, 0);
    usbd_ep_deconfig (dev, MX_DATA_OUT_EP);
    usbd_ep_deconfig (dev, MX_DATA_IN_EP);
    usbd_reg_endpoint (dev, MX_DATA_IN_EP, 0);
    usbd_reg_endpoint (dev, MX_DATA_OUT_EP, 0);
    return usbd_ack;
  case 1:
    /* configuring device */
    usbd_ep_config (dev, MX_CON_IN_EP, USB_EPTYPE_BULK, MX_DATA_SZ);
    usbd_ep_config (dev, MX_CON_OUT_EP, USB_EPTYPE_BULK, MX_DATA_SZ);
    usbd_reg_endpoint (dev, MX_CON_IN_EP, mx_con_in);
    usbd_reg_endpoint (dev, MX_CON_OUT_EP, mx_con_out);
    usbd_ep_write (dev, MX_CON_OUT_EP, 0, 0);

    usbd_ep_config (dev, MX_DATA_IN_EP, USB_EPTYPE_BULK, MX_DATA_SZ);
    usbd_ep_config (dev, MX_DATA_OUT_EP, USB_EPTYPE_BULK, MX_DATA_SZ);
    usbd_reg_endpoint (dev, MX_DATA_IN_EP, mx_data_in);
    usbd_reg_endpoint (dev, MX_DATA_OUT_EP, mx_data_out);
    usbd_ep_write (dev, MX_DATA_OUT_EP, 0, 0);
    return usbd_ack;
  default:
    return usbd_fail;
  }
}

#define MX_USE_IRQ  1

#if (MX_USE_IRQ)
  #if defined(USBD_PRIMARY_OTGHS)
    #define USB_HANDLER     OTG_HS_IRQHandler
    #define USB_NVIC_IRQ    OTG_HS_IRQn
    /* WA. With __WFI/__WFE interrupt will not be fired
     * faced with F4 series and OTGHS only
     */
    #undef  __WFI
    #define __WFI __NOP
  #else
    #define USB_HANDLER     OTG_FS_IRQHandler
    #define USB_NVIC_IRQ    OTG_FS_IRQn
  #endif

  void USB_HANDLER (void)
  {
    usbd_poll (&m_usb_dev);
  }

#endif

void usb_init (void)
{
  m_usb_critical_nesting = 0;

  rb_init (&m_usb_con_rx_rb, m_usb_con_rx_buf, sizeof(m_usb_con_rx_buf));
  rb_init (&m_usb_con_tx_rb, m_usb_con_tx_buf, sizeof(m_usb_con_tx_buf));
  usb_buf_init ();

  usbd_init (&m_usb_dev, &usbd_hw, MX_EP0_SIZE, m_ubuf, sizeof (m_ubuf));
  usbd_reg_config (&m_usb_dev, mx_setconf);
  usbd_reg_control (&m_usb_dev, mx_control);
  usbd_reg_descr (&m_usb_dev, mx_getdesc);
  usbd_reg_event (&m_usb_dev, usbd_evt_reset, mx_reset);
  usbd_reg_event (&m_usb_dev, usbd_evt_sof, mx_sof);

#if (MX_USE_IRQ)
  SET_USER_VECT (USB_NVIC_IRQ, USB_HANDLER);
  NVIC_EnableIRQ (USB_NVIC_IRQ);
  NVIC_SetPriority (USB_NVIC_IRQ, 12);
#endif
}

void usb_poll (void)
{
  usbd_poll (&m_usb_dev);
}

void usb_start (void)
{
	usbd_enable  (&m_usb_dev, true);
	usbd_connect (&m_usb_dev, true);
}

void usb_deinit (void)
{
	usbd_connect (&m_usb_dev, false);
	usbd_enable  (&m_usb_dev, false);
	NVIC_DisableIRQ (OTG_FS_IRQn);
}

bool is_usb_connected ()
{
  return m_usb_connected;
}

bool is_usb_console_ready ()
{
  return m_usb_console_ready;
}

bool usb_haschar ()
{
  return ! rb_is_empty(&m_usb_con_rx_rb);
}

int usb_getchar()
{
  uint8_t data;
  if (!usb_haschar ()) {
    return -1;
  }
  if (rb_get (&m_usb_con_rx_rb, &data)) {
    return data;
  } else {
    return 0;
  }
}

void usb_putchar(int c)
{
  while (usb_isfull ()) delay_us (1);
  rb_put (&m_usb_con_tx_rb, c);
}

void usb_puts (char *s)
{
  while (*s) {
    usb_putchar (*s++);
  }
}

bool usb_isfull ()
{
  return rb_is_full(&m_usb_con_tx_rb);
}
