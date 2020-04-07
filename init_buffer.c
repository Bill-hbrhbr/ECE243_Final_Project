void init_buffer(void) {
    // initialize everything to black
    for (int i = 0; i < SCREEN_WIDTH; ++i) {
        for (int j = 0; j < SCREEN_HEIGHT; ++j) {
            buffer[i][j] = COLOR_BLACK;
        }
    }
    
    // set initial buffer number
    current_buffer_number = 0;
}

void init_blocks(void) {
    // Initialize all colors to black
    for (int r = 0; r < NUM_ROWS; ++r) {
        for (int c = 0; c < NUM_COLS; ++c) {
            s[r][c].color = COLOR_BLACK;
        }
    }
    
    // Initialize all blocks
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
            
            // Intialize position
            s[i][j].left = grid_left + i * SQUARE_SIZE;
            s[i][j].top = grid_top + j * SQUARE_SIZE;
            
            // Intialize status
            s[i][j].active = true;
            if (i == 0 || i == NUM_ROWS - 1 || j == 0 || j == NUM_COLS - 1) {
                s[i][j].exposed = true;
            } else {
                s[i][j].exposed = false;
            }
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
        cursor_info[i].x = 0;
        cursor_info[i].y = 0;
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