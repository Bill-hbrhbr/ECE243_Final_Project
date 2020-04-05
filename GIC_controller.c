/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg

/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST       0xFFFED000    // PERIPH_BASE + 0x1000
#define ICDDCR                0x00          // offset to distributor control reg
#define ICDISER               0x100         // offset to interrupt set-enable regs
#define ICDICER               0x180         // offset to interrupt clear-enable regs
#define ICDIPTR               0x800         // offset to interrupt processor targets regs
#define ICDICFR               0xC00         // offset to interrupt configuration regs

/*
* Configure the Generic Interrupt Controller (GIC)
*/
void config_GIC(void) {
    
    // configure the KEYs parallel port (Interrupt ID = 73)
    config_interrupt (PS2_IRQ, 0x1); 
    
    // Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all priorities
    *((int *) MPCORE_GIC_CPUIF + ICCICR) = 0xFFFF;
    
    // Set CPU Interface Control Register (ICCICR). Enable signaling of interrupts
    *((int *) MPCORE_GIC_CPUIF + ICCICR) = 0x1;
    
    // Configure the Distributor Control Register (ICDDCR) to send pending interrupts to CPUs
    *((int *) MPCORE_GIC_DIST + ICDDCR) = 0x1;
}

/*
* Configure Set Enable Registers (ICDISERn) and Interrupt Processor Target Registers (ICDIPTRn).
* The default (reset) values are used for other registers in the GIC.
*/
void config_interrupt (int N, int CPU_target) {
    
    // Initialize variables
    int reg_offset, index, value, address;
    
    // Configure the Interrupt Set-Enable Registers (ICDISERn).
    // reg_offset = integer_div(N / 32) * 4
    reg_offset = (N >> 3) & ~0x3;
    address = MPCORE_GIC_DIST + ICDISER + reg_offset;
    
    // value = 1 << (N mod 32)
    index = N & 0x1F;
    value = 0x1 << index;
    
    // Set the bit
    *(int *) address |= value;
    
    
    // Configure the Interrupt Processor Targets Register (ICDIPTRn)
    // reg_offset = integer_div(N / 4) * 4
    reg_offset = N & ~0x3;
    
    // index = N mod 4
    index = N & 0x3;
    
    // get address
    address = MPCORE_GIC_DIST + ICDIPTR + reg_offset + index;
    
    /* Now that we know the register address and value, write to (only) the appropriate byte */
    *(char *) address = (char) CPU_target;
}