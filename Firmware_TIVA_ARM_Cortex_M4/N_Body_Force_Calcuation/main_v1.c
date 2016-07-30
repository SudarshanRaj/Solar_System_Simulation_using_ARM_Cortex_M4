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
#define Bodies 3
#define TimeStep 86400
#define NUM_ITERATIONS 365
#define DWT_O_CYCCNT 0x00000004
#define INSTRUMENTATION 0

#if INSTRUMENTATION
volatile int Time[NUM_ITERATIONS];
#endif



long double G = 6.67384e-11;

long double M[Bodies] = {1.98855e30, 5.97219e24, 7.3476e22};
long double Px[Bodies] = {0, 1.47098161e11, 1.77098161e11};
long double Py[Bodies] = {0, 0, 10000};
long double Vx[Bodies] = {0, 0, 0};
long double Vy[Bodies] = {0, 30300, 30000};



long double InstrumentationData[NUM_ITERATIONS * 3 * 2];

void Calc_Force(void);


int main(void)
{
	volatile int T1,T2;

	long double dVx[Bodies],dVy[Bodies];

	long double Rx[Bodies][Bodies], Ry[Bodies][Bodies];

	long double Dsq[Bodies][Bodies];
	long double D[Bodies][Bodies];
	long double F[Bodies][Bodies];
	long double Fx[Bodies][Bodies];
	long double Fy[Bodies][Bodies];

	long double FxNet[Bodies];
	long double FyNet[Bodies];

	int i=0,j=0,iteration=0;

	// Set clock frequency as 16MHz
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	// Enable clock to Port F
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

#if INSTRUMENTATION
	HWREG(NVIC_DBG_INT) |= 0x01000000;            /*enable TRCENA bit in NVIC_DBG_INT*/
	HWREG(DWT_BASE + DWT_O_CYCCNT) = 0;   /* reset the counter */
	HWREG(DWT_BASE) |= 0x01;
#endif

	for(iteration=0;iteration<NUM_ITERATIONS;iteration++)
	{
#if INSTRUMENTATION
		T1 = HWREG(DWT_BASE + DWT_O_CYCCNT);
#endif
		for(i=0;i<3;i++)
			for(j=0;j<3;j++)
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

			}

		for(i=0;i<3;i++)
		{
			FxNet[i] = Fx[i][0] + Fx[i][1] + Fx[i][2];
			FyNet[i] = Fy[i][0] + Fy[i][1] + Fy[i][2];

			dVx[i] = FxNet[i] * TimeStep / M[i];
			dVy[i] = FyNet[i] * TimeStep / M[i];

			Vx[i] = dVx[i] + Vx[i];
			Vy[i] = dVy[i] + Vy[i];

			Px[i] = Px[i] + (Vx[i] * TimeStep);
			Py[i] = Py[i] + (Vy[i] * TimeStep);

		}
#if INSTRUMENTATION
		T2 = HWREG(DWT_BASE + DWT_O_CYCCNT);

		Time[iteration] = T2 - T1;

		Time[iteration] = Time[iteration];


		for(i=0;i<3;i++)
			InstrumentationData[(6*iteration)+i] = Px[i];

		for(i=3;i<6;i++)
			InstrumentationData[(6*iteration)+i] = Py[i-3];
#endif
	}


	while(1);



}
