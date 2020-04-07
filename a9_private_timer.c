// Interrupt service routine for A9 private timer
void timer_isr(void) {
    volatile int *timer_base_ptr = (int *) MPCORE_PRIV_TIMER;
    volatile int *timer_ctrl_ptr = timer_base_ptr + 0x2;
    volatile int *timer_acknowledge_ptr = timer_base_ptr + 0x3;
    // Acknowledge the interrupt
    *timer_acknowledge_ptr = 0x1;
    
    // if the game hasn't started, do not perform anything
    if (!game_start) {
        return;
    }
    
    play_time += TIME_DIV;
    // Won the game
    if (!num_remaining_blocks) {
        game_start = false;
        current_buffer_number = 3;
        return;
    } 
    // lost the game
    else if (play_time >= GAME_TIME) {
        game_start = false;
        current_buffer_number = 4;
        return;
    }
    
    // Update buffers
    --timer_pos;
    for (int i = TIMER_LEFT; i < TIMER_RIGHT; ++i) {
        buffer[i][timer_pos] = COLOR_BLACK;
        plot_pixel_with_buffer(SDRAM_BASE, i, timer_pos, COLOR_BLACK);
        plot_pixel_with_buffer(FPGA_ONCHIP_BASE, i, timer_pos, COLOR_BLACK);
    }
}

