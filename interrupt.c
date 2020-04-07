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
        // A9 Private Timer
        case MPCORE_PRIV_TIMER_IRQ: timer_isr(); break;
        // Unexpected: stop here
        default: 
            while (1)
                ;
    }
    
    // Clear the interrupt
    *clear_interrupt_ptr = interrupt_id;
}