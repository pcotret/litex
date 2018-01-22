#ifndef __IRQ_H
#define __IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __riscv
// PicoRV32 has a very limited interrupt support, implemented via custom
// instructions. It also doesn't have a global interrupt enable/disable, so
// we have to emulate it via saving and restoring a mask and using 0/~1 as a
// hardware mask.
// Due to all this somewhat low-level mess, all of the glue is implementein
// the RiscV crt0, and this header is kept as a thin wrapper. Since interrupts
// managed by this layer, do not call interrupt instructions directly, as the
// state will go out of sync with the hardware.

// Read only.
extern unsigned int _irq_pending;
// Read only.
extern unsigned int _irq_mask;
// Read only.
extern unsigned int _irq_enabled;
extern void _irq_enable(void);
extern void _irq_disable(void);
extern void _irq_setmask(unsigned int);
#endif

#ifdef __or1k__
#include <system.h>
#endif

static inline unsigned int irq_getie(void)
{
#if defined (__lm32__)
	unsigned int ie;
	__asm__ __volatile__("rcsr %0, IE" : "=r" (ie));
	return ie;
#elif defined (__or1k__)
	return !!(mfspr(SPR_SR) & SPR_SR_IEE);
#elif defined (__riscv)
	return _irq_enabled != 0;
#else
#error Unsupported architecture
#endif
}

static inline void irq_setie(unsigned int ie)
{
#if defined (__lm32__)
	__asm__ __volatile__("wcsr IE, %0" : : "r" (ie));
#elif defined (__or1k__)
	if (ie & 0x1)
		mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_IEE);
	else
		mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_IEE);
#elif defined (__riscv)
    if (ie & 0x1)
        _irq_enable();
    else
        _irq_disable();
#else
#error Unsupported architecture
#endif
}

static inline unsigned int irq_getmask(void)
{
#if defined (__lm32__)
	unsigned int mask;
	__asm__ __volatile__("rcsr %0, IM" : "=r" (mask));
	return mask;
#elif defined (__or1k__)
	return mfspr(SPR_PICMR);
#elif defined (__riscv)
    // PicoRV32 interrupt mask bits are high-disabled. This is the inverse of how
    // LiteX sees things.
    return ~_irq_mask;
#else
#error Unsupported architecture
#endif
}

static inline void irq_setmask(unsigned int mask)
{
#if defined (__lm32__)
	__asm__ __volatile__("wcsr IM, %0" : : "r" (mask));
#elif defined (__or1k__)
	mtspr(SPR_PICMR, mask);
#elif defined (__riscv)
    // PicoRV32 interrupt mask bits are high-disabled. This is the inverse of how
    // LiteX sees things.
    _irq_setmask(~mask);
#else
#error Unsupported architecture
#endif
}

static inline unsigned int irq_pending(void)
{
#if defined (__lm32__)
	unsigned int pending;
	__asm__ __volatile__("rcsr %0, IP" : "=r" (pending));
	return pending;
#elif defined (__or1k__)
	return mfspr(SPR_PICSR);
#elif defined (__riscv)
	return _irq_pending;
#else
#error Unsupported architecture
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* __IRQ_H */
