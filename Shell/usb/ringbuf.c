#include <common.h>
#include <stm32.h>
#include <ringbuf.h>
#include "usbmcu.h"

extern   uint32_t m_usb_critical_nesting;

void rb_init (ringbuf_t* buffer,	uint8_t* const data_ptr,	const uint16_t size)
{
	USB_CRITICAL_SECTION (
		buffer->count  = 0;
		buffer->in     = data_ptr;
		buffer->out    = data_ptr;
		buffer->start  = &data_ptr[0];
		buffer->end    = &data_ptr[size];
		buffer->size   = size;
	)
}

void rb_flush(ringbuf_t* const buffer)
{
	USB_CRITICAL_SECTION (
		buffer->count	= 0;
		buffer->in		= buffer->start;
		buffer->out		= buffer->start;
	)
}

uint16_t rb_get_count(ringbuf_t* const buffer)
{
	uint16_t count;

	USB_CRITICAL_SECTION (
		count = buffer->count;
	)
	return count;
}

uint16_t rb_get_free_count(ringbuf_t* const buffer)
{
	uint16_t  count;

	USB_CRITICAL_SECTION (
	  count = buffer->size - buffer->count;
	)
	return count;
}

uint8_t rb_is_empty(ringbuf_t* const buffer)
{
	uint8_t empty;

	USB_CRITICAL_SECTION (
		empty = (buffer->count == 0);
	)
	return empty;
}

uint8_t rb_is_full(ringbuf_t* const buffer)
{
	uint8_t full;

	USB_CRITICAL_SECTION (
		full = (buffer->count == buffer->size);
	)
	return full;
}

uint16_t rb_put (ringbuf_t* buffer, const uint8_t data)
{
	uint16_t  count;

	count = 0;
	USB_CRITICAL_SECTION (
		if (buffer->count < buffer->size) {
			*buffer->in = data;
			if (++buffer->in == buffer->end)
				buffer->in = buffer->start;
			buffer->count++;
			count++;
		}
	)
	return count;
}

uint16_t  rb_puts (ringbuf_t* buffer, uint8_t *data, uint16_t dlen)
{
	uint16_t  count;

	count = 0;
	USB_CRITICAL_SECTION (
		while (dlen && (buffer->count < buffer->size)) {
			*buffer->in++ = *data++;
			if (buffer->in == buffer->end)
				buffer->in = buffer->start;
			buffer->count++;
			dlen--;
			count++;
		}
	)
	return count;
}

uint16_t rb_get (ringbuf_t* buffer, uint8_t *data)
{
	uint16_t  count;

	count = 0;
	USB_CRITICAL_SECTION (
		if (buffer->count > 0) {
			*data = *buffer->out;
			if (++buffer->out == buffer->end)
				buffer->out = buffer->start;
			buffer->count--;
			count++;
		}
	)

	return count;
}

uint16_t rb_gets (ringbuf_t* buffer, uint8_t *data, uint16_t dlen)
{
	uint16_t  count;

	count = 0;
	USB_CRITICAL_SECTION (

		while (dlen && (buffer->count > 0)) {
			*data++ = *buffer->out++;;
			if (buffer->out == buffer->end)
				buffer->out = buffer->start;
			buffer->count--;
			dlen--;
			count++;
		}
	)
	return count;
}
