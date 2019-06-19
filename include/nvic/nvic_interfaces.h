#include <stdbool.h>
/* Interface between CPU and Interrupt controller.  */
void armv7m_nvic_set_pending(void *opaque, int irq, bool secure);
void armv7m_nvic_acknowledge_irq(void *opaque);
int armv7m_nvic_complete_irq(void *opaque, int irq, bool secure);
void armv7m_nvic_get_pending_irq_info(void *opaque, int *pirq,
                                      bool *ptargets_secure);
