#include <common.h>
#include <stm32.h>
#include "usbmcu.h"
#include "usbdev.h"

extern   uint32_t  m_usb_critical_nesting;

extern USB_RX_BUF  m_usb_rx_buf;
extern USB_TX_BUF  m_usb_tx_buf;

void usb_data_in (void);

void usb_buf_init (void)
{
  uint8_t  i;

  USB_CRITICAL_SECTION (
    for (i = 0; i < MAX_RX_PKT_NUM; i++) {
      m_usb_rx_buf.len[i] = 0xFF;
    }
    m_usb_rx_buf.tail   = 0;
    m_usb_rx_buf.head   = 0;
    m_usb_rx_buf.count  = 0;

    for (i = 0; i < MAX_TX_PKT_NUM; i++) {
      m_usb_tx_buf.len[i] = 0xFF;
    }
    m_usb_tx_buf.tail   = 0;
    m_usb_tx_buf.head   = 0;
    m_usb_tx_buf.count  = 0;
  )
}

uint8_t *usb_get_rx_buf (uint16_t *len)
{
  uint8_t  *buf;

  USB_CRITICAL_SECTION (
    if (m_usb_rx_buf.count) {
      *len = m_usb_rx_buf.len[m_usb_rx_buf.head];
      buf  = m_usb_rx_buf.buf[m_usb_rx_buf.head];
    } else {
      *len = 0;
      buf  = NULL;
    }
  )
  return buf;
}

void usb_free_rx_buf ()
{
  USB_CRITICAL_SECTION (
    if (m_usb_rx_buf.count > 0) {
      m_usb_rx_buf.len[m_usb_rx_buf.head] = 0xFF;
      if (++m_usb_rx_buf.head >= MAX_RX_PKT_NUM) {
        m_usb_rx_buf.head = 0;
      }
      m_usb_rx_buf.count--;
    }
  );
}

uint8_t *usb_get_tx_buf (uint16_t len)
{
  uint8_t  pktnum;
  uint8_t  *ptr;

  pktnum = len ? (len + EP_MAX_SIZE - 1) >> 6 : 1;
  USB_CRITICAL_SECTION (
    if (pktnum <= MAX_TX_PKT_NUM - m_usb_tx_buf.count) {
      // fill data to buffer
      ptr = m_usb_tx_buf.buf[m_usb_tx_buf.tail];
    } else {
      ptr = NULL;
    }
  )
  return ptr;
}

void usb_add_tx_buf (uint16_t len)
{
  uint8_t  pktnum;
  uint8_t  pktlen;

  USB_CRITICAL_SECTION ({
    pktnum = 0;
    do {
      pktlen = (len > EP_MAX_SIZE) ? EP_MAX_SIZE : len;
      len   -= pktlen;
      m_usb_tx_buf.len[m_usb_tx_buf.tail] = pktlen;
      if (++m_usb_tx_buf.tail >= MAX_TX_PKT_NUM) {
        m_usb_tx_buf.tail = 0;
      }
      pktnum++;
    } while (len);
    m_usb_tx_buf.count += pktnum;
  });
}


