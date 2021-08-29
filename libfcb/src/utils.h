#ifndef FCB_EMULATOR_UTILS_H
#define FCB_EMULATOR_UTILS_H

#include <stddef.h>
#include <stdint.h>

uint32_t crc32_nand(const uint8_t * block, size_t len, uint32_t carry);

#endif //FCB_EMULATOR_UTILS_H
