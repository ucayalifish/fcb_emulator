#ifndef FCB_EMULATOR_MEMBUF_H
#define FCB_EMULATOR_MEMBUF_H

#include <stddef.h>
#include <stdint.h>

void membuf_reset(void);

/**
 * Space in buffer, available for writing/
 * @return number of bytes buffer can accommodate in current state.
 */
size_t membuf_bytes_available(void);

/**
 * Converts current position in uint16_t array to byte address.
 * @return address of current byte in buffer.
 */
uint8_t * membuf_current_position(void);

/**
 * While writing reserves space in buffer.
 * @param how_many_bytes number of bytes to reserve.
 * @return current byte position in buffer (positive) of -1
 * if insufficient available space.
 */
ptrdiff_t membuf_skip_bytes(size_t how_many_bytes);

/**
 * Sets current position in buffer.
 * @param to requested position in bytes, must be not negative and even.
 * @return new current position in bytes or -1
 * if current byte position <= requested position.
 */
ptrdiff_t membuf_rewind(ptrdiff_t to);

/**
 * Copies bytes to block buffer and changes current position accordingly.
 * @param data pointer to bytes to be copied, must not be NULL.
 * @param sz number of bytes to be copied, must be positive.
 * @return new byte position or -1 if insufficient available space to accomodate all data.
 */
ptrdiff_t membuf_write_bytes(uint8_t const * data, size_t sz);

/**
 * Copies bytes from current buffer offset to external buffer.
 * @param dest address of external buffer, must not be NULL.
 * @param how_many number of bytes bo be copied.
 * @return new reading offset in bytes of -1 if requested number of bytes exceeds buffer capacity.
 */
ptrdiff_t membuf_read_bytes(uint8_t * dest, size_t how_many);

/**
 * Applies Linear-Feedback Shift Register to buffer.
 */
void membuf_shuffle_buffer(void);

/**
 * Calculates CRC32 for bytes in requested part of buffer
 * @param from start byte offset, must be even.
 * @param to end byte offset, must be even.
 * @return Calculated CRC32.
 */
uint32_t membuf_calc_crc32(ptrdiff_t from, ptrdiff_t to);

#endif //FCB_EMULATOR_MEMBUF_H
