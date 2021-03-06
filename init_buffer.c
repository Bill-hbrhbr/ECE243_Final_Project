void init_buffer(void) {
    // initialize everything to black
    memset(buffer, 0x0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(short int));
//    for (int i = 0; i < SCREEN_WIDTH; ++i) {
//        for (int j = 0; j < SCREEN_HEIGHT; ++j) {
//            buffer[i][j] = COLOR_BLACK;
//        }
//    }
    
    // Initialize the timer to green
    for (int i = TIMER_LEFT; i < TIMER_RIGHT; ++i) {
        for (int j = TIMER_TOP; j < TIMER_BOT; ++j) {
            buffer[i][j] = COLOR_GREEN;
        }
    }
    
    // draw timer icon
    for (int x = 0; x < TIMER_ICON_SIZE; ++x) {
        for (int y = 0; y < TIMER_ICON_SIZE; ++y) {
            short int color = (timer_mif[y][x * 2] << 8) | (timer_mif[y][x * 2 + 1]);
            if (color == 0x1F00) {
                color = 0xFFFF;
            }
            buffer[x + TIMER_ICON_LEFT][y + TIMER_ICON_TOP] = ~color;
        }
    }
}

void init_load_buffer(void) {
    short int color;
    // Loading screen
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            color = (loading_mif[y][x * 2] << 8) | (loading_mif[y][x * 2 + 1]);
            load_buffer[x][y] = ~color;
            plot_pixel_with_buffer(buffers[2].buffer_addr, x, y, ~color);
        }
    }
    // Win screen
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            color = (win_mif[y][x * 2] << 8) | (win_mif[y][x * 2 + 1]);
            win_buffer[x][y] = ~color;
            plot_pixel_with_buffer(buffers[3].buffer_addr, x, y, ~color);
        }
    }
    
    // Lose screen
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            color = (lose_mif[y][x * 2] << 8) | (lose_mif[y][x * 2 + 1]);
            lose_buffer[x][y] = ~color;
            plot_pixel_with_buffer(buffers[4].buffer_addr, x, y, ~color);
        }
    }
}

void init_blocks(void) {
    // Initialize all colors to black
    for (int r = 0; r < NUM_ROWS; ++r) {
        for (int c = 0; c < NUM_COLS; ++c) {
            s[r][c].color = COLOR_BLACK;
        }
    }
    
    // Initialize all blocks
    int color_count[NUM_COLORS] = {0}; // keep track of color count
    for (int i = 0; i < NUM_ROWS; ++i) {
        for (int j = 0; j < NUM_COLS; ++j) {
            // Get a random color
            bool color_valid = false;
            int color_choice = rand() % NUM_COLORS;
            while (!color_valid) {
                color_valid = true;
                color_choice = (color_choice + 1) % NUM_COLORS;
                // Check if same color already exists in adjacent celles
                if (i > 0 && colors[color_choice] == s[i - 1][j].color) {
                    color_valid = false;
                }
                if (i < NUM_ROWS - 1 && colors[color_choice] == s[i + 1][j].color) {
                    color_valid = false;
                }
                if (j > 0 && colors[color_choice] == s[i][j - 1].color) {
                    color_valid = false;
                }
                if (j < NUM_COLS - 1 && colors[color_choice] == s[i][j + 1].color) {
                    color_valid = false;
                }
            }
            s[i][j].color = colors[color_choice];
            ++color_count[color_choice];
            
            // Intialize position
            s[i][j].left = grid_left + i * SQUARE_SIZE;
            s[i][j].top = grid_top + j * SQUARE_SIZE;
            
            // Intialize status
            s[i][j].active = true;
        }
    }
    
    // Get rid of odd numbered colors
    int odd_colors = 0;
    short int first, second;
    for (int i = 0; i < NUM_COLORS; ++i) {
        if (color_count[i] & 0x1) {
            ++odd_colors;
            first = second;
            second = colors[i];
        } else {
            continue;
        }
        if (odd_colors == 2) {
            // replace a block of the first color with the second color
            Square *p = (Square*) s;
            for (int i = 0; i < NUM_ROWS * NUM_COLS; ++i) {
                if (p[i].color == first) {
                    p[i].color = second;
                    break;
                }
            }
            odd_colors = 0;
        }
    }
    
    // Update buffer
    for (int r = 0; r < NUM_ROWS; ++r) {
        for (int c = 0; c < NUM_COLS; ++c) {
            for (int x = 0; x < SQUARE_SIZE; ++x) {
                for (int y = 0; y < SQUARE_SIZE; ++y) {
                    buffer[grid_left + c * SQUARE_SIZE + x][grid_top + r * SQUARE_SIZE + y] = s[r][c].color;
                }
            }
        }
    }
}

void init_cursors(void) {
    // Set initial position to 0 to avoid vga index out of range
    for (int i = 0; i < NUM_BUFFERS; ++i) {
        buffers[i].cursor_x = 0;
        buffers[i].cursor_y = 0;
    }
}

void init_dijkstra(void) {
    // Set all vacancy to false
    memset(vacant, false, DIJKSTRA_SIZE * sizeof(bool));
    // Set all the border boxes to true
    int r, c;
    // Left
    for (r = 0, c = 0; r < NUM_ROWS + 2; ++r) {
        vacant[r * (NUM_COLS + 2) + c] = true;
    }
    // Right
    for (r = 0, c = NUM_COLS + 1; r < NUM_ROWS + 2; ++r) {
        vacant[r * (NUM_COLS + 2) + c] = true;
    }
    // Top
    for (r = 0, c = 0; c < NUM_COLS + 2; ++c) {
        vacant[r * (NUM_COLS + 2) + c] = true;
    }
    // Bottom
    for (r = NUM_ROWS + 1, c = 0; c < NUM_COLS + 2; ++c) {
        vacant[r * (NUM_COLS + 2) + c] = true;
    }
}