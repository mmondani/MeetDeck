#ifndef BASICDEFINITIONS_H_
#define BASICDEFINITIONS_H_

#include <stdint.h>

/******************  Manejo de bit  *********************/
#define bit_test(var, bit) ((var) & (1 <<(bit)))
#define bit_set(var, bit) ((var) |= (1 << (bit)))
#define bit_clear(var, bit) ((var) &= ~(1 << (bit)))
#define nibble_swap(var) (( var >> 4 ) | ( var << 4 ))
#define bcd2dec(var) (((var&0xF0) >> 4) * 10 + (var&0x0F))


/******************  Definitions  *********************/
// Variables
#define         u8                          uint8_t
#define         i8                          int8_t
#define         u16                         uint16_t
#define         i16                         int16_t
#define         u32                         uint32_t
#define         i32                         int32_t


#define         BIT0                        (0x01 << 0)
#define         BIT1                        (0x01 << 1)
#define         BIT2                        (0x01 << 2)
#define         BIT3                        (0x01 << 3)
#define         BIT4                        (0x01 << 4)
#define         BIT5                        (0x01 << 5)
#define         BIT6                        (0x01 << 6)
#define         BIT7                        (0x01 << 7)

/* TECLADO NUMERICO */
#define         TECLA_0                     0x00
#define         TECLA_1                     0x01
#define         TECLA_2                     0x02
#define         TECLA_3                     0x03
#define         TECLA_4                     0x04
#define         TECLA_5                     0x05
#define         TECLA_6                     0x06
#define         TECLA_7                     0x07
#define         TECLA_8                     0x08
#define         TECLA_9                     0x09

/* TECLADO TELEFONICO */
#define         TECLA_A                     0x02
#define         TECLA_B                     0x02
#define         TECLA_C                     0x02
#define         TECLA_D                     0x03
#define         TECLA_E                     0x03
#define         TECLA_F                     0x03
#define         TECLA_G                     0x04
#define         TECLA_H                     0x04
#define         TECLA_I                     0x04
#define         TECLA_J                     0x05
#define         TECLA_K                     0x05
#define         TECLA_L                     0x05
#define         TECLA_M                     0x06
#define         TECLA_N                     0x06
#define         TECLA_O                     0x06
#define         TECLA_P                     0x07
#define         TECLA_Q                     0x07
#define         TECLA_R                     0x07
#define         TECLA_S                     0x07
#define         TECLA_T                     0x08
#define         TECLA_U                     0x08
#define         TECLA_V                     0x08
#define         TECLA_W                     0x09
#define         TECLA_X                     0x09
#define         TECLA_Y                     0x09
#define         TECLA_Z                     0x09
#define         TECLA_TLCD_P                0x0a
#define         TECLA_TLCD_F                0x0b
#define         TECLA_TLCD_Z                0x0c
#define         TECLA_TLCD_M                0x0d
#define         TECLA_TLCD_PAN              0x0e
#define         TECLA_TLCD_INC              0x0f



// Incrementar una variable hasta un límite
#define         _inch128(reg)               ( (reg == 128) ? reg : reg++ )
#define         _inch64(reg)                ( (reg == 64) ? reg : reg++ )
#define         _inch32(reg)                ( (reg == 32) ? reg : reg++ )
#define         _inch16(reg)                ( (reg == 16) ? reg : reg++ )
#define         _inch8(reg)                 ( (reg == 8) ? reg : reg++ )
#define         _inch4(reg)                 ( (reg == 4) ? reg : reg++ )
#define         _dech0(reg)                 ( (reg == 0) ? reg : reg-- )



#endif /* BASICDEFINITIONS_H_ */