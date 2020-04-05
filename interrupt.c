/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg

/* Interrupt IDs */
#define MPCORE_PRIV_TIMER_IRQ 29
#define INTERVAL_TIMER_IRQ    72
#define PS2_IRQ               79
#define PS2_DUAL_IRQ          89

// Define the IRQ exception handler
__attribute__ ((interrupt ("IRQ")))
void __cs3_isr_irq(void) {
    volatile int *cpu_ptr = (int *) MPCORE_GIC_CPUIF;
    volatile int *acknowledge_ptr = (int *) (MPCORE_GIC_CPUIF + ICCIAR);
    volatile int *clear_interrupt_ptr = (int *) (MPCORE_GIC_CPUIF + ICCEOIR);
    
    // Go to the correct interrupt service routine
    int interrupt_id = *acknowledge_ptr;
    switch (interrupt_id) {
        // PS2 Mouse
        case PS2_IRQ: mouse_isr(); break;
        // Unexpected: stop here
        default: 
            while (1)
                ;
    }
    
    // Clear the interrupt
    *clear_interrupt_ptr = interrupt_id;
}