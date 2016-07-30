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

#define TIMESTEP 365
#define Bodies 9
#define TimeStep 86400
#define NUM_ITERATIONS 365
#define DWT_O_CYCCNT 0x00000004
#define INSTRUMENTATION 0
#define GUI_X	1600
#define GUI_Y	900
#define POSITION_MAX 11e11
#define COMPUTATION_PERIOD 10

#if INSTRUMENTATION
volatile int Time[NUM_ITERATIONS];
#endif


#if 1
long double G = 6.67384e-11;
long double M[Bodies] = {1.98855e30, 328.5e21, 4.867e24, 5.97219e24, 639e21, 1.898e27, 568.3e24, 86.81e24, 102.4e24};
long double Px[Bodies] = {0, 5.791e10, 1.082e11, 1.47098161e11, 2.279e11, 3.085e11, 3.73e11, 4.57e11, 5.03e11};
long double Py[Bodies] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
long double Vx[Bodies] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
long double Vy[Bodies] = {0, 41000, 33000, 30300, 24300, 12300, 16000, 15000, 12000};

long double dVx[Bodies],dVy[Bodies];
long double Rx[Bodies][Bodies], Ry[Bodies][Bodies];
long double Dsq[Bodies][Bodies];
long double D[Bodies][Bodies];
long double F[Bodies][Bodies];
long double Fx[Bodies][Bodies];
long double Fy[Bodies][Bodies];

#endif

#if 0
long double G = 6.67384e-11;
long double M[Bodies] = {1.98855e30, 5.97219e24};
long double Px[Bodies] = {0, 1.47098161e11};
long double Py[Bodies] = {0, 0};
long double Vx[Bodies] = {0, 0};
long double Vy[Bodies] = {0, 30300};
#endif


int GUI_Value [Bodies][2];

int Calc_Iteration=0,Send_Interation=0;
int volatile Star_End=0,ch = 'E';
int volatile GUI_Junk_Start=0,GUI_Junk_End=0;
int volatile Delay_Divide = 6000;
int volatile Count=0,IterationCount=0,EarthOrbitCount=1;
int volatile Computation_Visualization=0;

char Long_S[35] = {'1','9','8','8','5','5','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','.','0','0'};
char Int_S1 = '0';
char Int_S2[2] = {'0','0'};
char Int_S3[3] = {'0','0','0'};

char Receive_String[1000];

volatile int IntTemp[4];
volatile long double LongTemp[5];
volatile int Collection_Start = 0,Collection_Done = 0,Collection_Index = 0;
int Weeks;
int Days;
int Num_Bodies;


typedef struct
{
	int X;
	int Y;
}Pixel_Positon;

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


	ch = (char)UARTCharGetNonBlocking(UART0_BASE);
#if 1
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
	SysCtlDelay(SysCtlClockGet() / (1000 * 3));
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
#endif




	if (ch == 'S')
		Star_End = 1;

	if (ch == 'E')
		Star_End = 0;

}

void Send_Cordinates(int Send_Interation)
{
	int i,j;
#if 0
	int Data[Bodies][2][2];

	for(i=0;i<Bodies;i++)
		for(j=0;j<2;j++)
		{
			Data[i][j][0] = (char)((GUI_Value[Send_Interation][i][j])/100);
			Data[i][j][1] = (char)((GUI_Value[Send_Interation][i][j])%100);
		}
#endif

	UARTCharPut(UART0_BASE,243);

	for(i=0;i<Bodies;i++)
		for(j=0;j<2;j++)
		{
			UARTCharPut(UART0_BASE,((char)((GUI_Value[i][j])/100)));

			UARTCharPut(UART0_BASE,((char)((GUI_Value[i][j])%100)));
		}
	//UARTCharPut(UART0_BASE,'F');
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
	long double FxNet[Bodies] = {0,0,0};
	long double FyNet[Bodies] = {0,0,0};

	Pixel_Positon PixelValue;

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

		GUI_Value[i][0] = PixelValue.X;
		GUI_Value[i][1] = PixelValue.Y;
	}

	UARTCharPut(UART0_BASE,243);
	UARTCharPut(UART0_BASE,243);

	for(i=0;i<Bodies;i++)
		for(j=0;j<2;j++)
		{
			UARTCharPut(UART0_BASE,((char)((GUI_Value[i][j])/100)));

			UARTCharPut(UART0_BASE,((char)((GUI_Value[i][j])%100)));
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
	int i,j;

	//FPUEnable();
	//FPULazyStackingEnable();

	// Set clock frequency as 16MHz
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	// Enable clock to Port F
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	// Configure pin 2 of PORT F as GPIO Output
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
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

	SysCtlDelay(SysCtlClockGet() /6000);

#if 0
	while(!GUI_Junk_Start);

	for(i=0;i<6000;i++)
	{
		UARTCharPut(UART0_BASE,'T');
	}
#endif

#if INSTRUMENTATION
	volatile int T1,T2;
	HWREG(NVIC_DBG_INT) |= 0x01000000;            /*enable TRCENA bit in NVIC_DBG_INT*/
	HWREG(DWT_BASE + DWT_O_CYCCNT) = 0;   /* reset the counter */
	HWREG(DWT_BASE) |= 0x01;
#endif
#if 0
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
#endif

#if 1
	for(i=0;i<500;i++)
	{
		for(j=0;j<Bodies;j++)
		{
			//UARTCharPut(UART0_BASE,'D');

			UARTCharPut(UART0_BASE,'~');
			UARTCharPut(UART0_BASE,'~');

			UARTCharPut(UART0_BASE,'~');
			UARTCharPut(UART0_BASE,'~');
		}

	}

#endif
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
	SysCtlDelay(SysCtlClockGet() / (1000 * 3));
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);


	while(1)
	{
		if(Star_End || (Computation_Visualization))
		{
			Count++;
			IterationCount++;

			Calc_Force_Update_Pixel();

			if(Computation_Visualization)
				if(IterationCount > (COMPUTATION_PERIOD))
					break;

			if(!Computation_Visualization)
			{
				UARTCharPut(UART0_BASE,'@');
				//SysCtlDelay(SysCtlClockGet() / (Delay_Divide));
				UARTCharPut(UART0_BASE,((char)(Count/100)));
				UARTCharPut(UART0_BASE,((char)(Count%100)));

				UARTCharPut(UART0_BASE,(EarthOrbitCount));
			}

			if(Count == 365)
			{
				Count = 0;
				EarthOrbitCount++;
			}
		}

	}
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
	while(1);

}
