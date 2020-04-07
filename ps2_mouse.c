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
    if (clicked && !game_start) {
        // start the game
        game_start = true;
    }
    else if (clicked && !last_clicked && !draw_connection) {
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
        // update status variables of the block
        s[r][c].active = false;
        s[r][c].exposed = false;
        s[clicked_row][clicked_col].active = false;
        s[clicked_row][clicked_col].exposed = false;
        // record the boxes
        match1_r = clicked_row;
        match1_c = clicked_col;
        match2_r = r;
        match2_c = c;
        // Calculate the connection path
        calculate_backtrack();
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

// Calculate the connection path
void calculate_backtrack(void) {
    draw_connection = true;
    connection_color = s[match1_r][match1_c].color;
    connect_line_num_pixels = 0;
    // Start backtracking
    int id1 = get_dijkstra_id(match2_r, match2_c), id2;
    int r = match2_r + 1, c = match2_c + 1;
    int x1, y1, x2, y2, delta;
    x1 = grid_left + c * SQUARE_SIZE - SQUARE_SIZE / 2;
    y1 = grid_top + r * SQUARE_SIZE - SQUARE_SIZE / 2;
    while (id1 != -1) {
        id2 = backtrack[id1];
        // get new block indices
        r = id2 / (NUM_COLS + 2);
        c = id2 - r * (NUM_COLS + 2);
        // Get the x, y position
        x2 = grid_left + c * SQUARE_SIZE - SQUARE_SIZE / 2;
        y2 = grid_top + r * SQUARE_SIZE - SQUARE_SIZE / 2;
        if (x1 == x2) {
            delta = (y1 < y2) ? 1 : -1;
            for (int y = y1; y != y2; y += delta) {
                connect_pixels[connect_line_num_pixels++] = (y << 16) | x1;
            }
        } else if (y1 == y2) {
            delta = (x1 < x2) ? 1 : -1;
            for (int x = x1; x != x2; x += delta) {
                connect_pixels[connect_line_num_pixels++] = (y1 << 16) | x;
            }
        }
        // Update x and y
        x1 = x2;
        y1 = y2;
        id1 = id2;
    }
    // Start drawing from the end
    connect_line_index = connect_line_num_pixels;
}