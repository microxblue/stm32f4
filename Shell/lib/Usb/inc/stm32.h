#ifndef _STM32_H_
#define _STM32_H_

#include <stm32f407xx.h>

/* modify bitfield */
#define _BMD(reg, msk, val)     (reg) = (((reg) & ~(msk)) | (val))
/* set bitfield */
#define _BST(reg, bits)         (reg) = ((reg) | (bits))
/* clear bitfield */
#define _BCL(reg, bits)         (reg) = ((reg) & ~(bits))
/* wait until bitfield set */
#define _WBS(reg, bits)         while(((reg) & (bits)) == 0)
/* wait until bitfield clear */
#define _WBC(reg, bits)         while(((reg) & (bits)) != 0)
/* wait for bitfield value */
#define _WVL(reg, msk, val)     while(((reg) & (msk)) != (val))
/* bit value */
#define _BV(bit)                (0x01 << (bit))

#ifndef _VAL2FLD
#define _VAL2FLD(field, value)    ((value << field ## _Pos) & field ## _Msk)
#endif

#ifndef _FLD2VAL
#define _FLD2VAL(field, value)    ((value & field ## _Msk) >> field ## _Pos)
#endif

#endif // _STM32_H_