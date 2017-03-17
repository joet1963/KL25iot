/*
 * code.c
 *
 *  Created on: Nov 17, 2015
 *      Author: Magu
 */

#include "code.h"


//       ASCII - 0x2C = i
const byte npattern[] = {0xCE,0x86,0x56,0x94,							 // 0x2C...0x2F
                0xFC,0x7C,0x3C,0x1C,0x0C,0x04,0x84,0xC4,0xE4,0xF4,  // 0,1,2....8,9   0x30...0x39
				0xE2,0xAA,0x80,0x8C,0x80,0x32,0x80,					// 0x3A......0x40  0x80s are NULLs
				0x60,0x88,0xA8,0x90,0x40,0x28,0xD0,0x08,0x20,0x78,   // A,B,C.....   0x41....
				0xB0,0x48,0xE0,0xA0,0xF0,0x68,0xD8,0x50,0x10,0xC0,
				0x30,0x18,0x70,0x98,0xB8,0xC8};                    // ....X,Y,Z     ....0x5A




byte State = doneMESS;
volatile byte shiftchar;
byte *codemesspoint = NULL;
volatile byte MARKtimer = 0;
volatile byte SPACEtimer = 0;
static volatile byte Symboltime = 0;
static volatile byte interSymboltime = 0;
volatile byte SymbolVar = 2;
static volatile byte RXcharacter = 0x80;
byte nSPEED = 15;
byte iSPEED = 12;
byte timeConstant = 120;
byte CHbuffer[2] = {0,0};
static byte DecoderStatus = idle;
static byte DecisionStatus = nobit;
static byte CurrentPosition = waitSYMBOL;
static byte shiftcount;
byte cMessage[32];
static byte FrameStatus;
byte paylbuff[10];
extern word mStimer;
extern word samples[];
extern word delayline[];

byte *cMESSpoint = cMessage;



byte sendChar(char c){

	if(State != doneMESS) return ERR_BUSY;
	CHbuffer[0] = c;
	CHbuffer[1] = '\0';

	codemesspoint = CHbuffer;
	State = initCHAR;

	return ERR_OK;

}

byte sendMess(byte *mess){

	if(State != doneMESS) return ERR_BUSY;

	codemesspoint = mess;
	State = initCHAR;

	return ERR_OK;
}


void CODEprocess(void) // called from RTI every 10mS
{
	byte c, i;

	if (MARKtimer == 0)
	{
		(void)PWM1_Disable();
		LEDR_Off();
		if(SPACEtimer) SPACEtimer--;
	}

		else MARKtimer--;

	if(MARKtimer == 0 && SPACEtimer == 0)
	{	//once both timers are done continue charprocess()

		switch(State)
		{
			case initCHAR:
				c = * codemesspoint;
				switch(c)
				{
					case ('\0'):
						State = doneMESS;
						break;

					case (' '):
						MARKtimer = 0;
						SPACEtimer = (7 * (120/nSPEED));
						State = nextCHAR;
						break;

					case (0x03):     //ETX
						shiftchar =	_ar;
						State = outCHAR;
						break;

					case (0x04):     //EOT
						shiftchar =	_va;
						State = outCHAR;
						break;

					case (0x16):     //SYN
						shiftchar =	_bt;
						State = outCHAR;
						break;

					case (0x10):
						shiftchar =	_bk;
						State = outCHAR;
						break;

					case (0x17):
						shiftchar =	_kn;
						State = outCHAR;
						break;

					case (0x1B):
						shiftchar =	_as;
						State = outCHAR;
						break;

					case (0x08):
						shiftchar =	_aa;
						State = outCHAR;
						break;

					default:
						if (c < 44 || c > 122){
						MARKtimer = 0;
						SPACEtimer = (5 * (120/nSPEED));
						State = nextCHAR;
						break;
						}

					    if (c > 96 && c < 123)	c -= 32;  // convert lower case to upper case

						i = c - 44;  // convert ASCII to index
						shiftchar = npattern[i];
						State = outCHAR;
				}
				break;

			case outCHAR:

				switch(shiftchar)
				{
					case 0x80:
						SPACEtimer = (2 * (120/nSPEED)); // space between chars
						State = nextCHAR;
						break;

					default:
						if (shiftchar & 0x80)
							{
								(void)PWM1_Enable();  // dash is 3 elements
								LEDR_On();
								MARKtimer = (3 * (120/nSPEED));
							}

							else{
									(void)PWM1_Enable();  // dot is 1 element
									LEDR_On();
									MARKtimer = (1 * (120/nSPEED));
								}

						shiftchar =	(shiftchar << 1);  // shift and save for next element
						SPACEtimer = (1 * (120/nSPEED)); // space between elements
						  // process elements
				}
				break;

			case nextCHAR:
				codemesspoint++;
				State = initCHAR;
				break;

			case doneMESS:
				SPACEtimer = (7 * (120/nSPEED)); // space to next message
				break;
		}
	}
}


enum DecisionStatus inCode(void)

{
	byte i = 0;
	byte c;

	//poll input

	if(Bit1_GetVal() == 1)
	{
		Symboltime++;
		DecoderStatus = counting;
		CurrentPosition = waitSYMBOL;  // when symbol is finished need to be in waitSYMBOL state
		interSymboltime = 0;
	}

	else
	{
		interSymboltime++;
		DecoderStatus = idle;
		Symboltime = 0;  // symbol is finished now wait the inter symbol time
	}

	switch(DecoderStatus)
	{
		case idle:

			if((interSymboltime >= ((1 * (timeConstant/iSPEED)) - SymbolVar)) && (interSymboltime <= ((1 * (120/iSPEED)) + SymbolVar)))
				CurrentPosition = nextSYMBOL;

			else if((interSymboltime >= ((5 * (timeConstant/iSPEED)) - SymbolVar)) && (interSymboltime <= ((5 * (120/iSPEED)) + SymbolVar)))
				CurrentPosition = nextLETTER;

			else if((interSymboltime >= ((7 * (timeConstant/iSPEED)) - SymbolVar)))
				CurrentPosition = nextWORD;

			else CurrentPosition = waitSYMBOL;

			switch(CurrentPosition)
			{
				case waitSYMBOL:
					// do nothing here
				break;

				case nextSYMBOL:

					switch(DecisionStatus)
					{
						case nobit:
						// do nothing here
						break;

						case midbit:
						DecisionStatus = nobit;  // don't process bit again
						RXcharacter = 0x80; // clear previous
						shiftcount = 0;
						break;

						case shortbit:
						RXcharacter >> 1;
						RXcharacter = RXcharacter & 0x7F; // shift in a 0
						if(++shiftcount > 7)  // error
						{
							RXcharacter = 0x80; // clear previous
							shiftcount = 0;
						}
						DecisionStatus = nobit;  // don't process bit again
						break;

						case longbit:
						RXcharacter >> 1;
						RXcharacter = RXcharacter | 0x80; // shift in a 1
						if(++shiftcount > 7)  // error
						{
							RXcharacter = 0x80; // clear previous
						    shiftcount = 0;
						}
						DecisionStatus = nobit;  // don't process bit again
						break;

						case errorbit:
						DecisionStatus = nobit;  // don't process bit again
						RXcharacter = 0x80; // clear previous
						shiftcount = 0;
						break;
					}
				break;

				case nextLETTER:

					if(RXcharacter != 0x80)
					{
						i = 0;
						while(RXcharacter != npattern[i] && i < 48) i++;  // look up character in table

						if(i == 48) c = '?';

						else c = i + 44; // convert index to ASCII

						*cMESSpoint = c;
						if(++cMESSpoint > (cMessage + 31)) cMESSpoint = cMessage ;
						RXcharacter = 0x80; // clear previous so we don't process letter again
					}
						break;

				case nextWORD:

					*cMESSpoint = ' ';  // space
					if(++cMESSpoint > (cMessage + 31)) cMESSpoint = cMessage ;
					interSymboltime = 0;  // to keep interSymboltime from overflow and not repeat another space
					CurrentPosition = waitSYMBOL;
					break;
			}
			break;

		case counting:

			if((Symboltime <= ((1 * (timeConstant/iSPEED)) - SymbolVar)))
					DecisionStatus = nobit; // too short

			else if((Symboltime >= ((1 * (timeConstant/iSPEED)) - SymbolVar)) && (Symboltime <= ((1 * (120/iSPEED)) + SymbolVar)))
					DecisionStatus = shortbit;  // short bit

			else if((Symboltime >= ((3 * (timeConstant/iSPEED)) - SymbolVar)) && (Symboltime <= ((3 * (120/iSPEED)) + SymbolVar)))
					DecisionStatus = longbit;  // long bit

			else if(Symboltime >= ((3 * (timeConstant/iSPEED)) + SymbolVar))
				{
					DecisionStatus = errorbit; // too long
					Symboltime--; // to keep Symboltime from overflow
				}

			else DecisionStatus = midbit; // default status
			break;
	}
		return DecisionStatus;
}


byte parseCODE(byte * codestream)
{
	byte k = 0, n;
	byte c, i = 0;
	byte p;


	c = *codestream;

	while ( k <= 31)
	{
		switch(FrameStatus)
		{
		case preamble:

			switch(c)
			{
				case 'V':
				if(++p == 3) FrameStatus = sync;
				break;
				default:
				p = 0; // reset any previous non consecutive sync characters
				break;
			}
			break;

		case sync:

			switch(c)
			{
				case BT:
				FrameStatus = loadsize;
				break;
				default:
				FrameStatus = incomplete;
				break;
			}
			break;

		case loadsize:

			n = c - 30;
			if(n > 0 && n < 10)FrameStatus = payload; // real number?
			else FrameStatus = incomplete;
			break;

		case payload:

			paylbuff[i] = c;
			i++;
			if (--n == 0)
			{
				FrameStatus = endframe;
				paylbuff[i] = '\0';
			}
			break;

		case endframe:

			switch(c)
			{
				case AR:
				FrameStatus = complete;
				k = 31; //force exit
				break;
				default:
				FrameStatus = incomplete;
				k = 31; // force exit
				break;
			}
			break;
		}

		k++;
		codestream++;
	}

}


byte findSPEED(word sample)
{
	word start = 0, stop = 0, lperiod = 0, speriod = sample;
	byte speed_0, speed_1;

	mStimer = 0; // starting time

	while(Bit1_GetVal() == 0 && mStimer < sample){} // wait for rising edge
	start = mStimer;
	while(Bit1_GetVal() == 1 && mStimer < sample){} // falling edge
	stop = mStimer;

	if((stop - start) > lperiod) lperiod = (stop - start);

	speed_1 = (byte)370 / lperiod; // 360 + tolerance factor of 10

	mStimer = 0;

	while(Bit1_GetVal() == 0 && mStimer < sample){} // wait for rising edge
	start = mStimer;
	while(Bit1_GetVal() == 1 && mStimer < sample){} // falling edge
	stop = mStimer;

	if((stop - start) < speriod) speriod = (stop - start);

	speed_0 = (byte)123 / lperiod; // 120 + tolerance factor of 3

	if(speed_0 == speed_1 && speed_0 > 5 && speed_0 < 40 ) return speed_0;
	return 0;
}

int8 findRSSI(void)
{
	if(samples[1] < 4805) return ((samples[1] / 546) - 85);
	return ((samples[1] / 546) - 82);
}

word findNOISE(void)
{
	byte i;
	word lownoise = 0xFFFF, highnoise = 0x0000;
	for(i = 0; i < 32; i++)
	{
		if(delayline[i] > highnoise) highnoise = delayline[i];
		if(delayline[i] < lownoise) lownoise = delayline[i];
	}

	return ((highnoise - lownoise) / 2) + lownoise;
}





