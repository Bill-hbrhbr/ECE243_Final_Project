/* initialize two buffers */
void vga_init(void) {
    // Set back buffer
    *pixel_back_buffer_ptr = SDRAM_BASE; // start of SDRAM memory
    // Clear back buffer
    pixel_buffer_start = *pixel_back_buffer_ptr; // point to the back buffer
    draw_buffer();
    // Write a one to the front buffer to turn on status flag S
    *pixel_front_buffer_ptr = 0x1;
    // Wait for swap
    wait_for_vsync();
    
    // Set front buffer
    *pixel_back_buffer_ptr = FPGA_ONCHIP_BASE; // start of FPGA On-Chip Memory
    // Clear front buffer
    pixel_buffer_start = *pixel_back_buffer_ptr; // point to the back buffer
    draw_buffer();
    // Write a one to the front buffer to turn on status flag S
    *pixel_front_buffer_ptr = 0x1;
    // Wait for swap
    wait_for_vsync();
}

/* draw an N * N box given the topleft coordinate */
void draw_box(int left, int top, int size, short int box_color) {
    for (int x = left; x < left + size; ++x) {
        for (int y = top; y < top + size; ++y) {
            plot_pixel(x, y, box_color);
        }
    }
}

/* wait for buffer swap: vertical synchronization */
void wait_for_vsync(void) {
    while ((*pixel_status_ptr) & 1) {
        ;
    }
    // Switch buffer number
    if (current_buffer_number == 0) {
        current_buffer_number = 1;
    } else if (current_buffer_number = 1) {
        current_buffer_number = 0;
    }
}

/* plot a pixel */
void plot_pixel(int x, int y, short int pixel_color) {
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = pixel_color;
}

/* integer swap function */
void swap(int *x, int *y) {
    int temp = *x;
    *x = *y;
    *y = temp;
}

/* line drawing algorithm */
void draw_line(int x0, int y0, int x1, int y1, short int line_color) {
    bool is_steep = abs(y1 - y0) > abs(x1 - x0);
    if (is_steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    
    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }
    
    int deltax = x1 - x0, deltay = abs(y1 - y0);
    int error = - (deltax / 2);
    int y = y0, y_step = (y0 < y1) ? 1 : -1;
    
    for (int x = x0; x <= x1; ++x) {
        if (is_steep) {
            plot_pixel(y, x, line_color);
        } else {
            plot_pixel(x, y, line_color);
        }
        error += deltay;
        if (error >= 0) {
            y += y_step;
            error -= deltax;
        }
    }
}

/* clear screen */
void clear_screen(void) {
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            plot_pixel(x, y, COLOR_BLACK);
        }
    }
}

/* draw buffer */
void draw_buffer(void) {
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            plot_pixel(x, y, buffer[x][y]);
        }
    }
}

/* draw cursor */
void draw_cursor(int left, int top, bool erase) {
    short int color;
    for (int x = 0; x < CURSOR_SIZE; ++x) {
        for (int y = 0; y < CURSOR_SIZE; ++y) {
            // get the vga positions
            int vga_x = x + left, vga_y = y + top;
            // skip the position if it is out of vga index bounds
            if (vga_x >= SCREEN_WIDTH || vga_y >= SCREEN_HEIGHT) {
                continue;
            }
            
            // get the pixel color
            color = (cursor_mif[y][x * 2] << 8) | cursor_mif[y][x * 2 + 1];
            // skip the color if it is not black
            if (color) {
                continue;
            }
            // Check erase conditions
            if (erase) {
                // Replace the pixel with buffer content
                plot_pixel(vga_x, vga_y, buffer[vga_x][vga_y]);
                
            } else {
                // draw a white cursor
                plot_pixel(vga_x, vga_y, COLOR_WHITE);
            }
        }
    }
}