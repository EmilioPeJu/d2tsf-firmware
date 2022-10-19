#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H
#include <stdbool.h>
#include <stdint.h>
#include "peripherals.h"

#define RING_BUFFER_SIZE_SHIFT 10
#define RING_BUFFER_SIZE_MASK ((1 << RING_BUFFER_SIZE_SHIFT) - 1)


struct ring_buffer {
    uint16_t head;
    uint16_t tail;
    uint32_t buffer_overruns;
    uint32_t uart_overruns;
    uint8_t data[1 << RING_BUFFER_SIZE_SHIFT];
};


static inline bool rb_is_empty(struct ring_buffer *rb)
{
    return rb->head == rb->tail;
}


static inline void _rb_put(struct ring_buffer *rb, uint8_t item)
{
    rb->data[rb->head] = item;
    rb->head = (rb->head + 1) & RING_BUFFER_SIZE_MASK;
}


static inline uint8_t _rb_get(struct ring_buffer *rb)
{
    uint8_t val = rb->data[rb->tail];
    rb->tail = (rb->tail + 1) & RING_BUFFER_SIZE_MASK;
    return val;
}


static inline void rb_put_isr(struct ring_buffer *rb, uint8_t item)
{
    _rb_put(rb, item);
}


static inline uint8_t rb_get_isr(struct ring_buffer *rb)
{
    return _rb_get(rb);
}


static inline void rb_put(struct ring_buffer *rb, uint8_t item)
{
    __disable_irq();
    _rb_put(rb, item);
    __enable_irq();
}


static inline uint8_t rb_get(struct ring_buffer *rb)
{
    __disable_irq();
    uint8_t val = _rb_get(rb);
    __enable_irq();
    return val;
}


#endif
