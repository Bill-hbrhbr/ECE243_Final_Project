/* This files provides address values that exist in the system */

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
#define NUM_ROWS              8
#define NUM_COLS              6

/* Game status */
#define START                 0x1
#define PLAY                  0x2
#define WAIT                  0x4

/* Colors */
#define COLOR_BLUE            0x001F  
#define COLOR_RED             0xF800  
#define COLOR_GREEN           0x07E0
#define COLOR_PINK            0xF81F
#define COLOR_YELLOW          0xFFF1
#define COLOR_ORANGE          0xFEA0
#define COLOR_WHITE           0xFFFF
#define COLOR_BLACK           0x0000
#define NUM_COLORS            6

short int colors[NUM_COLORS] = {COLOR_BLUE, COLOR_RED, COLOR_GREEN, COLOR_PINK, COLOR_YELLOW, COLOR_ORANGE};

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

// global variables
volatile unsigned char mouse_byte1, mouse_byte2, mouse_byte3;
volatile int pixel_buffer_start;
volatile int* pixel_front_buffer_ptr = (int *) PIXEL_BUF_CTRL_BASE;
volatile int* pixel_back_buffer_ptr = (int *) (PIXEL_BUF_CTRL_BASE + 0x4);
volatile int* pixel_resolution_ptr = (int *) (PIXEL_BUF_CTRL_BASE + 0x8);
volatile int* pixel_status_ptr = (int *) (PIXEL_BUF_CTRL_BASE + 0xC);
int mouse_x = 160, mouse_y = 120, mouse_byte_num = 0;
const int grid_left = 30, grid_top = 30;

// Prototypes
void config_GIC(void);
void config_interrupt (int N, int CPU_target);
void config_ps2_mouse();
void mouse_isr(void);
void __cs3_isr_irq(void);
void init_IRQ(void);
void vga_init(void);
void draw_box(int left, int top, int size, short int box_color);
void wait_for_vsync(void);
void plot_pixel(int x, int y, short int pixel_color);
void swap(int *x, int *y);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void clear_screen(void);

int main(void) {
    // Seed random engine
    srand((unsigned) time(NULL));
    
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
    
    // intialize interrupt services
    init_IRQ();
    
    // initialize two buffers
    vga_init();
    
    while (1) {
        // new back buffer
        pixel_buffer_start = *pixel_back_buffer_ptr; 
        
        // draw box
        draw_box(mouse_x, mouse_y, 10, COLOR_WHITE);
        
        // Write a one to the front buffer to turn on status flag S
        *pixel_front_buffer_ptr = 1;
        
        // poll the status register. if the flag is still on, wait
        wait_for_vsync();
    }
}