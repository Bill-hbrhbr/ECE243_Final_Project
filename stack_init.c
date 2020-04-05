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



/* Set up stack pointers for IRQ and SVC processor modes */
void init_stack(void) {
    int mode, stack_top;
    
    // IRQ mode
    mode = INT_DISABLE | IRQ_MODE;
    stack_top = A9_ONCHIP_END - 3;
    asm volatile("msr cpsr_c, #[immd]" : /* no output */ : [immd] "M" (mode));
    asm volatile("ldr sp, =[immd]" : /* no output */ : [immd] "M" (stack_top));
    
    // SVC mode
    mode = INT_DISABLE | SVC_MODE;
    stack_top = DDR_END - 3;
    asm volatile("msr cpsr_c, #[immd]" : /* no output */ : [immd] "M" (mode));
    asm volatile("ldr sp, =[immd]" : /* no output */ : [immd] "M" (stack_top));
}