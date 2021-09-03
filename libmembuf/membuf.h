#ifndef FCB_EMULATOR_MEMBUF_H
#define FCB_EMULATOR_MEMBUF_H

#include <stddef.h>
#include <stdint.h>

void membuf_reset(void);

size_t membuf_bytes_available(void);

uint8_t * membuf_current_position(void);

ptrdiff_t membuf_skip_bytes(size_t how_many);

ptrdiff_t membuf_rewind(ptrdiff_t to);

ptrdiff_t membuf_write_bytes(uint8_t const * data, size_t sz);

ptrdiff_t membuf_read_bytes(uint8_t * dest, size_t how_many);

void membuf_shuffle_buffer(void);

#endif //FCB_EMULATOR_MEMBUF_H
