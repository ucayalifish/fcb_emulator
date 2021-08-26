#ifndef FCB_EMULATOR_MEMBUF_H
#define FCB_EMULATOR_MEMBUF_H

#include <stddef.h>
#include <stdint.h>

void membuf_reset(void);

size_t membuf_bytes_available(void);

uint8_t * membuf_current_position(void);

int membuf_skip_bytes(ptrdiff_t how_many);

void membuf_rewind(void);

#endif //FCB_EMULATOR_MEMBUF_H
