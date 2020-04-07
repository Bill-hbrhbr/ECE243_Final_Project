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
void *memset(void *str, int c, size_t n);
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