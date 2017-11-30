/*
	Main header file
*/

#ifndef __Config__
#define __Config__

#include <stdint.h> 


#define SAMPLERATE 44100    // Hz
// #define SAMPLERATE 32000    // Hz
#define SAMPLETIME 0.023 // Approx sample time in mS.
#define CLIPLEVEL 30000  // Full range is +32767 to -32767
#define CLIPHARD 32767

// PIC port/pins
// PORT A
#define OLEDRES_BIT BIT_0  // RA0
#define CLIP_BIT    BIT_1  // RA1
// BIT_2 Osc
// BIT_3 Osc
#define DACHI_BIT   BIT_4  // RA4

// PORT B
// Outputs
#define OLEDCS_BIT  BIT_0
#define MOSI_BIT    BIT_1   // RB1
#define SPARE_BIT   BIT_2		// RB2
#define OLEDDC_BIT  BIT_3   // RB3
#define OUT_BIT   BIT_5   // RA5
#define SCLK_BIT    BIT_14

// Inputs
#define BTNSELECT_BIT BIT_4 // RB4
#define BTNEFFECT_BIT BIT_7 // RB7
#define ENCA_BIT BIT_8 // RB8
#define ENCB_BIT BIT_9 // RB9
#define ADC_BIT BIT_13 // RB13/AN11
#define ENCBTN_BIT BIT_15 // RB15

#define ENC_MASK ENCA_BIT | ENCB_BIT // Mask for encoder inputs
#define BTN_MASK BTNEFFECT_BIT | BTNSELECT_BIT | ENCBTN_BIT | ADC_BIT // Mask for buttons
#define INPUT_MASK ENC_MASK | BTN_MASK // Mask for all inputs

#define VUTICKLEN 16
#define VUTICKLEN_MASK 0x0f

// Some handy values for screen layout
#define DISP_FEAT_Y 20  // Start point for features area
#define DISP_FEAT_W 123 // MAx width of features area (allowing space for VU meter)
#define DISP_FEAT_INDENT 2  // Start point for features area

// Stupid Arduino pin numbers
#define OLED_RESET 9 // RA0 
#define DACL 0 // RB5 DAC Low byte
#define DACH 18 // RA4 DAC High byte
#define SPI_CLK 7 // RB14
#define OLED_CS 11 // RB0
#define OLED_DC 14 // RB3
#define SPI_MOSI 12 // RB1
#define _DSPI1_MOSI_PIN   12
// End of stupid Arduino pin numbers

#define SYS_FREQ             40000000
#define GetSystemClock()    SYS_FREQ
#define GetPeripheralClock()    (GetSystemClock() / (1 << OSCCONbits.PBDIV))

#define SPI_SPEED_FAST 8000000

#define SPI_FAST() SpiChnSetBitRate(SPI_CHANNEL2, GetPeripheralClock(), SPI_SPEED_FAST)
#define SPI_SLOW() SpiChnSetBitRate(SPI_CHANNEL2, GetPeripheralClock(), 200000)

#endif
