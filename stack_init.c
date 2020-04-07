/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_END       0xC803FFFF
#define FPGA_CHAR_BASE        0xC9000000
#define FPGA_CHAR_END         0xC9001FFF

/* Program Modes */
#define USER_MODE             0x10
#define FIQ_MODE              0x11
#define IRQ_MODE              0x12
#define SVC_MODE              0x13
#define ABORT_MODE            0x17
#define UNDEF_MODE            0x1B
#define SYS_MODE              0x1F

#define INT_ENABLE            0x40
#define INT_DISABLE           0xC0

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