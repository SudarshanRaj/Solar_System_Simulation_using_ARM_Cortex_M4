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

//char a = '0';

#define TIMESTEP 365
#define Bodies 2
#define TimeStep 86400
#define NUM_ITERATIONS 365
#define DWT_O_CYCCNT 0x00000004
#define INSTRUMENTATION 1
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
#endif

long double PxValue[Bodies][NUM_ITERATIONS];
long double PyValue[Bodies][NUM_ITERATIONS];

int GUI_XValue [NUM_ITERATIONS][Bodies];
int GUI_YValue [NUM_ITERATIONS][Bodies];

#if INSTRUMENTATION
int GUI_Value [NUM_ITERATIONS][Bodies][2];
#endif

int Iteration=0;


typedef struct {
	int X;
	int Y;
}Pixel_Positon;

#if INSTRUMENTATION
long double InstrumentationData[NUM_ITERATIONS * 3 * 2];
#endif

void Calc_Force_Update_Pixel(void);
Pixel_Positon Covert_Into_Pixel(long double dValueX, long double dValueY);


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
	long double dVx[Bodies],dVy[Bodies];
	long double Rx[Bodies][Bodies], Ry[Bodies][Bodies];
	long double Dsq[Bodies][Bodies];
	long double D[Bodies][Bodies];
	long double F[Bodies][Bodies];
	long double Fx[Bodies][Bodies];
	long double Fy[Bodies][Bodies];
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
		GUI_XValue[Iteration][i] = PixelValue.X;
		GUI_YValue[Iteration][i] = PixelValue.Y;

#if INSTRUMENTATION
		GUI_Value[Iteration][i][0] = PixelValue.X;
		GUI_Value[Iteration][i][1] = PixelValue.Y;
#endif
	}

#if INSTRUMENTATION
	for(i=0;i<Bodies;i++)
		InstrumentationData[(Bodies*2*Iteration)+i] = Px[i];

	for(i=Bodies;i<Bodies*2;i++)
		InstrumentationData[(Bodies*2*Iteration)+i] = Py[i-Bodies];
#endif
}


int main(void)
{

	// Set clock frequency as 16MHz
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	// Enable clock to Port F
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

#if INSTRUMENTATION
	volatile int T1,T2;
	HWREG(NVIC_DBG_INT) |= 0x01000000;            /*enable TRCENA bit in NVIC_DBG_INT*/
	HWREG(DWT_BASE + DWT_O_CYCCNT) = 0;   /* reset the counter */
	HWREG(DWT_BASE) |= 0x01;
#endif

	for(Iteration=0;Iteration<NUM_ITERATIONS;Iteration++)
	{
#if INSTRUMENTATION
		T1 = HWREG(DWT_BASE + DWT_O_CYCCNT);
#endif

		Calc_Force_Update_Pixel();

#if INSTRUMENTATION
		T2 = HWREG(DWT_BASE + DWT_O_CYCCNT);

		Time[Iteration] = T2 - T1;

		Time[Iteration] = Time[Iteration];
#endif
	}
	while(1);
}
