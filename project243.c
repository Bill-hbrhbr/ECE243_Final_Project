#define BOARD                 "DE1-SoC"

/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_END       0xC803FFFF
#define FPGA_CHAR_BASE        0xC9000000
#define FPGA_CHAR_END         0xC9001FFF

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define JP1_BASE              0xFF200060
#define JP2_BASE              0xFF200070
#define PS2_BASE              0xFF200100
#define PS2_DUAL_BASE         0xFF200108
#define JTAG_UART_BASE        0xFF201000
#define JTAG_UART_2_BASE      0xFF201008
#define IrDA_BASE             0xFF201020
#define TIMER_BASE            0xFF202000
#define AV_CONFIG_BASE        0xFF203000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define AUDIO_BASE            0xFF203040
#define VIDEO_IN_BASE         0xFF203060
#define ADC_BASE              0xFF204000

/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE        0xFF709000
#define HPS_TIMER0_BASE       0xFFC08000
#define HPS_TIMER1_BASE       0xFFC09000
#define HPS_TIMER2_BASE       0xFFD00000
#define HPS_TIMER3_BASE       0xFFD01000
#define FPGA_BRIDGE           0xFFD0501C

/* ARM A9 MPCORE devices */
#define PERIPH_BASE           0xFFFEC000    // base address of peripheral devices
#define MPCORE_PRIV_TIMER     0xFFFEC600    // PERIPH_BASE + 0x0600

/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg
#define CPU0                  0x1           // CPU0
#define CPU1                  0x2           // CPU1

/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST       0xFFFED000    // PERIPH_BASE + 0x1000
#define ICDDCR                0x00          // offset to distributor control reg
#define ICDISER               0x100         // offset to interrupt set-enable regs
#define ICDICER               0x180         // offset to interrupt clear-enable regs
#define ICDIPTR               0x800         // offset to interrupt processor targets regs
#define ICDICFR               0xC00         // offset to interrupt configuration regs

/* Interrupt IDs */
#define MPCORE_PRIV_TIMER_IRQ 29
#define INTERVAL_TIMER_IRQ    72
#define PS2_IRQ               79
#define PS2_DUAL_IRQ          89

/* Program Modes */
#define USER_MODE             0x10
#define FIQ_MODE              0x11
#define IRQ_MODE              0x12
#define SVC_MODE              0x13
#define ABORT_MODE            0x17
#define UNDEF_MODE            0x1B
#define SYS_MODE              0x1F

#define INT_ENABLE            0x40
#define INT_DISABLE           0xC0

/* VGA settings */
#define SCREEN_WIDTH          320
#define SCREEN_HEIGHT         240
#define SQUARE_SIZE           30
#define NUM_ROWS              6
#define NUM_COLS              8
#define NUM_BUFFERS           2
#define CURSOR_SIZE           20
#define SELECT_WIDTH          2

/* Game status */
#define START                 0x1
#define PLAY                  0x2
#define WAIT                  0x4

/* Colors */
#define COLOR_BLUE            0x001F  
#define COLOR_RED             0xF800  
#define COLOR_GREEN           0x07E0
#define COLOR_GREY            0x8C91
#define COLOR_PINK            0xF81F
#define COLOR_YELLOW          0xFFF1
#define COLOR_CYAN            0xAEDC
#define COLOR_ORANGE          0xFEA0
#define COLOR_WHITE           0xFFFF
#define COLOR_BLACK           0x0000
#define NUM_COLORS            8

short int colors[NUM_COLORS] = {COLOR_BLUE, COLOR_RED, COLOR_GREEN, COLOR_PINK, COLOR_YELLOW, COLOR_ORANGE, COLOR_CYAN, COLOR_GREY};

#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "math.h"
#include "time.h"

typedef struct square{
    short int color;
    int left, top;
    bool active;
    bool exposed;
} Square;

// Initialize the blocks
Square s[NUM_ROWS][NUM_COLS];

// Screen pixel colors
short int buffer[SCREEN_WIDTH][SCREEN_HEIGHT];

// Cursors
int current_buffer_number;
typedef struct cursor{
    int x, y;
} Cursor;
Cursor cursor_info[NUM_BUFFERS];

// global variables
volatile unsigned char mouse_byte1, mouse_byte2, mouse_byte3;
volatile int pixel_buffer_start;
volatile int* pixel_front_buffer_ptr = (int *) PIXEL_BUF_CTRL_BASE;
volatile int* pixel_back_buffer_ptr = (int *) (PIXEL_BUF_CTRL_BASE + 0x4);
volatile int* pixel_resolution_ptr = (int *) (PIXEL_BUF_CTRL_BASE + 0x8);
volatile int* pixel_status_ptr = (int *) (PIXEL_BUF_CTRL_BASE + 0xC);
int mouse_x = 160, mouse_y = 120, mouse_byte_num = 0;
const int grid_left = 30, grid_top = 30;
bool last_clicked = false, run_mouse = false;
int clicked_row = -1, clicked_col = -1;

// Dijkstra global variables
#define DIJKSTRA_SIZE ((NUM_ROWS + 2) * (NUM_COLS + 2))
typedef struct node {
    int index;
    unsigned distance;
} Node;
Node priority_queue[DIJKSTRA_SIZE];
int backtrack[DIJKSTRA_SIZE];
unsigned int best_distances[DIJKSTRA_SIZE];
bool vacant[DIJKSTRA_SIZE];
int queue_size;

// Draw connection lines
bool draw_connection = false;
short int connection_color = COLOR_BLACK;
int connect_line_index, connect_line_num_pixels, match1_r, match1_c, match2_r, match2_c;
unsigned int connect_pixels[10000];

// Prototypes
void init_buffer(void);
void init_blocks(void);
void init_cursors(void);
void init_dijkstra(void);

void config_GIC(void);
void config_interrupt (int N, int CPU_target);
void config_ps2_mouse();

void mouse_isr(void);
bool get_clicked_tile(int *r, int *c);
void update_grid_status(int r, int c);
void mark_selection(int r, int c, short int color);
void remove_block(int r, int c);

void __cs3_isr_irq(void);
void init_IRQ(void);

void vga_init(void);
void draw_box(int left, int top, int size, short int box_color);
void wait_for_vsync(void);
void plot_pixel(int x, int y, short int pixel_color);
void plot_pixel_with_buffer(volatile int buffer_start, int x, int y, short int pixel_color);
void swap(int *x, int *y);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void clear_screen(void);
void draw_buffer(void);
void draw_cursor(int left, int top, bool erase);

bool find_path(int src_row, int src_col, int dest_row, int dest_col);
inline int get_dijkstra_id(int r, int c);
void pq_insert(int node_id, int src_id, unsigned int new_dist);
void calculate_backtrack(void);

unsigned char cursor_mif[CURSOR_SIZE][CURSOR_SIZE * 2] =
{
    /*Pixel format: Red: 5 bit, Green: 6 bit, Blue: 5 bit*/
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x10, 0x00, 0x17, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x03, 0x00, 0x13, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0e, 0x00, 0x1d, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x1a, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x15, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0f, 0x00, 0x1e, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x1b, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x16, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x11, 0x00, 0x1e, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x1c, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x05, 0x00, 0x08, 0x00, 0x0d, 0x00, 0x11, 0x00, 0x1b, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x15, 0x00, 0x1d, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x1a, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x1b, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x1d, 0x00, 0x1f, 0x00, 0x17, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0c, 0x00, 0x0f, 0x00, 0x1e, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x1b, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1a, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x16, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1e, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x1b, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x16, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x18, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
  0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1e, 0x00, 0x09, 0x00, 0x0c, 0x00, 0x1a, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 
};

int main(void) {
    // Seed random engine
    srand((unsigned) time(NULL));
    
    // Initialize buffer
    init_buffer();
    
    // Initialize blocks
    init_blocks();
    
    // Initialize buffer cursors
    init_cursors();
    
    // Initialize dijkstra global variables
    init_dijkstra();
    
    // intialize interrupt services
    init_IRQ();
    
    // initialize two buffers
    vga_init();
    
    while (1) {
        // new back buffer
        pixel_buffer_start = *pixel_back_buffer_ptr; 
        
        // erase the previous cursor
        draw_cursor(cursor_info[current_buffer_number].x, cursor_info[current_buffer_number].y, true);
        
        // draw connection if there is any
        if (draw_connection) {
            // Draw the next pixel
            int x, y;
            for (int i = 0; i < SQUARE_SIZE / 2; ++i) {
                --connect_line_index;
                x = connect_pixels[connect_line_index] & 0xFFFF;
                y = connect_pixels[connect_line_index] >> 16;
                plot_pixel_with_buffer(SDRAM_BASE, x, y, connection_color);
                plot_pixel_with_buffer(FPGA_ONCHIP_BASE, x, y, connection_color);
            }
            
            // If the line is complete, erase both the line and the box
            if (connect_line_index == 0) {
                for (int i = 0; i < connect_line_num_pixels; ++i) {
                    x = connect_pixels[i] & 0xFFFF;
                    y = connect_pixels[i] >> 16;
                    plot_pixel_with_buffer(SDRAM_BASE, x, y, COLOR_BLACK);
                    plot_pixel_with_buffer(FPGA_ONCHIP_BASE, x, y, COLOR_BLACK);
                }
                remove_block(match1_r, match1_c);
                remove_block(match2_r, match2_c);
                // Reset connection settings
                draw_connection = false;
            }
        }
        
        // draw the new cursor
        draw_cursor(mouse_x, mouse_y, false);
        
        // update the cursor info
        cursor_info[current_buffer_number].x = mouse_x;
        cursor_info[current_buffer_number].y = mouse_y;
        
        // Write a one to the front buffer to turn on status flag S
        *pixel_front_buffer_ptr = 1;
        
        // poll the status register. if the flag is still on, wait
        wait_for_vsync();
    }
}

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
            Square *p = (Node*) s;
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

/*
* Configure the Generic Interrupt Controller (GIC)
*/
void config_GIC(void) {
    
    // configure the PS2 port (Interrupt ID = 79)
    config_interrupt (PS2_IRQ, CPU0); 
    
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

// Configure the ps2 mouse interrupt
void config_ps2_mouse() {
    // Enable the ps2 interrupt
    volatile int *ps2_status_ptr = (int *) (PS2_BASE + 0x4);
    *ps2_status_ptr = 0x1;
}

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
    if (clicked && !last_clicked && !draw_connection) {
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

// Define the IRQ exception handler
__attribute__ ((interrupt ("IRQ")))
void __cs3_isr_irq(void) {
    volatile int *cpu_ptr = (int *) MPCORE_GIC_CPUIF;
    volatile int *acknowledge_ptr = (int *) (MPCORE_GIC_CPUIF + ICCIAR);
    volatile int *clear_interrupt_ptr = (int *) (MPCORE_GIC_CPUIF + ICCEOIR);
    
    // Go to the correct interrupt service routine
    int interrupt_id = *acknowledge_ptr;
    switch (interrupt_id) {
        // PS2 Mouse
        case PS2_IRQ: mouse_isr(); break;
        // Unexpected: stop here
        default: 
            while (1)
                ;
    }
    
    // Clear the interrupt
    *clear_interrupt_ptr = interrupt_id;
}

// intialize interrupt services
void init_IRQ(void) {
    int mode;
    
    // SVC mode with IRQ disabled
    mode = INT_DISABLE | SVC_MODE;
    asm volatile("msr cpsr_c, %[mode]" : /* no output */ : [mode] "r" (mode));
    
    // configure Generic Interrupt Controller (GIC)
    config_GIC();
    
    // configure ps2 mouse
    config_ps2_mouse();
    
    // Enable IRQ, disable FIQ
    mode = INT_ENABLE | SVC_MODE;
    asm volatile("msr cpsr_c, %[mode]" : /* no output */ : [mode] "r" (mode));
}

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
    } else if (current_buffer_number == 1) {
        current_buffer_number = 0;
    }
}

/* plot a pixel */
void plot_pixel(int x, int y, short int pixel_color) {
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = pixel_color;
}

void plot_pixel_with_buffer(volatile int buffer_start, int x, int y, short int pixel_color) {
    *(short int *)(buffer_start + (y << 10) + (x << 1)) = pixel_color;
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

// Find path between two tiles. Return true if found, false if not
bool find_path(int src_row, int src_col, int dest_row, int dest_col) {
    // Clear the processed array: set all to false
    //memset(processed, false, DIJKSTRA_SIZE * sizeof(bool));
    // Set the backtrack array to -1
    memset(backtrack, 0xFF, DIJKSTRA_SIZE * sizeof(int));
    // Set the best distances to INT_MAX
    memset(best_distances, 0xFF, DIJKSTRA_SIZE * sizeof(unsigned int));
    
    // Set the queue size to 0
    queue_size = 0;
    // Get the dijkstra ids of the src and dest positions
    int best_node_id = get_dijkstra_id(src_row, src_col);
    int dest_node_id = get_dijkstra_id(dest_row, dest_col);
    
    // The best distance so far is 0
    unsigned int best_dist = 0;
    best_distances[best_node_id] = 0;
    //processed[best_node_id] = true;
    
    while (true) {
        int left = best_node_id - 1, right = best_node_id + 1;
        int up = best_node_id - (NUM_COLS + 2), down = best_node_id + (NUM_COLS + 2);
        
        // Extend the best_node_id in all 4 directions
        // up or down
        if (up == dest_node_id || down == dest_node_id) {
            backtrack[dest_node_id] = best_node_id;
            return true;
        }
        pq_insert(up, best_node_id, best_dist + 1); // up
        pq_insert(down, best_node_id, best_dist + 1); // down
        
        // left
        if (best_node_id % (NUM_COLS + 2)) {
            if (left == dest_node_id) {
                backtrack[dest_node_id] = best_node_id;
                return true;
            }
            pq_insert(left, best_node_id, best_dist + 1);
        }
        
        // right
        if (right % (NUM_COLS + 2)) {
            if (right == dest_node_id) {
                backtrack[dest_node_id] = best_node_id;
                return true;
            }
            pq_insert(right, best_node_id, best_dist + 1);
        }
        
        // Check the best element in the priority queue
        while (queue_size > 0) {
            // If the node has already been processed, skip
            if (priority_queue[queue_size - 1].distance > best_distances[priority_queue[queue_size - 1].index]) {
                --queue_size;
            } else {
                break;
            }
        }
        
        // If empty, then there's no valid path
        if (queue_size == 0) {
            return false;
        }
        
        // Update the best node
        --queue_size;
        best_node_id = priority_queue[queue_size].index;
        best_dist = priority_queue[queue_size].distance;
    }
    return true;
}

// Return the dijkstra queue index given a row and a col
inline int get_dijkstra_id(int r, int c) {
    return (r + 1) * (NUM_COLS + 2) + c + 1;
}

// Insert a node into the priority queue
void pq_insert(int node_id, int src_id, unsigned int new_dist) {
    // Check if out of bounds
    if (node_id < 0 || node_id >= DIJKSTRA_SIZE) {
        return;
    }
    // Check if vacant
    if (!vacant[node_id]) {
        return;
    }
    // Check if the time is longer than already recorded
    if (best_distances[node_id] < new_dist) {
        return;
    }

    // Update the new best distance
    best_distances[node_id] = new_dist;
    // Update backtrack list
    backtrack[node_id] = src_id;
    
    // Push into the priority queue
    priority_queue[queue_size].index = node_id;
    priority_queue[queue_size].distance = new_dist;
    ++queue_size;
    
    // Correct the order using insertion sort
    for (int i = queue_size - 1; i > 0; --i) {
        if (priority_queue[i].distance > priority_queue[i - 1].distance) {
            Node temp = priority_queue[i];
            priority_queue[i] = priority_queue[i - 1];
            priority_queue[i - 1] = temp;
        } else {
            break;
        }
    }
}
