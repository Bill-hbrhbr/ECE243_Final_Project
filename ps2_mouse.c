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
    if (!run_mouse && mouse_byte2 == 0xAA && mouse_byte3 == 0x00) {
        *ps2_ptr = 0xF4; // send data command
        mouse_byte_num = 0;
        run_mouse = true;
        return;
    } else {
        mouse_byte_num = (mouse_byte_num + 1) % 3;
    }
    
    // If the 3-byte packet is not complete, do not perform analysis
    if (mouse_byte_num) {
        return;
    }
    
    // If mouse_byte1 doesn't satisfy the pattern, do not perform analysis
    if (!(mouse_byte1 & 0x08) || (mouse_byte1 & 0x06)) {
        mouse_byte_num = 2;
        return;
    }
    
    // Update the mouse position
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
    
    // check if the left button is clicked on an object
    bool clicked = (bool) (mouse_byte1 & 0x1);
    // Edge-triggered rather than level sensitive
    if (clicked && !last_clicked) {
        int r, c;
        bool click_valid = get_clicked_tile(&r, &c);
        if (click_valid) {
            update_grid_status(r, c);
        }
    }
    // Update the clicking status
    last_clicked = clicked;
}

// return true if the clicked tile is within grid, false otherwise
bool get_clicked_tile(int *r, int *c) {
    int delta_x = mouse_x - grid_left, delta_y = mouse_y - grid_top;
    // indices smaller than 0
    if (delta_x < 0 || delta_y < 0) {
        return false;
    }
    // get indices
    int col = delta_x / SQUARE_SIZE, row = delta_y / SQUARE_SIZE;
    if (row >= NUM_ROWS || col >= NUM_COLS) {
        return false;
    } else {
        *r = row;
        *c = col;
        return true;
    }
}

// update the grid based on the clicked box
void update_grid_status(int r, int c) {
    // If the block is not active, return
    if (!s[r][c].active) {
        return;
    }
    // If clicking on the same tile, return
    if (r == clicked_row && c == clicked_col) {
        return;
    }
    // Initial selection, simply update clicked pos
    if (clicked_row == -1 && clicked_col == -1) {
        mark_selection(r, c, COLOR_WHITE);
        clicked_row = r;
        clicked_col = c;
        return;
    }
    // Color matches the last selection and both blocks are connected
    if (s[clicked_row][clicked_col].color == s[r][c].color &&
        find_path(clicked_row, clicked_col, r, c)) 
    {
        // get rid of the boxes
        remove_block(clicked_row, clicked_col);
        remove_block(r, c);
        // Update the global variables to default
        clicked_row = -1;
        clicked_col = -1;
        return;
    }
    // Color does not match the last selection
    mark_selection(clicked_row, clicked_col, s[clicked_row][clicked_col].color);
    mark_selection(r, c, COLOR_WHITE);
    // Update global variables
    clicked_row = r;
    clicked_col = c;
    return;
}

// Add a selection box to the block
void mark_selection(int r, int c, short int color) {
    // Get the topleft corner of the block
    int block_left = grid_left + c * SQUARE_SIZE;
    int block_top = grid_top + r * SQUARE_SIZE;
    // Add white boundaries
    for (int x = 0; x < SELECT_WIDTH; ++x) {
        for (int y = 0; y < SQUARE_SIZE; ++y) {
            buffer[block_left + x][block_top + y] = color;
            plot_pixel_with_buffer(SDRAM_BASE, block_left + x, block_top + y, color);
            plot_pixel_with_buffer(FPGA_ONCHIP_BASE, block_left + x, block_top + y, color);
        }
    }
    for (int x = SQUARE_SIZE - 1; x >= SQUARE_SIZE - SELECT_WIDTH; --x) {
        for (int y = 0; y < SQUARE_SIZE; ++y) {
            buffer[block_left + x][block_top + y] = color;
            plot_pixel_with_buffer(SDRAM_BASE, block_left + x, block_top + y, color);
            plot_pixel_with_buffer(FPGA_ONCHIP_BASE, block_left + x, block_top + y, color);
        }
    }
    for (int y = 0; y < SELECT_WIDTH; ++y) {
        for (int x = 0; x < SQUARE_SIZE; ++x) {
            buffer[block_left + x][block_top + y] = color;
            plot_pixel_with_buffer(SDRAM_BASE, block_left + x, block_top + y, color);
            plot_pixel_with_buffer(FPGA_ONCHIP_BASE, block_left + x, block_top + y, color);
        }
    }
    for (int y = SQUARE_SIZE - 1; y >= SQUARE_SIZE - SELECT_WIDTH; --y) {
        for (int x = 0; x < SQUARE_SIZE; ++x) {
            buffer[block_left + x][block_top + y] = color;
            plot_pixel_with_buffer(SDRAM_BASE, block_left + x, block_top + y, color);
            plot_pixel_with_buffer(FPGA_ONCHIP_BASE, block_left + x, block_top + y, color);
        }
    }
}

// Remove a box from the screen
void remove_block(int r, int c) {
    // update status variables of the block
    s[r][c].active = false;
    s[r][c].exposed = false;
    // Get the topleft corner of the block
    int block_left = grid_left + c * SQUARE_SIZE;
    int block_top = grid_top + r * SQUARE_SIZE;
    // Update buffers
    for (int x = 0; x < SQUARE_SIZE; ++x) {
        for (int y = 0; y < SQUARE_SIZE; ++y) {
            buffer[block_left + x][block_top + y] = COLOR_BLACK;
            plot_pixel_with_buffer(SDRAM_BASE, block_left + x, block_top + y, COLOR_BLACK);
            plot_pixel_with_buffer(FPGA_ONCHIP_BASE, block_left + x, block_top + y, COLOR_BLACK);
        }
    }
    // Update the vacant matrix
    vacant[get_dijkstra_id(r, c)] = true;
}