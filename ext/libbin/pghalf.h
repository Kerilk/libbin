#ifndef PGHALF_H
#define PGHALF_H

#include <stdint.h>

uint32_t pghalf_to_float( uint16_t h );
uint16_t pghalf_from_float( uint32_t f );

#endif /* PGHALF_H */
