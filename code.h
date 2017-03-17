/*
 * code.h
 *
 *  Created on: Nov 17, 2015
 *      Author: Magu
 */

#ifndef SOURCES_CODE_H_
#define SOURCES_CODE_H_

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "PWM1.h"
#include "LEDR.h"
#include "LEDG.h"
#include "LEDB.h"
#include "Bit1.h"


#define AR  0x03
#define VA  0x04
#define BT  0x16
#define BK  0x10
#define KN  0x17
#define AS  0x1B
#define AA  0x08



#define _ar  0b01010100
#define _va  0b00010110
#define _bt  0b10001100
#define _bk  0b10001011  // DLE 0x10
#define _kn  0b10110100  //
#define _as  0b01000100  //
#define _aa  0b01011000  // LF 0x0A


enum State{

	initCHAR = 0,
	outCHAR,
	nextCHAR,
	doneMESS
};

enum DecoderStatus{

	idle = 0,
	counting
};

enum DecisionStatus{

	nobit = 0,
	midbit,
	shortbit,
	longbit,
	errorbit
};

enum CurrentPosition{

	waitSYMBOL,
	nextSYMBOL,
	nextLETTER,
	nextWORD,

};

enum FrameStatus{

	preamble = 0,
	sync,
	loadsize,
	payload,
	endframe,
	complete,
	incomplete
};


byte sendMess(byte *mess);
byte sendChar(char c);
void CODEprocess(void);
enum DecisionStatus inCode(void);
byte parseCODE(byte * codestream);
byte findSPEED(word sample);
byte iSPEED;





#endif /* SOURCES_CODE_H_ */
