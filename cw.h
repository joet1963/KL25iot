/*
 * cw.h
 *
 *  Created on: Oct 16, 2015
 *      Author: Magu
 */

#ifndef SOURCES_CW_H_
#define SOURCES_CW_H_

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PWM1.h"
#include "CLS1.h"
//#include "DEC_POINT.h"
//#include "SYNC_LED.h"
//#include "RX_IN.h"
//#include "LED3.h"

//#define SPEED 15
extern const char Time[] ;
extern const char QM[];
extern const char R[] ;
extern const char TEMP[];
extern const char SYNC[];
extern const char POINT[];
extern const char Call[] ;
extern const char CQ[] ;
extern const char WWVB[];
extern const char AM[];
extern const char PM[];

//       ASCII - 0x2C = i
extern const byte pattern[];

extern volatile byte elementdur ;



void sendDITDAH(byte elementtime);
void sendSPACE(byte elementtime);
void Dit(void);
void Dah(void);
//void pointWWVB(void);
byte outcharacter(byte character);
byte outword(const byte * messpoint);

uint8_t CW_ParseCommand(const unsigned char *cmd, bool *handled, const CLS1_StdIOType *io);
uint8_t CW_SendValue(int32_t value, const CLS1_StdIOType *io);


#endif /* SOURCES_CW_H_ */
