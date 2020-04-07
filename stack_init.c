// intialize interrupt services
void init_IRQ(void) {
    int mode;
    
    // SVC mode with IRQ disabled
    mode = INT_DISABLE | SVC_MODE;
    asm volatile("msr cpsr_c, %[mode]" : /* no output */ : [mode] "r" (mode));
    
    // configure Generic Interrupt Controller (GIC)
    config_GIC();
    
    // configure ps2 mouse
    config_ps2_mouse();
    
    // configure private timer
    config_private_timer();
    
    // Enable IRQ, disable FIQ
    mode = INT_ENABLE | SVC_MODE;
    asm volatile("msr cpsr_c, %[mode]" : /* no output */ : [mode] "r" (mode));
}