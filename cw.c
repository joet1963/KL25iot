/*
 * cw.c
 *
 *  Created on: Oct 16, 2015
 *      Author: Magu
 */

#include "cw.h"
#include "UTIL1.h"
#include "WAIT1.h"
#include "LEDR.h"
#include "LEDG.h"
#include "LEDB.h"
//#include "RFkey.h"

const char Time[] = "QTR";
const char QM[] = "?";
const char R[] = "RR";
const char TEMP[] = "TEMP IS";
const char AM[] = "AM";
const char PM[] = "PM";
const char SYNC[] = "SYNC";
const char POINT[] = "POINT";
const char Call[] = "VA3VCO";
const char CQ[] = "CQ CQ CQ DE";
const char WWVB[] ="WWVB";

extern uint16_t pointTimer;
uint8_t SPEED = 15;

//       ASCII - 0x2C = i
const byte pattern[] = {0xCE,0x86,0x56,0x94,							 // 0x2C...0x2F
                0xFC,0x7C,0x3C,0x1C,0x0C,0x04,0x84,0xC4,0xE4,0xF4,  // 0,1,2....8,9   0x30...0x39
				0xE2,0xAA,0x80,0x8C,0x80,0x32,0x80,					// 0x3A......0x40  0x80s are NULLs
				0x60,0x88,0xA8,0x90,0x40,0x28,0xD0,0x08,0x20,0x78,   // A,B,C.....   0x41....
				0xB0,0x48,0xE0,0xA0,0xF0,0x68,0xD8,0x50,0x10,0xC0,
				0x30,0x18,0x70,0x98,0xB8,0xC8};                    // ....X,Y,Z     ....0x5A

void sendDITDAH(byte elementtime)
{
	elementdur = elementtime * (120/SPEED);
	(void)PWM1_Enable();
	LEDR_On();
	//LEDG_On();
	//LEDB_On();
//	DEC_POINT_SetVal();

	while(elementdur){ }  //wait for element to finish
	(void)PWM1_Disable();
	LEDR_Off();
	//LEDG_Off();
	//LEDB_Off();
//	DEC_POINT_ClrVal();

}

void sendSPACE(byte elementtime)
{
	elementdur = elementtime * (120/SPEED);
	while(elementdur){ }  //wait for space
}

byte outcharacter(byte character)
{
     byte i;
	if(character == 0x20){  // trap whitespace
		sendSPACE(7);
		return ERR_OK ;
	}

	if(character < 44 || character > 122){  // out of range
		sendSPACE(5);
		return ERR_VALUE ;
	}

	if(character > 96 && character < 123) character -= 32;  // convert lower case to upper case
     i = character - 0x2C;  // convert ASCII to index
    character = pattern[i];
	while(character != 0x80)  // CW NULL?
	{
	 if (character & 0x80) sendDITDAH(3);  // dash is 3 elements
	 	 else sendDITDAH(1);  // dot is 1 element

	character =	(character << 1);
	sendSPACE(1); // space between elements
	}
return ERR_OK ;

}

byte outword(const byte *messpoint)
{
	byte temp, errors;

	temp = * messpoint;
	while(temp != '\0') {

		errors = outcharacter(temp);
		messpoint++;
		temp = * messpoint;
		sendSPACE(2); // space between letters
	}
	sendSPACE(7);  //space between words
	return errors;
}



static uint8_t CW_PrintHelp(const CLS1_StdIOType *io) {
  CLS1_SendHelpStr("CW", "CW commands\r\n", io->stdOut);
  CLS1_SendHelpStr("  help|status", "Print help or status information\r\n", io->stdOut);
  CLS1_SendHelpStr("  vary <speed>", "Vary speed\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t CW_PrintStatus(const CLS1_StdIOType *io) {
  uint8_t buf[24];

  CLS1_SendStatusStr("CW", "\r\n", io->stdOut);

  UTIL1_Num32uToStr(buf, sizeof(buf), SPEED);
  UTIL1_strcat(buf, sizeof(buf), "\r\n");
  CLS1_SendStatusStr("  CW Speed", buf, io->stdOut);
  return ERR_OK;
}

uint8_t CW_ParseCommand(const unsigned char *cmd, bool *handled, const CLS1_StdIOType *io) {
  uint8_t val;
  uint8_t res;
  const unsigned char *p;

  if (UTIL1_strcmp((char*)cmd, CLS1_CMD_HELP)==0 || UTIL1_strcmp((char*)cmd, "CW help")==0) {
    *handled = TRUE;
    res = CW_PrintHelp(io);
  } else if (UTIL1_strcmp((char*)cmd, CLS1_CMD_STATUS)==0 || UTIL1_strcmp((char*)cmd, "CW status")==0) {
    *handled = TRUE;
    res = CW_PrintStatus(io);
  } else if (UTIL1_strncmp((char*)cmd, "CW vary ", sizeof("CW vary ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("CW vary ")-1;
    res = UTIL1_ScanDecimal8uNumber(&p, &val);
    if (res==ERR_OK) {
        if(val>4 && val<51){
    	SPEED = val;
        CLS1_SendStr("Speed: OK!\r\n", io->stdOut);
        (void)outword("OK");
        }
        else {   CLS1_SendStr("Speed: Out of Range!\r\n", io->stdOut);
               (void)outcharacter('?');
        }

    }
  }
  return res;
}
