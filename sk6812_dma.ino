// Chipkit DP32
// Uses DMA and OC module output to drive 32bit (WRGB) SK6812 LEDs
// OC output is on pin 18 (RA4)
// Debug output DMA loading on RB0 - 2
// Main blink on RB3

#include "config.h"

#define LEDCOUNT 4 // Number of leds inside each pixel
#define BITBYTES LEDCOUNT*8  // How many bytes doe we need to DMA : 1 byte per bit  (24byte for RGB, 32 for WRGB)
#define HALFBITBYTES BITBYTES/2
#define PIXELCOUNT 60 // Number of complete LED units in the string

#define LO_RS 12 // OCxRS for a 0 bit : 300ns
#define HI_RS 27 // OCxRS for a 1 bit : 600ns
#define PERIOD 49 // OCx Timer period : 1250ns

typedef union {
  struct {
    uint8_t white;  // Option : Only for WRGB
    uint8_t blue;
    uint8_t red;
    uint8_t green;
  };
  uint32_t rawdata;
  uint8_t rawbytes[4]; // 3 for RGB, 4 for WRGB
} pixel_t;

pixel_t leds[PIXELCOUNT] = {
		{0x40, 0x00, 0x00, 0x10}
	, {0x00, 0x40, 0x00, 0x00}
	, {0x00, 0x40, 0x20, 0x00}
	, {0x10, 0x20, 0x40, 0x00}
	, {0x00, 0x00, 0x40, 0x20}
	, {0x00, 0X40, 0x20, 0x20}
	, {0x00, 0x20, 0x10, 0x40}
	, {0x00, 0x20, 0X40, 0x20}
	, {0x40, 0x00, 0x20, 0x00}
	, {0x10, 0x40, 0x20, 0x10}
	, {0x00, 0x20, 0x20, 0x00}
	, {0x00, 0x20, 0x40, 0x00}
	, {0x20, 0X00, 0x20, 0x20}
	, {0x00, 0x00, 0x10, 0x40}
	, {0x40, 0x20, 0X00, 0x20}
	, {0x40, 0x00, 0x20, 0x00}
	, {0x00, 0x40, 0x00, 0x00}
	, {0x00, 0x20, 0x20, 0x00}
	, {0x10, 0x00, 0x40, 0x00}
	, {0x00, 0X00, 0x20, 0x20}
	, {0x00, 0x00, 0x10, 0x40}
	, {0x00, 0x20, 0X00, 0x20}
	, {0x40, 0x00, 0x00, 0x20}
	, {0x00, 0x40, 0x00, 0x00}
	, {0x20, 0x20, 0x20, 0x00}
	, {0x00, 0x00, 0x40, 0x10}
	, {0x00, 0X40, 0x20, 0x20}
	, {0x00, 0x00, 0x20, 0x40}
	, {0x00, 0x20, 0X00, 0x20}
	, {0x40, 0x00, 0x00, 0x00}
	, {0x00, 0x40, 0x00, 0x00}
	, {0x00, 0x20, 0x20, 0x00}
	, {0x00, 0x00, 0x40, 0x00}
	, {0x00, 0X00, 0x20, 0x20}
	, {0x00, 0x00, 0x00, 0x40}
	, {0x10, 0x20, 0X10, 0x20}
	, {0x40, 0x00, 0x00, 0x00}
	, {0x00, 0x40, 0x00, 0x00}
	, {0x00, 0x20, 0x20, 0x10}
	, {0x00, 0x00, 0x40, 0x00}
	, {0x00, 0X10, 0x20, 0x20}
	, {0x00, 0x00, 0x00, 0x40}
	, {0x00, 0x20, 0X00, 0x20}
	, {0x40, 0x00, 0x00, 0x00}
	, {0x00, 0x40, 0x00, 0x00}
	, {0x00, 0x20, 0x20, 0x00}
	, {0x00, 0x00, 0x40, 0x00}
	, {0x10, 0X10, 0x20, 0x20}
	, {0x00, 0x00, 0x00, 0x40}
	, {0x00, 0x20, 0X00, 0x20}
	, {0x40, 0x00, 0x00, 0x00}
	, {0x00, 0x40, 0x00, 0x10}
	, {0x00, 0x20, 0x20, 0x00}
	, {0x00, 0x10, 0x40, 0x00}
	, {0x00, 0X00, 0x20, 0x20}
	, {0x00, 0x00, 0x00, 0x40}
	, {0x00, 0x20, 0X00, 0x20}
	, {0x40, 0x10, 0x00, 0x00}
	, {0x00, 0x40, 0x00, 0x10}
	, {0x00, 0x20, 0x20, 0x00}
};
// For animating the values
int8_t ddr[PIXELCOUNT][LEDCOUNT];
uint8_t maxintens[LEDCOUNT] = {
	  0b00001111
	, 0b01111111
	, 0b01111111
	, 0b01111111
};
uint8_t maxspeed[LEDCOUNT] = {
	  0b00000001
	, 0b00000111
	, 0b00000111
	, 0b00000111
};


unsigned int sourceAddr;
unsigned int destinationAddr;

// a byte per led bytes plus one to end it
uint8_t junkdata[BITBYTES];
uint8_t ledctr = 0;

extern "C" void __attribute__((interrupt(),nomips16)) DMA1Handler(void);

void setup() {
	ANSELA = 0;  // All port A as digital
	ANSELB = 0;  // All port B as digital
	TRISA = 0;
  TRISB = 0;
  // TRISBSET = ENCA_BIT | ENCB_BIT | BTNEFFECT_BIT | BTNSELECT_BIT | ENCBTN_BIT | ADC_BIT;


	///// Virtual to physical memory

	/* using a pointer returns virtual memory address:
	 *      eg. const unsigned int* src = (void*) &LATA;
	 * returns 0xBF886030 -> agrees with datasheet
	 *
	 * reference manual says that to convert this to physical
	 *      address, AND it with 0x1FFFFFFF and get the
	 *      corresponding physical address
	 */	
	sourceAddr = (unsigned int) &junkdata & 0x1FFFFFFF; 
	destinationAddr = (unsigned int) &OC4RS & 0x1FFFFFFF; 
	// destinationAddr = (unsigned int) &LATB & 0x1FFFFFFF;    // Physical address of LATA

	///// Initialize dma first
	DMACON = 0x8000;            // dma module on
	DCRCCON = 0;                // crc module off
	
	DCH1INT = 0;                // interrupts disabled
	// DCH1INTbits.CHBCIE = 1;  // Int at end of transfer
	DCH1INTbits.CHSDIE = 1;  // Int at source done
	DCH1INTbits.CHSHIE = 1;  // Int at source half empty
	
	DCH1SSA = sourceAddr;       // source start address
	DCH1DSA = destinationAddr;  // destination start address
	
	DCH1SSIZ = BITBYTES;               // source size
	DCH1DSIZ = 1;               // destination size - 1 byte
	DCH1CSIZ = 1;  
	             // cell size - 1 bytes
	DCH1ECONbits.CHSIRQ = _TIMER_3_IRQ;  // Cell transfer IRQ
	DCH1ECONbits.SIRQEN = 1;  // Transfer a cell when CHSIRQ occurs

	///// Enable dma channel
	DCH1CONbits.CHEN = 0; 
	DCH1CONbits.CHAEN = 1;  // Auto enable
	DCH1CONbits.CHPRI = 3;

	mapPps(18, PPS_OUT_OC4);
	// Setup T3 to run flat out with period of 256
	T3CONbits.TCKPS = 0; // prescale
	TMR3 = 0;
	PR3 = PERIOD;
	
	//	Setup OC for free running PWM
	OC4CON = OCCON_ON | OCCON_SRC_TIMER3 | OCCON_PWM_FAULT_DISABLE;
	OC4R = 0;
	OC4RS = 0;

  setIntVector(_DMA_1_VECTOR, DMA1Handler);
  clearIntFlag(_DMA1_IRQ);
  setIntPriority(_DMA_1_VECTOR, _CN_IPL_IPC, _CN_SPL_IPC);
  setIntEnable(_DMA1_IRQ);
	
	uint8_t idx = 0;
	uint8_t jdx = 0;

	// Intro blink to let me know things are working
	LATBSET = BIT_0;
	// delay(250);
	// idx = 100;
	// while(idx--){
	// 	LATBINV = BIT_0;
	// 	delay(25);	
	// }
	T3CONbits.ON = 1;
	
	LATBCLR = BIT_1;
	// Preload LEDs with random(ish) data
	// Grab value of TMR3
	// for(idx=0; idx<PIXELCOUNT; idx++){
	// 	LATBINV = BIT_1;
	// 	for(jdx=0; jdx<LEDCOUNT; jdx++){
	// 		// Set direction
	// 		ddr[idx][jdx] = ((TMR3 & maxspeed[jdx]) + 1);
	// 		// set value (restricted to half intensity)
	// 		leds[idx].rawbytes[jdx] = (uint8_t)TMR3 & maxintens[jdx];
	// 		delay(11);
	// 	}
	// 	delay(17);
	// }
	// preload LEDs with zeros
	for(idx=0; idx<PIXELCOUNT; idx++){
		LATBINV = BIT_1;
		for(jdx=0; jdx<LEDCOUNT; jdx++){
			leds[idx].rawbytes[jdx] = 0;
			delay(3);
		}
		delay(7);
	}

	LATBCLR = BIT_1;
	LATBCLR = BIT_0;


	delay(100);

	DCH1CONbits.CHEN = 1;
}

void loop() {
	uint8_t pdx = 0;
	uint8_t idx = 0;
	int32_t value;
	uint8_t bulletcount = 4;
	pixel_t bullets[bulletcount];
	uint8_t bulletpos[4] = {PIXELCOUNT,PIXELCOUNT,PIXELCOUNT,PIXELCOUNT};
	uint8_t launchcheck = 0;
	// A number of rolling dots
	while(1){
		LATBINV = BIT_3;

		// Decay
		for(pdx=0; pdx<PIXELCOUNT; pdx++){
			for(idx=0; idx<LEDCOUNT; idx++){
				value = leds[pdx].rawbytes[idx] - 7;
				if(value < 0){
					value = 0;
				}
				leds[pdx].rawbytes[idx] = (uint8_t)value;
			}
		}
		
		// If a bullet is in play
		for(idx=0; idx<bulletcount; idx++){
			if(bulletpos[idx] < PIXELCOUNT){
				// We have a live one
				// leds[bulletpos[idx]].rawbytes[idx] = 0x11;
				for(pdx=0; pdx < LEDCOUNT; pdx++){
					leds[bulletpos[idx]].rawbytes[pdx] = bullets[idx].rawbytes[pdx];
				}
				bulletpos[idx]++;
			}
		}

		// A chance at making a live one
		if(bulletpos[launchcheck] >= PIXELCOUNT){
			if(random(55) == 0){
				bulletpos[launchcheck] = 0;
				// Set the colour here
				for(pdx=0; pdx < LEDCOUNT; pdx++){
					bullets[launchcheck].rawbytes[pdx] = random(0x61);
				}
			}
			launchcheck++;
			if(launchcheck >= bulletcount){
				launchcheck = 0;
			}
		}

		delay(12);
	}



	// Random fade in/out
	// while(1){	
	// 	LATBINV = BIT_3;
	// 	for(pdx=0; pdx<PIXELCOUNT; pdx++){
	// 		for(idx=0; idx<LEDCOUNT; idx++){
				
	// 			value = leds[pdx].rawbytes[idx] + ddr[pdx][idx];
				
	// 			if(value > maxintens[idx]){
	// 				// subtract
	// 				value = maxintens[idx];
	// 				ddr[pdx][idx] = 0 - ((TMR3 & maxspeed[idx])+2);
	// 			}
	// 			if(value < 0){
	// 				value = 0;
	// 				ddr[pdx][idx] = 1 + (TMR3 & maxspeed[idx]);
	// 			}
	// 			leds[pdx].rawbytes[idx] = (uint8_t)value;
	// 		}
	// 	}
	// 	delay(50);	
	// }
}


void __attribute__((interrupt(),nomips16)) DMA1Handler(void){
	uint8_t idx, nextled;
	uint32_t val;
	LATBSET = BIT_1;
	LATBCLR = BIT_2;
	if(ledctr < PIXELCOUNT){
		if(DCH1INTbits.CHSHIF){
			LATBCLR = BIT_0;
			// Source half
			// Fill bits 0 - 11
//			val = leds[ledctr].rawdata << 8;  // For RGB we need to ignore the first 8 bits
			val = leds[ledctr].rawdata;
			for(idx=0; idx < HALFBITBYTES; idx++){  // Half number of bits
				LATBINV = BIT_0;
				// Test highest bit
				if(val & 0x80000000){
					// a one
					junkdata[idx] = HI_RS;
				}else{
					// a zero
					junkdata[idx] = LO_RS;
				}
				val <<= 1;
			}
		}
		if(DCH1INTbits.CHSDIF){
			// Source done
			// Fill bits 12 - 23
			// Next LED	
//			val = leds[ledctr].rawdata << 20;  // For RGB we need to shift
			val = leds[ledctr].rawdata << 16;
			for(idx=HALFBITBYTES; idx < BITBYTES; idx++){ // Half number of bits
				LATBINV = BIT_2;
				// Test highest bit
				if(val & 0x80000000){
					// a one
					junkdata[idx] = HI_RS;
				}else{
					// a zero
					junkdata[idx] = LO_RS;
				}
				val <<= 1;
			}
			ledctr++;
		}
	}else{
		if(DCH1INTbits.CHSDIF){
			// OC4RS = 0;
			// OC4R = 0;
			// RESET
			// Send some empty LEDS to trigger reset
			for(idx = 0; idx < BITBYTES+1; idx++){
				junkdata[idx] = 0;
			}
			if(ledctr>PIXELCOUNT+1){
				ledctr = 0;
			}else{
				ledctr++;
			}
		}
	}

	DCH1INTCLR = 0x000000ff; // Clear DMA int flags (bottom 8 bits)
	DCH1CONbits.CHEN = 1;	// Re-enable dma

	// DCH1INTbits.CHBCIE = 1;  // Int at end of transfer
	LATBCLR = BIT_2;
	LATBCLR = BIT_1;
	clearIntFlag(_DMA1_IRQ);
}
