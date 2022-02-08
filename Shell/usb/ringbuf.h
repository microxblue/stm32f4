#ifndef _RING_BUF_H_
#define _RING_BUF_H_

#include <stdint.h>

typedef struct
{
	volatile uint16_t count;	/**< Number of bytes currently stored in the buffer. */
	volatile uint8_t * in;		/**< Current storage location in the circular buffer. */
	volatile uint8_t * out;		/**< Current retrieval location in the circular buffer. */
	uint8_t* start;				/**< Pointer to the start of the buffer's underlying storage array. */
	uint8_t* end;				/**< Pointer to the end of the buffer's underlying storage array. */
	uint16_t size;				/**< Size of the buffer's underlying storage array. */
} ringbuf_t;

void     rb_init (ringbuf_t* buffer,	uint8_t* const data_ptr,	const uint16_t size);
void     rb_flush(ringbuf_t* const buffer);
uint16_t rb_get_count(ringbuf_t* const buffer);
uint16_t rb_get_free_count(ringbuf_t* const buffer);
uint8_t  rb_is_empty(ringbuf_t* const buffer);
uint8_t  rb_is_full(ringbuf_t* const buffer);
uint16_t rb_put (ringbuf_t* buffer, const uint8_t data);
uint16_t rb_get (ringbuf_t* buffer, uint8_t *data);
uint16_t rb_puts (ringbuf_t* buffer, uint8_t *data, uint16_t dlen);
uint16_t rb_gets (ringbuf_t* buffer, uint8_t *data, uint16_t dlen);
#endif
