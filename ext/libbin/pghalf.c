// Branch-free implementation of half-precision (16 bit) floating point
// Copyright 2006 Mike Acton <macton@gmail.com>
// Copyright 2019 Brice Videau <brice.videau@gmail.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE
//
// PlatinumGames Half-precision floating point format
// ------------------------------------
//
//   | Field    | Last | First | Note
//   |----------|------|-------|----------
//   | Sign     | 15   | 15    |
//   | Exponent | 14   | 9     | Bias = 47
//   | Mantissa | 8    | 0     |
//
// Compiling
// ---------
//
//  Preferred compile flags for GCC: 
//     -O3 -fstrict-aliasing -std=c99 -pedantic -Wall -Wstrict-aliasing
//
//     This file is a C99 source file, intended to be compiled with a C99 
//     compliant compiler. However, for the moment it remains combatible
//     with C++98. Therefore if you are using a compiler that poorly implements
//     C standards (e.g. MSVC), it may be compiled as C++. This is not
//     guaranteed for future versions. 
//

#include "pghalf.h"

// Load immediate
static inline uint32_t _uint32_li( uint32_t a )
{
  return (a);
}

// Decrement
static inline uint32_t _uint32_dec( uint32_t a )
{
  return (a - 1);
}

// Increment
static inline uint32_t _uint32_inc( uint32_t a )
{
  return (a + 1);
}

// Complement
static inline uint32_t _uint32_not( uint32_t a )
{
  return (~a);
}

// Negate
static inline uint32_t _uint32_neg( uint32_t a )
{
  return (-a);
}

// Extend sign
static inline uint32_t _uint32_ext( uint32_t a )
{
  return (((int32_t)a)>>31);
}

// And
static inline uint32_t _uint32_and( uint32_t a, uint32_t b )
{
  return (a & b);
}

// Exclusive Or
static inline uint32_t _uint32_xor( uint32_t a, uint32_t b )
{
  return (a ^ b);
}

// And with Complement
static inline uint32_t _uint32_andc( uint32_t a, uint32_t b )
{
  return (a & ~b);
}

// Or
static inline uint32_t _uint32_or( uint32_t a, uint32_t b )
{
  return (a | b);
}

// Shift Right Logical
static inline uint32_t _uint32_srl( uint32_t a, int sa )
{
  return (a >> sa);
}

// Shift Left Logical
static inline uint32_t _uint32_sll( uint32_t a, int sa )
{
  return (a << sa);
}

// Add
static inline uint32_t _uint32_add( uint32_t a, uint32_t b )
{
  return (a + b);
}

// Subtract
static inline uint32_t _uint32_sub( uint32_t a, uint32_t b )
{
  return (a - b);
}

// Multiply
static inline uint32_t _uint32_mul( uint32_t a, uint32_t b )
{
  return (a * b);
}

// Select on Sign bit
static inline uint32_t _uint32_sels( uint32_t test, uint32_t a, uint32_t b )
{
  const uint32_t mask   = _uint32_ext( test );
  const uint32_t sel_a  = _uint32_and(  a,     mask  );
  const uint32_t sel_b  = _uint32_andc( b,     mask  );
  const uint32_t result = _uint32_or(   sel_a, sel_b );

  return (result);
}

// Select Bits on mask
static inline uint32_t _uint32_selb( uint32_t mask, uint32_t a, uint32_t b )
{
  const uint32_t sel_a  = _uint32_and(  a,     mask  );
  const uint32_t sel_b  = _uint32_andc( b,     mask  );
  const uint32_t result = _uint32_or(   sel_a, sel_b );

  return (result);
}

// Load Immediate
static inline uint16_t _uint16_li( uint16_t a )
{
  return (a);
}

// Extend sign
static inline uint16_t _uint16_ext( uint16_t a )
{
  return (((int16_t)a)>>15);
}

// Negate
static inline uint16_t _uint16_neg( uint16_t a )
{
  return (-a);
}

// Complement
static inline uint16_t _uint16_not( uint16_t a )
{
  return (~a);
}

// Decrement
static inline uint16_t _uint16_dec( uint16_t a )
{
  return (a - 1);
}

// Shift Left Logical
static inline uint16_t _uint16_sll( uint16_t a, int sa )
{
  return (a << sa);
}

// Shift Right Logical
static inline uint16_t _uint16_srl( uint16_t a, int sa )
{
  return (a >> sa);
}

// Add
static inline uint16_t _uint16_add( uint16_t a, uint16_t b )
{
  return (a + b);
}

// Subtract
static inline uint16_t _uint16_sub( uint16_t a, uint16_t b )
{
  return (a - b);
}

// And
static inline uint16_t _uint16_and( uint16_t a, uint16_t b )
{
  return (a & b);
}

// Or
static inline uint16_t _uint16_or( uint16_t a, uint16_t b )
{
  return (a | b);
}

// Exclusive Or
static inline uint16_t _uint16_xor( uint16_t a, uint16_t b )
{
  return (a ^ b);
}

// And with Complement
static inline uint16_t _uint16_andc( uint16_t a, uint16_t b )
{
  return (a & ~b);
}

// And then Shift Right Logical
static inline uint16_t _uint16_andsrl( uint16_t a, uint16_t b, int sa )
{
  return ((a & b) >> sa);
}

// Shift Right Logical then Mask
static inline uint16_t _uint16_srlm( uint16_t a, int sa, uint16_t mask )
{
  return ((a >> sa) & mask);
}

// Add then Mask
static inline uint16_t _uint16_addm( uint16_t a, uint16_t b, uint16_t mask )
{
  return ((a + b) & mask);
}


// Select on Sign bit
static inline uint16_t _uint16_sels( uint16_t test, uint16_t a, uint16_t b )
{
  const uint16_t mask   = _uint16_ext( test );
  const uint16_t sel_a  = _uint16_and(  a,     mask  );
  const uint16_t sel_b  = _uint16_andc( b,     mask  );
  const uint16_t result = _uint16_or(   sel_a, sel_b );

  return (result);
}

// Count Leading Zeros
static inline uint32_t _uint32_cntlz( uint32_t x )
{
#ifdef __GNUC__
  /* NOTE: __builtin_clz is undefined for x == 0 */
  /* On PowerPC, this will map to insn: cntlzw   */
  /* On Pentium, this will map to insn: clz      */
  uint32_t is_x_nez_msb = _uint32_neg( x );
  uint32_t nlz          = __builtin_clz( x );
  uint32_t result       = _uint32_sels( is_x_nez_msb, nlz, 0x00000020 );
  return (result);
#else
  const uint32_t x0  = _uint32_srl(  x,  1 );
  const uint32_t x1  = _uint32_or(   x,  x0 );
  const uint32_t x2  = _uint32_srl(  x1, 2 );
  const uint32_t x3  = _uint32_or(   x1, x2 );
  const uint32_t x4  = _uint32_srl(  x3, 4 );
  const uint32_t x5  = _uint32_or(   x3, x4 );
  const uint32_t x6  = _uint32_srl(  x5, 8 );
  const uint32_t x7  = _uint32_or(   x5, x6 );
  const uint32_t x8  = _uint32_srl(  x7, 16 );
  const uint32_t x9  = _uint32_or(   x7, x8 );
  const uint32_t xA  = _uint32_not(  x9 );
  const uint32_t xB  = _uint32_srl(  xA, 1 );
  const uint32_t xC  = _uint32_and(  xB, 0x55555555 );
  const uint32_t xD  = _uint32_sub(  xA, xC );
  const uint32_t xE  = _uint32_and(  xD, 0x33333333 );
  const uint32_t xF  = _uint32_srl(  xD, 2 );
  const uint32_t x10 = _uint32_and(  xF, 0x33333333 );
  const uint32_t x11 = _uint32_add(  xE, x10 );
  const uint32_t x12 = _uint32_srl(  x11, 4 );
  const uint32_t x13 = _uint32_add(  x11, x12 );
  const uint32_t x14 = _uint32_and(  x13, 0x0f0f0f0f );
  const uint32_t x15 = _uint32_srl(  x14, 8 );
  const uint32_t x16 = _uint32_add(  x14, x15 );
  const uint32_t x17 = _uint32_srl(  x16, 16 );
  const uint32_t x18 = _uint32_add(  x16, x17 );
  const uint32_t x19 = _uint32_and(  x18, 0x0000003f );
  return ( x19 );
#endif
}

// Count Leading Zeros
static inline uint16_t _uint16_cntlz( uint16_t x )
{
#ifdef __GNUC__
  uint16_t nlz32 = (uint16_t)_uint32_cntlz( (uint32_t)x );
  uint32_t nlz   = _uint32_sub( nlz32, 16 );
  return (nlz);
#else
  const uint16_t x0  = _uint16_srl(  x,  1 );
  const uint16_t x1  = _uint16_or(   x,  x0 );
  const uint16_t x2  = _uint16_srl(  x1, 2 );
  const uint16_t x3  = _uint16_or(   x1, x2 );
  const uint16_t x4  = _uint16_srl(  x3, 4 );
  const uint16_t x5  = _uint16_or(   x3, x4 );
  const uint16_t x6  = _uint16_srl(  x5, 8 );
  const uint16_t x7  = _uint16_or(   x5, x6 );
  const uint16_t x8  = _uint16_not(  x7 );
  const uint16_t x9  = _uint16_srlm( x8, 1, 0x5555 );
  const uint16_t xA  = _uint16_sub(  x8, x9 );
  const uint16_t xB  = _uint16_and(  xA, 0x3333 );
  const uint16_t xC  = _uint16_srlm( xA, 2, 0x3333 );
  const uint16_t xD  = _uint16_add(  xB, xC );
  const uint16_t xE  = _uint16_srl(  xD, 4 );
  const uint16_t xF  = _uint16_addm( xD, xE, 0x0f0f );
  const uint16_t x10 = _uint16_srl(  xF, 8 );
  const uint16_t x11 = _uint16_addm( xF, x10, 0x001f );
  return ( x11 );
#endif
}

uint16_t
pghalf_from_float( uint32_t f )
{
  const uint32_t one                        = _uint32_li( 0x00000001 );
  const uint32_t f_s_mask                   = _uint32_li( 0x80000000 ); //bit 31
  const uint32_t f_e_mask                   = _uint32_li( 0x7f800000 ); //bits 30-23
  const uint32_t f_m_mask                   = _uint32_li( 0x007fffff ); //bits 22-0
  const uint32_t f_m_hidden_bit             = _uint32_li( 0x00800000 ); //1<<f_e_pos
  const uint32_t f_m_round_bit              = _uint32_li( 0x00002000 ); //1<<(f_e_pos - h_e_pos - 1)
  const uint32_t f_snan_mask                = _uint32_li( 0x7fc00000 ); //f_e_mask + 1 << (f_e_pos - 1)
  const uint32_t f_e_pos                    = _uint32_li( 0x00000017 ); //23
  const uint32_t h_e_pos                    = _uint32_li( 0x00000009 ); //9
  const uint32_t h_e_mask                   = _uint32_li( 0x00007e00 ); //bits 14-9
  const uint32_t h_snan_mask                = _uint32_li( 0x00007f00 ); //h_e_mask + 1 << (h_e_pos - 1)
  const uint32_t h_e_mask_value             = _uint32_li( 0x0000003f ); //h_e_mask >> 9
  const uint32_t f_h_s_pos_offset           = _uint32_li( 0x00000010 ); //f_s_pos - h_s_pos
  const uint32_t f_h_bias_offset            = _uint32_li( 0x00000050 ); //f_bias - h_bias
  const uint32_t f_h_m_pos_offset           = _uint32_li( 0x0000000e ); //f_e_pos - h_e_pos
  const uint32_t h_nan_min                  = _uint32_li( 0x00007e01 ); //h_e_mask + 1
  const uint32_t f_h_e_biased_flag          = _uint32_li( 0x000000af ); //f_bias + h_bias + 1
  const uint32_t f_s                        = _uint32_and( f,               f_s_mask         );
  const uint32_t f_e                        = _uint32_and( f,               f_e_mask         );
  const uint16_t h_s                        = _uint32_srl( f_s,             f_h_s_pos_offset );
  const uint32_t f_m                        = _uint32_and( f,               f_m_mask         );
  const uint16_t f_e_amount                 = _uint32_srl( f_e,             f_e_pos          );
  const uint32_t f_e_half_bias              = _uint32_sub( f_e_amount,      f_h_bias_offset  );
  const uint32_t f_snan                     = _uint32_and( f,               f_snan_mask      );
  const uint32_t f_m_round_mask             = _uint32_and( f_m,             f_m_round_bit    );
  const uint32_t f_m_round_offset           = _uint32_sll( f_m_round_mask,  one              );
  const uint32_t f_m_rounded                = _uint32_add( f_m,             f_m_round_offset );
  const uint32_t f_m_denorm_sa              = _uint32_sub( one,             f_e_half_bias    );
  const uint32_t f_m_with_hidden            = _uint32_or(  f_m_rounded,     f_m_hidden_bit   );
  const uint32_t f_m_denorm                 = _uint32_srl( f_m_with_hidden, f_m_denorm_sa    );
  const uint32_t h_m_denorm                 = _uint32_srl( f_m_denorm,      f_h_m_pos_offset );
  const uint32_t f_m_rounded_overflow       = _uint32_and( f_m_rounded,     f_m_hidden_bit   );
  const uint32_t m_nan                      = _uint32_srl( f_m,             f_h_m_pos_offset );
  const uint32_t h_em_nan                   = _uint32_or(  h_e_mask,        m_nan            );
  const uint32_t h_e_norm_overflow_offset   = _uint32_inc( f_e_half_bias );
  const uint32_t h_e_norm_overflow          = _uint32_sll( h_e_norm_overflow_offset, h_e_pos          );
  const uint32_t h_e_norm                   = _uint32_sll( f_e_half_bias,            h_e_pos          );
  const uint32_t h_m_norm                   = _uint32_srl( f_m_rounded,              f_h_m_pos_offset );
  const uint32_t h_em_norm                  = _uint32_or(  h_e_norm,                 h_m_norm         );
  const uint32_t is_h_ndenorm_msb           = _uint32_sub( f_h_bias_offset,   f_e_amount    );
  const uint32_t is_f_e_flagged_msb         = _uint32_sub( f_h_e_biased_flag, f_e_half_bias );
  const uint32_t is_h_denorm_msb            = _uint32_not( is_h_ndenorm_msb );
  const uint32_t is_f_m_eqz_msb             = _uint32_dec( f_m   );
  const uint32_t is_h_nan_eqz_msb           = _uint32_dec( m_nan );
  const uint32_t is_f_inf_msb               = _uint32_and( is_f_e_flagged_msb, is_f_m_eqz_msb   );
  const uint32_t is_f_nan_underflow_msb     = _uint32_and( is_f_e_flagged_msb, is_h_nan_eqz_msb );
  const uint32_t is_e_overflow_msb          = _uint32_sub( h_e_mask_value,     f_e_half_bias    );
  const uint32_t is_h_inf_msb               = _uint32_or(  is_e_overflow_msb,  is_f_inf_msb     );
  const uint32_t is_f_nsnan_msb             = _uint32_sub( f_snan,             f_snan_mask      );
  const uint32_t is_m_norm_overflow_msb     = _uint32_neg( f_m_rounded_overflow );
  const uint32_t is_f_snan_msb              = _uint32_not( is_f_nsnan_msb );
  const uint32_t h_em_overflow_result       = _uint32_sels( is_m_norm_overflow_msb, h_e_norm_overflow, h_em_norm                 );
  const uint32_t h_em_nan_result            = _uint32_sels( is_f_e_flagged_msb,     h_em_nan,          h_em_overflow_result      );
  const uint32_t h_em_nan_underflow_result  = _uint32_sels( is_f_nan_underflow_msb, h_nan_min,         h_em_nan_result           );
  const uint32_t h_em_inf_result            = _uint32_sels( is_h_inf_msb,           h_e_mask,          h_em_nan_underflow_result );
  const uint32_t h_em_denorm_result         = _uint32_sels( is_h_denorm_msb,        h_m_denorm,        h_em_inf_result           );
  const uint32_t h_em_snan_result           = _uint32_sels( is_f_snan_msb,          h_snan_mask,       h_em_denorm_result        );
  const uint32_t h_result                   = _uint32_or( h_s, h_em_snan_result );

  return (uint16_t)(h_result);
}

uint32_t 
pghalf_to_float( uint16_t h )
{
  const uint32_t h_e_mask              = _uint32_li( 0x00007e00 ); //bits 14-9
  const uint32_t h_m_mask              = _uint32_li( 0x000001ff ); //bits 8-0
  const uint32_t h_s_mask              = _uint32_li( 0x00008000 ); //bit 15
  const uint32_t h_f_s_pos_offset      = _uint32_li( 0x00000010 ); //f_s_pos - h_s_pos
  const uint32_t h_f_e_pos_offset      = _uint32_li( 0x0000000e ); //f_m_bitcount - h_m_bitcount
  const uint32_t h_f_bias_offset       = _uint32_li( 0x0000a000 ); //(f_bias - h_bias) << 9
  const uint32_t f_e_mask              = _uint32_li( 0x7f800000 ); //bits 30-23
  const uint32_t f_m_mask              = _uint32_li( 0x007fffff ); //bits 22-0
  const uint32_t h_f_e_denorm_bias     = _uint32_li( 0x0000005f ); //h_f_e_pos_offset + 1 + (f_bias - h_bias)
  const uint32_t h_f_m_denorm_sa_bias  = _uint32_li( 0x00000008 ); //float exp bit count
  const uint32_t f_e_pos               = _uint32_li( 0x00000017 ); //23
  const uint32_t h_e_mask_minus_one    = _uint32_li( 0x00007dff ); //h_e_mask + h_m_mask - 1<<h_e_pos
  const uint32_t h_e                   = _uint32_and( h, h_e_mask );
  const uint32_t h_m                   = _uint32_and( h, h_m_mask );
  const uint32_t h_s                   = _uint32_and( h, h_s_mask );
  const uint32_t h_e_f_bias            = _uint32_add( h_e, h_f_bias_offset );
  const uint32_t h_m_nlz               = _uint32_cntlz( h_m );
  const uint32_t f_s                   = _uint32_sll( h_s,        h_f_s_pos_offset );
  const uint32_t f_e                   = _uint32_sll( h_e_f_bias, h_f_e_pos_offset );
  const uint32_t f_m                   = _uint32_sll( h_m,        h_f_e_pos_offset );
  const uint32_t f_em                  = _uint32_or(  f_e,        f_m              );
  const uint32_t h_f_m_sa              = _uint32_sub( h_m_nlz,             h_f_m_denorm_sa_bias );
  const uint32_t f_e_denorm_unpacked   = _uint32_sub( h_f_e_denorm_bias,   h_f_m_sa             );
  const uint32_t h_f_m                 = _uint32_sll( h_m,                 h_f_m_sa             );
  const uint32_t f_m_denorm            = _uint32_and( h_f_m,               f_m_mask             );
  const uint32_t f_e_denorm            = _uint32_sll( f_e_denorm_unpacked, f_e_pos              );
  const uint32_t f_em_denorm           = _uint32_or(  f_e_denorm,          f_m_denorm           );
  const uint32_t f_em_nan              = _uint32_or(  f_e_mask,            f_m                  );
  const uint32_t is_e_eqz_msb          = _uint32_dec(  h_e );
  const uint32_t is_m_nez_msb          = _uint32_neg(  h_m );
  const uint32_t is_e_flagged_msb      = _uint32_sub(  h_e_mask_minus_one, h_e );
  const uint32_t is_zero_msb           = _uint32_andc( is_e_eqz_msb,       is_m_nez_msb );
  const uint32_t is_inf_msb            = _uint32_andc( is_e_flagged_msb,   is_m_nez_msb );
  const uint32_t is_denorm_msb         = _uint32_and(  is_m_nez_msb,       is_e_eqz_msb );
  const uint32_t is_nan_msb            = _uint32_and(  is_e_flagged_msb,   is_m_nez_msb ); 
  const uint32_t is_zero               = _uint32_ext(  is_zero_msb );
  const uint32_t f_zero_result         = _uint32_andc( f_em, is_zero );
  const uint32_t f_denorm_result       = _uint32_sels( is_denorm_msb, f_em_denorm, f_zero_result );
  const uint32_t f_inf_result          = _uint32_sels( is_inf_msb,    f_e_mask,    f_denorm_result );
  const uint32_t f_nan_result          = _uint32_sels( is_nan_msb,    f_em_nan,    f_inf_result    );
  const uint32_t f_result              = _uint32_or( f_s, f_nan_result );
 
  return (f_result);
}
