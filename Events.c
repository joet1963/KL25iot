/* ###################################################################
**     Filename    : Events.c
**     Project     : KL25Ziot
**     Processor   : MKL25Z128VLK4
**     Component   : Events
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2015-09-25, 11:45, # CodeGen: 0
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Contents    :
**         Cpu_OnNMIINT - void Cpu_OnNMIINT(void);
**
** ###################################################################*/
/*!
** @file Events.c
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         
/* MODULE Events */

#include "Cpu.h"
#include "Events.h"
#include "code.h"

#ifdef __cplusplus
extern "C" {
#endif 


/* User includes (#include below this line is not maintained by Processor Expert) */

bool send = 0;
word measINTerval = 10;
word samples[2];
word delayline[32];
byte delayindex = 0;
volatile byte elementdur = 0;
volatile word timeadjperiod = 300;
volatile word mStimer = 0;

/*
** ===================================================================
**     Event       :  Cpu_OnNMIINT (module Events)
**
**     Component   :  Cpu [MKL25Z128LK4]
*/
/*!
**     @brief
**         This event is called when the Non maskable interrupt had
**         occurred. This event is automatically enabled when the [NMI
**         interrupt] property is set to 'Enabled'.
*/
/* ===================================================================*/
void Cpu_OnNMIINT(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  TI1_OnInterrupt (module Events)
**
**     Component   :  TI1 [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void TI1_OnInterrupt(void)
{
  /* Write your code here ... */
	//measINTerval--;
	enum DecisionStatus result;

	mStimer++;

	if(--measINTerval == 0){
		measINTerval = 10; // 15 seconds
		(void)AD1_GetValue16((word*) samples );
		send = 1;
	}

	if(elementdur) elementdur--;  // element timing
	 CODEprocess();

	 result = inCode();

	 if(result == errorbit)
	 {
		 if(--timeadjperiod == 0)
		 {
			 iSPEED--;
			 timeadjperiod = 300;
		 }
	 }

	 else if(result == shortbit || result == longbit )
		 {
		 	 if(++timeadjperiod > 300)
		 	 {
		 		 timeadjperiod = 300;
		 	 }
		 }

	 else if(result == nobit )
	 {
		 if(--timeadjperiod == 0)
		 {
			 iSPEED++;
			 timeadjperiod = 300;
		 }
	 }
}

/*
** ===================================================================
**     Event       :  AD1_OnEnd (module Events)
**
**     Component   :  AD1 [ADC]
**     Description :
**         This event is called after the measurement (which consists
**         of <1 or more conversions>) is/are finished.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void AD1_OnEnd(void)
{
  //send = 1;/* Write your code here ... */
  //LEDG_Off();
 // LEDR_On();

	delayline[delayindex] = samples[1];
	delayindex = (delayindex + 1) & 31;  // point to next position in circular buffer


}

/*
** ===================================================================
**     Event       :  AD1_OnCalibrationEnd (module Events)
**
**     Component   :  AD1 [ADC]
**     Description :
**         This event is called when the calibration has been finished.
**         User should check if the calibration pass or fail by
**         Calibration status method./nThis event is enabled only if
**         the <Interrupt service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void AD1_OnCalibrationEnd(void)
{
  /* Write your code here ... */
}

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
