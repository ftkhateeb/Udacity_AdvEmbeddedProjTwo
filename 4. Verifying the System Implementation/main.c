/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "queue.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"

/*-----------------------------------------------------------*/

/* Tasks Periods */
#define BUTTON_1_MONITOR         50
#define BUTTON_2_MONITOR         50
#define PERIODIC_TRANSMITTER     100
#define UART_RECEIVER            20
#define LOAD_1_SIMULATION        10
#define LOAD_2_SIMULATION        100

/* Tasks Handlers */
TaskHandle_t Button1Monitor_Handler      = NULL;
TaskHandle_t Button2Monitor_Handler      = NULL;
TaskHandle_t PeriodicTransmitter_Handler = NULL;
TaskHandle_t UartReceiver_Handler        = NULL;
TaskHandle_t Load1Simulation_Handler     = NULL;
TaskHandle_t Load2Simulation_Handler     = NULL;
BaseType_t xReturned;

/*Queue Handlers*/
QueueHandle_t xQueue1 = NULL;
QueueHandle_t xQueue2 = NULL;
QueueHandle_t xQueue3 = NULL;

int Button1_TaskIntialTime,  Button1_TaskFinalTime,  Button1_TaskTotalTime;
int Button2_TaskIntialTime,  Button2_TaskFinalTime,  Button2_TaskTotalTime;
int Periodic_TaskIntialTime, Periodic_TaskFinalTime, Periodic_TaskTotalTime;
int Uart_TaskIntialTime,     Uart_TaskFinalTime,     Uart_TaskTotalTime;
int Load1_TaskIntialTime,    Load1_TaskFinalTime,    Load1_TaskTotalTime;
int Load2_TaskIntialTime,    Load2_TaslFinalTime,    Load2_TaskTotalTime;

int System_Time;
int CPU_Load;


/*-----------------------------------------------------------*/
/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );

/*-----------------------------------------------------------*/

/*Tasks Implemntations*/ 
void Button_1_Monitor( void * pvParameters )
{
	pinState_t Button1_NewState;
	pinState_t  Button1_OldState = GPIO_read(PORT_0 , PIN0);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	signed char Edge = 0;

	for( ;; )
	{
		
		/* Read GPIO Input */
		Button1_NewState = GPIO_read(PORT_0 , PIN0);
		
		/*Check for Edges*/
		if( Button1_NewState == PIN_IS_HIGH &&  Button1_OldState == PIN_IS_LOW)
		{
			/*Positive Edge*/
			Edge = 'P';
		}
		else if (Button1_NewState == PIN_IS_LOW &&  Button1_OldState == PIN_IS_HIGH)
		{
			
			/*Negative Edge*/
			Edge = 'N';
		}
		else
		{
			Edge = '.';
		}
		/*Update Button State*/
		 Button1_OldState = Button1_NewState;
		
		/*Send Data to consumer*/
		xQueueOverwrite( xQueue1 , &Edge );
		//xQueueSend( xQueue1 , &Edge  ,0);

		/*Periodicity: 50*/

		vTaskDelayUntil( &xLastWakeTime , BUTTON_1_MONITOR);
	}
}

void Button_2_Monitor( void * pvParameters )
{
	pinState_t  Button2_OldState = GPIO_read(PORT_0 , PIN1);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	signed char Edge = 0;
	pinState_t Button2_NewState;

	for( ;; )
	{
		/* Read GPIO Input */
		Button2_NewState = GPIO_read(PORT_0 , PIN1);
		/*Check for Edges*/
		if( Button2_NewState == PIN_IS_HIGH &&  Button2_OldState == PIN_IS_LOW)
		{
			/*Positive Edge*/
			Edge = 'P';
		}
		else if (Button2_NewState == PIN_IS_LOW &&  Button2_OldState == PIN_IS_HIGH)
		{
			/*Negative Edge*/
			Edge = 'N';
			
		}
		else
		{
			Edge = '.';
		}
		/*Update Button State*/
		 Button2_OldState = Button2_NewState;
		
		/*Send Data to consumer*/
		xQueueOverwrite( xQueue2 , &Edge );
		//xQueueSend( xQueue2 , &Edge  ,0);
			
		/*Periodicity: 50*/
		
		vTaskDelayUntil( &xLastWakeTime , BUTTON_2_MONITOR);
	}
}


void Periodic_Transmitter (void * pvParameters )
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	uint8_t i = 0;
	char Pc_String[28];
	strcpy(Pc_String, "\nPeriodic Transmitter 100ms.");

	for( ; ; )
	{
		/*Send string characters over Queue xQueue3 to Uart_Receiver*/
		for( i = 0 ; i < 28 ; i++)
		{
			xQueueSend( xQueue3 , Pc_String+i ,100);
		}
		
		/*Periodicity: 100*/
	
		vTaskDelayUntil( &xLastWakeTime , PERIODIC_TRANSMITTER);
	}
}

void Uart_Receiver (void * pvParameters )
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	signed char Button1;
	signed char Button2;
	char Rx_String[28];
	
	uint8_t i = 0;
	for( ; ; )
	{
		/*Receive Button 1 State*/
		if( xQueueReceive( xQueue1, &Button1 , 0) && Button1 != '.')
		{
			/*Transmit if +/- edge detected*/
			xSerialPutChar('\n');		
			if(Button1=='P')
			{
							char String2[27]=" Button 1: Rising edge \n";
							vSerialPutString((signed char *) String2, strlen(String2));
			}
			else
			{
							char String3[27]=" Button 1: Falling edge \n";
							vSerialPutString((signed char *) String3, strlen(String3));
			}
		}
		else
		{
			/*Transmit spaces to take the same execution time 
			*even if Received nothing */
			xSerialPutChar(' ');
			xSerialPutChar(' ');
			xSerialPutChar(' ');
			xSerialPutChar(' ');
			xSerialPutChar(' ');
		}
		
		/*Receive Button 2 state*/
		if( xQueueReceive( xQueue2, &Button2 , 0) && Button2 != '.')
		{
			/*Transmit if +/- edge detected*/
			xSerialPutChar('\n');		
			if(Button2=='P')
			{
							char String2[27]=" Button 2: Rising edge \n";
							vSerialPutString((signed char *) String2, strlen(String2));
			}
			else
			{
							char String3[27]=" Button 2: Falling edge \n";
							vSerialPutString((signed char *) String3, strlen(String3));
			}
		}
		else
		{
			/*Transmit spaces to take the same execution time 
			*even if Received nothing */
			xSerialPutChar(' ');		
			xSerialPutChar(' ');
			xSerialPutChar(' ');
			xSerialPutChar(' ');
			xSerialPutChar(' ');
		}
		
		/*Receive String from Periodic_Transmitter*/
		if( uxQueueMessagesWaiting(xQueue3) != 0)
		{
			for( i = 0 ; i < 28 ; i++)
			{
				xQueueReceive( xQueue3, (Rx_String+i) , 0);
			}
			vSerialPutString( (signed char *) Rx_String, strlen(Rx_String));
			xQueueReset(xQueue3);
		}
		/*Periodicity: 20*/
		vTaskDelayUntil( &xLastWakeTime , UART_RECEIVER);
	}
}
	




void Load_1_Simulation ( void * pvParameters )
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	uint32_t i = 0;
	uint32_t Period = 12000*5; /* (XTAL / 1000U)*time_in_ms  */
	for( ; ; )
	{
		for( i = 0 ; i <= Period; i++)
		{
			/*5ms delay*/
		}
		/*Periodicity: 10*/
		vTaskDelayUntil( &xLastWakeTime , LOAD_1_SIMULATION);

	}
}

void Load_2_Simulation ( void * pvParameters )
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	uint32_t i = 0;
	uint32_t Period = 12000*12; /* (XTAL / 1000U)*time_in_ms  */
		
	for( ; ; )
	{		
		for( i = 0 ; i <= Period; i++)
		{
			/*12ms delay*/
		}

		/*Periodicity: 100*/
		vTaskDelayUntil( &xLastWakeTime , LOAD_2_SIMULATION);
	
	}
}
 
 
 
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	
	xQueue1 = xQueueCreate( 1, sizeof(char) );
	xQueue2 = xQueueCreate( 1, sizeof(char) );
	xQueue3 = xQueueCreate( 28, sizeof(char) );

    /* Create Tasks here */
	xTaskPeriodicCreate(
			Button_1_Monitor,                  /* Function that implements the task. */
			"BUTTON 1 MONITOR",                /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Button1Monitor_Handler,       /* Used to pass out the created task's handle. */
			BUTTON_1_MONITOR);     /* Period for the task */

	xTaskPeriodicCreate(
			Button_2_Monitor,                  /* Function that implements the task. */
			"BUTTON 2 MONITOR",                /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Button2Monitor_Handler,       /* Used to pass out the created task's handle. */
			BUTTON_2_MONITOR);     /* Period for the task */

	xTaskPeriodicCreate(
			Periodic_Transmitter,               /* Function that implements the task. */
			"PERIODIC TRANSMITTER",             /* Text name for the task. */
			100,                                /* Stack size in womain.crds, not bytes. */
			( void * ) 0,                       /* Parameter passed into the task. */
			1,                                  /* Priority at which the task is created. */
			&PeriodicTransmitter_Handler,   /* Used to pass out the created task's handle. */
			PERIODIC_TRANSMITTER);  /* Period for the task */

	xTaskPeriodicCreate(
			Uart_Receiver,                      /* Function that implements the task. */
			"UART RECEIVER",                    /* Text name for the task. */
			100,                                /* Stack size in words, not bytes. */
			( void * ) 0,                       /* Parameter passed into the task. */
			1,                                  /* Priority at which the task is created. */
			&UartReceiver_Handler,          /* Used to pass out the created task's handle. */
			UART_RECEIVER);         /* Period for the task */

	xTaskPeriodicCreate(
			Load_1_Simulation,                 /* Function that implements the task. */
			"LOAD 1 SIMULATION",               /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Load1Simulation_Handler,      /* Used to pass out the created task's handle. */
			LOAD_1_SIMULATION);	   /* Period for the task */

	xTaskPeriodicCreate(
			Load_2_Simulation,                 /* Function that implements the task. */
			"LOAD 1 SIMULATION",               /* Text name for the task. */
			100,                               /* Stack size in words, not bytes. */
			( void * ) 0,                      /* Parameter passed into the task. */
			1,                                 /* Priority at which the task is created. */
			&Load2Simulation_Handler,      /* Used to pass out the created task's handle. */
			LOAD_2_SIMULATION); 	 /* Period for the task */

	
		
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();
	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook (void)
{
	GPIO_write(PORT_0,PIN9,PIN_IS_HIGH);
	GPIO_write(PORT_0,PIN9,PIN_IS_LOW);

}

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000; //20 kHhZ
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/
