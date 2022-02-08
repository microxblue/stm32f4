#ifndef _USB_MCU_H_
#define _USB_MCU_H_

#define  LD_CCMRAM

#define  USB_CRITICAL_SECTION(x)  { \
                             NVIC_DisableIRQ (OTG_FS_IRQn); \
                             m_usb_critical_nesting++; \
                             x; \
                             m_usb_critical_nesting--; \
                             if (m_usb_critical_nesting == 0 ) NVIC_EnableIRQ (OTG_FS_IRQn); \
                           }

#define  SET_VECT(irq, handler)  { \
                           __disable_irq ();  \
                           ((uint32_t *)SCB->VTOR)[irq] =  (long unsigned int)(handler); \
                           __enable_irq (); }

#define  GET_VECT(irq)     ((uint32_t *)SCB->VTOR)[irq]

#define  SET_USER_VECT(irq, handler)   SET_VECT((irq + 16), handler)

#endif
