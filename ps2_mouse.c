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
        mouse_byte_num = 0;
    } else {
        mouse_byte_num = (mouse_byte_num + 1) % 3;
    }
    
    // Update the mouse position
    if (mouse_byte_num == 0) {
        // extract x, y signs
        unsigned char sign_x = mouse_byte1 & 0x10;
        unsigned char sign_y = mouse_byte1 & 0x20;
        
        // update mouse positions
        if (sign_x) {
            mouse_x -= (0x100 - mouse_byte2);
        } else {
            mouse_x += mouse_byte2;
        }
        if (sign_y) {
            mouse_y -= (0x100 - mouse_byte3);
        } else {
            mouse_y += mouse_byte3;
        }
        
        // check box boundaries
        if (mouse_x < 0) {
            mouse_x = 0;
        }
        if (mouse_x >= SCREEN_WIDTH) {
            mouse_x = SCREEN_WIDTH - 1;
        }
        if (mouse_y < 0) {
            mouse_y = 0;
        }
        if (mouse_y >= SCREEN_HEIGHT) {
            mouse_y = SCREEN_HEIGHT - 1;
        }
    }
}