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
#define SQUARE_SIZE           8
#define NUM_SQUARES           8

/* Colors */
#define COLOR_BLUE            0x001F  
#define COLOR_RED             0xF800  
#define COLOR_GREEN           0x07E0
#define COLOR_PINK            0xF81F 
#define COLOR_WHITE           0xFFFF
#define COLOR_BLACK           0x0000


#include "stdlib.h"
#include "stdbool.h"
#include "math.h"
#include "time.h"

//typedef struct square{
//    unsigned int pos_x, pos_y;
//    int delta_x, delta_y;
//} Square;

// global variables
volatile int pixel_buffer_start; 
volatile unsigned char mouse_byte1, mouse_byte2, mouse_byte3;

//void plot_pixel(unsigned int x, unsigned int y, unsigned short int line_color);
//void swap(unsigned int *x, unsigned int *y);
//void draw_line(
//        unsigned int x0, 
//        unsigned int y0, 
//        unsigned int x1, 
//        unsigned int y1, 
//        unsigned short int line_color);
//void clear_screen();
//void draw_graph(Square *s, unsigned short int graph_color);
//void update_box(Square *s, Square *sOld1, Square *sOld2);
//void wait_for_vsync(volatile int* pixel_status_ptr);

int main(void)
{
//    volatile int* pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
//    volatile int* pixel_front_buffer_ptr = pixel_ctrl_ptr;
//    volatile int* pixel_back_buffer_ptr = pixel_ctrl_ptr + 0x1;
//    volatile int* pixel_resolution_ptr = pixel_ctrl_ptr + 0x2;
//    volatile int* pixel_status_ptr = pixel_ctrl_ptr + 0x3;
//    
//    // Seed random engine
//    srand((unsigned) time(NULL));

//    // Initialize 8 squares
//    Square s[NUM_SQUARES], sOld1[NUM_SQUARES], sOld2[NUM_SQUARES];
//    for (unsigned int i = 0; i < NUM_SQUARES; ++i) {
//        s[i].pos_x = rand() % (SCREEN_WIDTH - SQUARE_SIZE);
//        s[i].pos_y = rand() % (SCREEN_HEIGHT - SQUARE_SIZE);
//        s[i].delta_x = (rand() % 2) ? 1 : -1;
//        s[i].delta_y = (rand() % 2) ? 1 : -1;
//        
//        // Initialize previous data to current ones
//        sOld2[i] = sOld1[i] = s[i];
//    }
    
//    // Set back buffer
//    *pixel_back_buffer_ptr = SDRAM_BASE; // start of SDRAM memory
//    // Clear back buffer
//    pixel_buffer_start = *pixel_back_buffer_ptr; // point to the back buffer
//    clear_screen();
//    // Write a one to the front buffer to turn on status flag S
//    *pixel_front_buffer_ptr = 0x1;
//    // Wait for swap
//    wait_for_vsync(pixel_status_ptr);
//    
//    // Set front buffer
//    *pixel_back_buffer_ptr = FPGA_ONCHIP_BASE; // start of FPGA On-Chip Memory
//    // Clear front buffer
//    pixel_buffer_start = *pixel_back_buffer_ptr; // point to the back buffer
//    clear_screen();
//    // Write a one to the front buffer to turn on status flag S
//    *pixel_front_buffer_ptr = 0x1;
//    // Wait for swap
//    wait_for_vsync(pixel_status_ptr);
    
    // intialize interrupt services
    init_IRQ();
    
    
    while (1) {
//        // new back buffer
//        pixel_buffer_start = *pixel_back_buffer_ptr; 
//        
//        // Redraw the graph two frames ago with black color
//        draw_graph(sOld2, COLOR_BLACK);
//        
//        // Draw new graph
//        draw_graph(s, COLOR_BLUE);
//        
//        // Update directions and positions
//        update_box(s, sOld1, sOld2);
//        
//        // Write a one to the front buffer to turn on status flag S
//        *pixel_front_buffer_ptr = 1;
//        
//        // poll the status register. if the flag is still on, wait
//        wait_for_vsync(pixel_status_ptr);
    }
}

void wait_for_vsync(volatile int* pixel_status_ptr) {
    while ((*pixel_status_ptr) & 1) {
        ;
    }
}

void draw_graph(Square *s, unsigned short int graph_color) {
    // Draw squares
    for (unsigned int i = 0; i < NUM_SQUARES; ++i) {
        for (unsigned int x = 0; x < SQUARE_SIZE; ++x) {
            for (unsigned int y = 0; y < SQUARE_SIZE; ++y) {
                plot_pixel(s[i].pos_x + x, s[i].pos_y + y, graph_color);
            }
        }
    }
    
    // Draw lines
    for (unsigned int i = 0; i < NUM_SQUARES; ++i) {
        // Get the next square's id
        unsigned int j = (i + 1) % NUM_SQUARES;
        // Start from square centers
        draw_line(
                s[i].pos_x + SQUARE_SIZE / 2, 
                s[i].pos_y + SQUARE_SIZE / 2, 
                s[j].pos_x + SQUARE_SIZE / 2, 
                s[j].pos_y + SQUARE_SIZE / 2,
                graph_color
        );
    }
}

void update_box(Square *s, Square *sOld1, Square *sOld2) {
    // Record previous boxes
    for (unsigned int i = 0; i < NUM_SQUARES; ++i) {
        sOld2[i] = sOld1[i];
        sOld1[i] = s[i];
    }
    
    // Update box moving direction
    for (unsigned int i = 0; i < NUM_SQUARES; ++i) {
        // Update x-direction
        if (s[i].pos_x == 0) {
            s[i].delta_x = 1;
        }
        if (s[i].pos_x + SQUARE_SIZE == SCREEN_WIDTH - 1) {
            s[i].delta_x = -1;
        }
        s[i].pos_x += s[i].delta_x;
        // Update y-direction
        if (s[i].pos_y == 0) {
            s[i].delta_y = 1;
        }
        if (s[i].pos_y + SQUARE_SIZE == SCREEN_HEIGHT - 1) {
            s[i].delta_y = -1;
        }
        s[i].pos_y += s[i].delta_y;
    }
}

void plot_pixel(unsigned int x, unsigned int y, unsigned short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void swap(unsigned int *x, unsigned int *y) {
    unsigned int temp = *x;
    *x = *y;
    *y = temp;
}

void draw_line(
        unsigned int x0, 
        unsigned int y0, 
        unsigned int x1, 
        unsigned int y1, 
        unsigned short int line_color) 
{
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
    
    for (unsigned int x = x0; x <= x1; ++x) {
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

void clear_screen() {
    for (unsigned int x = 0; x < SCREEN_WIDTH; ++x) {
        for (unsigned int y = 0; y < SCREEN_HEIGHT; ++y) {
            plot_pixel(x, y, COLOR_BLACK);
        }
    }
}
