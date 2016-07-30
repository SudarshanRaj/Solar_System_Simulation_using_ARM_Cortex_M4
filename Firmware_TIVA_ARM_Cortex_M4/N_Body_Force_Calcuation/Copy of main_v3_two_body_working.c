#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_nvic.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/fpu.h"

//char a = '0';

//#define TIMESTEP 365
#define Bodies 2
#define TimeStep 86400
#define NUM_ITERATIONS 365
#define DWT_O_CYCCNT 0x00000004
#define INSTRUMENTATION 0
#define GUI_X	1600
#define GUI_Y	870
#define POSITION_MAX 6e11

#if INSTRUMENTATION
volatile int Time[NUM_ITERATIONS];
#endif


#if 0
long double G = 6.67384e-11;
long double M[Bodies] = {1.98855e30, 5.97219e24, 7.3476e22};
long double Px[Bodies] = {0, 1.47098161e11, 1.77098161e11};
long double Py[Bodies] = {0, 0, 10000};
long double Vx[Bodies] = {0, 0, 0};
long double Vy[Bodies] = {0, 30300, 30000};

#endif

#if 1
long double G = 6.67384e-11;
long double M[Bodies] = {1.98855e30, 5.97219e24};
long double Px[Bodies] = {0, 1.47098161e11};
long double Py[Bodies] = {0, 0};
long double Vx[Bodies] = {0, 0};
long double Vy[Bodies] = {0, 30300};

long double dVx[Bodies],dVy[Bodies];
long double Rx[Bodies][Bodies], Ry[Bodies][Bodies];
long double Dsq[Bodies][Bodies];
long double D[Bodies][Bodies];
long double F[Bodies][Bodies];
long double Fx[Bodies][Bodies];
long double Fy[Bodies][Bodies];




#endif

#if INSTRUMENTATION

#endif

int Calc_Iteration=0,Send_Interation=0;
int Star_End,ch = 'E';

int GUI_Value [NUM_ITERATIONS][Bodies][2];


typedef struct
{
	int X;
	int Y;
}Pixel_Positon;

Pixel_Positon PixelValue;

#if INSTRUMENTATION
long double InstrumentationData[NUM_ITERATIONS * 3 * 2];
#endif

void Calc_Force_Update_Pixel(void);
Pixel_Positon Covert_Into_Pixel(long double dValueX, long double dValueY);

void Send_Cordinates(int Send_Interation);

void UARTIntHandler(void)
{
	uint32_t ui32Status;
	// Read UART interrupt and clear it
	ui32Status = UARTIntStatus(UART0_BASE, true);

	UARTIntClear(UART0_BASE, ui32Status);
	//UARTSend((uint8_t *)"\r", 1);

	ch = (char)UARTCharGetNonBlocking(UART0_BASE);
#if 1
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
	SysCtlDelay(SysCtlClockGet() / (1000 * 3));
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
#endif
	if (ch == 'S')
	{
		Send_Cordinates(Send_Interation);
		Send_Interation++;

		if(Send_Interation == NUM_ITERATIONS)
			Send_Interation = 0;

	}

}

void Send_Cordinates(int Send_Interation)
{
int i,j;
	int Data[Bodies][2][2];

	for(i=0;i<Bodies;i++)
			for(j=0;j<2;j++)
			{
				Data[i][j][0] = (int)((GUI_Value[Send_Interation][i][j])/100);
				Data[i][j][1] = (int)((GUI_Value[Send_Interation][i][j])%100);
			}


	//UARTCharPut(UART0_BASE,'D');

	for(i=0;i<Bodies;i++)
			for(j=0;j<2;j++)
			{
				UARTCharPut(UART0_BASE,Data[i][j][0]);

				UARTCharPut(UART0_BASE,Data[i][j][1]);
			}
}


Pixel_Positon Covert_Into_Pixel(long double dValueX, long double dValueY)
{
	// Resoultion of the GUI = GUI_X x GUI_Y
	Pixel_Positon PixelValue;

	PixelValue.X = floor((GUI_X* ((float)(dValueX/POSITION_MAX))));
	PixelValue.Y = floor((GUI_Y* ((float)(dValueY/POSITION_MAX))));

	PixelValue.X = (GUI_X/2) + PixelValue.X;
	PixelValue.Y = (GUI_Y/2) + PixelValue.Y;

	return PixelValue;
}


void Calc_Force_Update_Pixel()
{

	long double FxNet[Bodies] = {0,0};
		long double FyNet[Bodies] = {0,0};

	int i=0,j=0;

	for(i=0;i<Bodies;i++)
		for(j=0;j<Bodies;j++)
		{
			Rx[i][j] = Px[j] - Px[i];
			Ry[i][j] = Py[j] - Py[i];

			if(i==j)
				Dsq[i][j] = 1;
			else
				Dsq[i][j] = (Rx[i][j]*Rx[i][j]) + (Ry[i][j]*Ry[i][j]);

			D[i][j] = sqrt(Dsq[i][j]);

			if(i==j)
				F[i][j]=0;
			else
				F[i][j] = (((G * M[i]) / Dsq[i][j]) * M[j]);

			Fx[i][j] = Rx[i][j] * F[i][j] / D[i][j];
			Fy[i][j] = Ry[i][j] * F[i][j] / D[i][j];

			FxNet[i]+= Fx[i][j];
			FyNet[i]+= Fy[i][j];

		}




	for(i=0;i<Bodies;i++)
	{
		//FxNet[i] = Fx[i][0] + Fx[i][1] + Fx[i][2];
		//FyNet[i] = Fy[i][0] + Fy[i][1] + Fy[i][2];

		dVx[i] = FxNet[i] * TimeStep / M[i];
		dVy[i] = FyNet[i] * TimeStep / M[i];

		Vx[i] = dVx[i] + Vx[i];
		Vy[i] = dVy[i] + Vy[i];

		Px[i] = Px[i] + (Vx[i] * TimeStep);
		Py[i] = Py[i] + (Vy[i] * TimeStep);

	}

	for(i=0;i<Bodies;i++)
	{
		PixelValue = Covert_Into_Pixel(Px[i],Py[i]);

		GUI_Value[Calc_Iteration][i][0] = PixelValue.X;
		GUI_Value[Calc_Iteration][i][1] = PixelValue.Y;

	}

#if INSTRUMENTATION
	for(i=0;i<Bodies;i++)
		InstrumentationData[(Bodies*2*Calc_Iteration)+i] = Px[i];

	for(i=Bodies;i<Bodies*2;i++)
		InstrumentationData[(Bodies*2*Calc_Iteration)+i] = Py[i-Bodies];
#endif
}


int main(void)
 {
	FPUEnable();
	FPULazyStackingEnable();



	// Set clock frequency as 16MHz
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	// Enable clock to Port F
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	// Configure pin 2 of PORT F as GPIO Output
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
	// Enable clock to PORT A and UART0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	// Enable global interrupts
	IntMasterEnable();
	// Configure UART0 pins as RX and TX
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	// Configure pad settings and modes for the UART pins
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	// Configure UART0 for operation - Clock source, BR, Data length, Stop bits and parity
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
					UART_CONFIG_PAR_NONE));
	// Enable interrupts for UART0
	IntEnable(INT_UART0);
	// Enable RX and Receive time-out as interrupt sources for UART0
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);


#if INSTRUMENTATION
	volatile int T1,T2;
	HWREG(NVIC_DBG_INT) |= 0x01000000;            /*enable TRCENA bit in NVIC_DBG_INT*/
	HWREG(DWT_BASE + DWT_O_CYCCNT) = 0;   /* reset the counter */
	HWREG(DWT_BASE) |= 0x01;
#endif

	for(Calc_Iteration=0;Calc_Iteration<NUM_ITERATIONS;Calc_Iteration++)
	{
#if INSTRUMENTATION
		T1 = HWREG(DWT_BASE + DWT_O_CYCCNT);
#endif

		Calc_Force_Update_Pixel();

#if INSTRUMENTATION
		T2 = HWREG(DWT_BASE + DWT_O_CYCCNT);

		Time[Calc_Iteration] = T2 - T1;

		Time[Calc_Iteration] = Time[Calc_Iteration];
#endif
	}
	while(1);
}
