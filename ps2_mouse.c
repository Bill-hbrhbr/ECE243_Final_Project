#define PS2_BASE              0xFF200100

volatile unsigned char mouse_byte1, mouse_byte2, mouse_byte3;

// Interrupt service routine for ps2 mouse
void mouse_isr(void) {
    volatile int *ps2_ptr = (int *) PS2_BASE;
    int ps2_data, data_valid;
    // Get the latest data
    ps2_data = *ps2_ptr;
    // extract valid field
    data_valid = ps2_data & 0x8000;
    
    // if the data is valid, update the bytes
    if (data_valid) {
        mouse_byte1 = mouse_byte2;
        mouse_byte2 = mouse_byte3;
        mouse_byte3 = ps2_data & 0xFF;
    }
    
    // If the mouse is inactive, make it send data
    if (mouse_byte2 == 0xAA && mouse_byte3 == 0x00) {
        *ps2_ptr = 0xF4; // send data command
    }
}