#include <stdbool.h>
/* Interface between CPU and Interrupt controller.  */
/**
 * armv7m_nvic_set_pending: mark the specified exception as pending
 * @opaque: the NVIC
 * @irq: the exception number to mark pending
 * @secure: false for non-banked exceptions or for the nonsecure
 * version of a banked exception, true for the secure version of a banked
 * exception.
 *
 * Marks the specified exception as pending. Note that we will assert()
 * if @secure is true and @irq does not specify one of the fixed set
 * of architecturally banked exceptions.
 */
void armv7m_nvic_set_pending(void *opaque, int irq, bool secure);
/**
 * armv7m_nvic_set_pending_derived: mark this derived exception as pending
 * @opaque: the NVIC
 * @irq: the exception number to mark pending
 * @secure: false for non-banked exceptions or for the nonsecure
 * version of a banked exception, true for the secure version of a banked
 * exception.
 *
 * Similar to armv7m_nvic_set_pending(), but specifically for derived
 * exceptions (exceptions generated in the course of trying to take
 * a different exception).
 */
void armv7m_nvic_set_pending_derived(void *opaque, int irq, bool secure);
/**
 * armv7m_nvic_get_pending_irq_info: return highest priority pending
 *    exception, and whether it targets Secure state
 * @opaque: the NVIC
 * @pirq: set to pending exception number
 * @ptargets_secure: set to whether pending exception targets Secure
 *
 * This function writes the number of the highest priority pending
 * exception (the one which would be made active by
 * armv7m_nvic_acknowledge_irq()) to @pirq, and sets @ptargets_secure
 * to true if the current highest priority pending exception should
 * be taken to Secure state, false for NS.
 */
void armv7m_nvic_get_pending_irq_info(void *opaque, int *pirq,
                                      bool *ptargets_secure);
/**
 * armv7m_nvic_acknowledge_irq: make highest priority pending exception active
 * @opaque: the NVIC
 *
 * Move the current highest priority pending exception from the pending
 * state to the active state, and update v7m.exception to indicate that
 * it is the exception currently being handled.
 */
void armv7m_nvic_acknowledge_irq(void *opaque);
/**
 * armv7m_nvic_complete_irq: complete specified interrupt or exception
 * @opaque: the NVIC
 * @irq: the exception number to complete
 * @secure: true if this exception was secure
 *
 * Returns: -1 if the irq was not active
 *           1 if completing this irq brought us back to base (no active irqs)
 *           0 if there is still an irq active after this one was completed
 * (Ignoring -1, this is the same as the RETTOBASE value before completion.)
 */
int armv7m_nvic_complete_irq(void *opaque, int irq, bool secure);

/* Interface between CPU and Interrupt controller.  */
#ifndef CONFIG_USER_ONLY
bool armv7m_nvic_can_take_pending_exception(void *opaque);
#else
static inline bool armv7m_nvic_can_take_pending_exception(void *opaque)
{
    return true;
}
#endif
