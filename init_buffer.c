void init_buffer(void) {
    // initialize everything to black
    for (int i = 0; i < SCREEN_WIDTH; ++i) {
        for (int j = 0; j < SCREEN_HEIGHT; ++j) {
            buffer[i][j] = COLOR_BLACK;
        }
    }
}

void init_blocks(void) {
    // Initialize all colors to black
    for (int i = 0; i < NUM_ROWS; ++i) {
        for (int j = 0; j < NUM_COLS; ++j) {
            s[i][j].color = COLOR_BLACK;
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
    for (int i = 0; i < NUM_ROWS; ++i) {
        for (int j = 0; j < NUM_COLS; ++j) {
            for (int x = 0; x < SQUARE_SIZE; ++x) {
                for (int y = 0; y < SQUARE_SIZE; ++y) {
                    buffer[grid_left + i * SQUARE_SIZE + x][grid_top + j * SQUARE_SIZE + y] = s[i][j].color;
                }
            }
        }
    }
}