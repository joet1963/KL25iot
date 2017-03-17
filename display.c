/*
 * display.c
 *
 *  Created on: Nov 9, 2015
 *      Author: Magu
 */

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "HC5951.h"

#include <stdlib.h>
#include <string.h>
#include "ESP8266.h"
#include "Shell.h"
#include "UTIL1.h"
#include "CLS1.h"
#include "AS2.h"
#include "WAIT1.h"
#include "cw.h"

#define  RST 0x01
#define  CE  0x80
#define  WR  0x40

char display[8] = {"        "};
char INstream[128];
word position = 0;





void HC595access(uint8_t data, uint8_t addr){

	HC5951_ShiftByte(data);
	HC5951_ShiftByte((addr>>1) | WR & (~CE) | RST);   // WR must be high,  CE low
	HC5951_Latch();

	HC5951_ShiftByte(data);
	HC5951_ShiftByte((addr>>1) & (~WR) & (~CE) | RST);     // WR low, CE low
	HC5951_Latch();

	HC5951_ShiftByte(data);
	HC5951_ShiftByte((addr>>1) | WR & (~CE) | RST);   // WR must be high, CE low
	HC5951_Latch();

	HC5951_ShiftByte(data);
	HC5951_ShiftByte((addr>>1) | WR | CE | RST);   // WR must be high, CE high
	HC5951_Latch();

}

void resetDisplay(void){
	uint8_t addr = 0x00;

	HC5951_ShiftByte(0x00);
	HC5951_ShiftByte((addr>>1) | WR | CE & (~RST));   // WR must be high,  CE high, Reset low
	HC5951_Latch();

	HC5951_ShiftByte(0x00);
	HC5951_ShiftByte((addr>>1) | WR | CE | RST);     // WR high, CE high,  Reset high
	HC5951_Latch();
}

void controlDisplay(uint8_t intensity, bool flash, bool blink, bool clear ){

	uint8_t addr = 10; // address to access control register
	uint8_t data = 0;

	data |= (intensity & 0x07);  // only 3 bits for intensity
	if (flash) data |= 0x08;
	if (blink) data |= 0x10;
	if (clear) data |= 0x80;

	HC595access(data, addr);

}

uint8_t ledDisplay(char *dataHC595 ){
	uint8_t addr = 00;  // keep Reset line high

	while (dataHC595 != '\0'){

		HC595access(*dataHC595, (addr | 0x18));

		dataHC595++;
		if(++addr > 7) return ERR_RANGE;
	}
	return ERR_OK;
}


uint8_t shiftCHARS(char *inBuff){
	uint8_t j = 0;

	while(position == 0 && j <= 7) {  // pad buffer with space
				display[j] = ' ';
				j++;
		}

		j = 0;
	while (*(inBuff + position) != '\0' && j <= 6){

			display[j] = display[j+1]; // shift data

			j++;
		}

		display[7] = *(inBuff + position );  // new data
		//display[8] = '\0';  // terminate string
		position++;
		if (j < 7) return ERR_RXEMPTY; // reached the end of stream
		return ERR_OK;

}

uint8_t scrollDisplay(char *stream, const CLS1_StdIOType *io){


	if(ERR_OK == shiftCHARS(stream))
	{
		CLS1_SendStr("\n\r", io->stdOut);
		CLS1_SendStr(display, io->stdOut);
		(void)outcharacter(display[7]);

		//(void)ledDisplay(display);
	return ERR_OK;
	}
	//(void)ledDisplay(display);
	//CLS1_SendStr("\n\r", io->stdOut);
	//CLS1_SendStr(display, io->stdOut);
	position = 0; // reset to beginning
	return ERR_RXEMPTY; // reached the end of stream
}


