/****************************************************************************
 ************                                                    ************
 ************                   Z61_SIMP                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z61_simp.c
 *       \author rla
 *        $Date: 2009/03/02 15:52:39 $
 *    $Revision: 1.2 $
 *
 *       \brief  Simple example program for the Z61 driver
 *
 *               Set various frequencies and change the duty cycle for
 *               every set frequency value.
 *
 *     Required: libraries: mdis_api
 *     \switches (none)
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z61_simp.c,v $
 * Revision 1.2  2009/03/02 15:52:39  MRoth
 * R: no 64 Bit support
 * M: changed path type to support 64 Bit
 *
 * Revision 1.1  2006/01/30 12:19:11  rla
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2006 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

static const char RCSid[]="$Id: z61_simp.c,v 1.2 2009/03/02 15:52:39 MRoth Exp $";

#include <stdio.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/mdis_api.h>
#include <MEN/z61_drv.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#define Z61_FREQ_START		1000
#define Z61_FREQ_END		1100

#define FPGA_CLK_HZ			33000000

/*--------------------------------------+
|   TYPDEFS                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   EXTERNALS                           |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);
static void CalcPrintPeriodDuty( u_int16 period, u_int16 duty );

/********************************* main ************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return	          success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	INT32_OR_64 path = 0;
	char	*device;
	u_int16 duty = 0;
	u_int16 period = 0;
	int32 temp = 0;
	int error = 0;
	u_int32 retry = 0;

	if (argc < 2 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: z61_simp <device> <chan>\n");
		printf("Function: Z61 example for PWM generator\n");
		printf("          PWM generator will output a signal from 33..30kHz\n");
		printf("          each frequency with a duty cycle from 0..100%%\n");
		printf("Options:\n");
		printf("    device       device name\n");
		printf("\n");
		return(1);
	}

	device = argv[1];

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = M_open(device)) < 0) {
		PrintError("open");
		return(1);
	}

	/*--------------------+
    |  Test Loop          |
    +--------------------*/
	for( period = Z61_FREQ_START; period < Z61_FREQ_END; period ++ ){
		/* set frequency */
		if( (M_setstat( path, Z061_PERIOD, period ) ) ){
			PrintError("setstat Z061_PERIOD");
			error = 1;
			goto abort;
		}

		for( duty = 1 ; duty < (period - 1); duty += (period / 20) ){

			/* calculate and print duty cycle */
			CalcPrintPeriodDuty( period, duty );

			/* set duty cycle */
			if( (M_setstat( path, Z061_PULSE, duty ) ) ){
				PrintError("setstat Z061_PULSE");
				error = 1;
				goto abort;
			}

			/* delay a little bit */
			UOS_Delay( 10 );

			temp = 0;
			retry = 0;
			/* read and compare duty */
			do{
				if( (M_getstat( path, Z061_PULSE, (int32*)&temp ) ) ){
					PrintError("getstat Z061_PULSE");
					error = 1;
					goto abort;
				}

				/* delay a little bit */
				UOS_Delay( 10 );
			}while( (temp != duty) && (retry++ < 10) );

			if( retry >= 10 ){
				PrintError("Duty mismatch");
				error = 1;
				goto abort;
			}
		}

		temp = 0;

		/* read and compare period */
		if( (M_getstat( path, Z061_PERIOD, (int32*)&temp ) ) ){
			PrintError("getstat Z061_PERIOD");
			error = 1;
			goto abort;
		}

		if( temp != period ){
			PrintError("Period mismatch");
			error = 1;
			goto abort;
		}
	}

	/*--------------------+
    |  cleanup            |
    +--------------------*/
	abort:
	if (M_close(path) < 0)
		PrintError("close");

	printf("\n");

	return(0);
}


/**************************** CalcPrintPeriodDuty **************************/
/** Routine to calculate and print Period/Duty
 *
 *  \param period     \IN  period
 *  \param duty       \IN  duty cycle
*/
static void CalcPrintPeriodDuty( u_int16 period, u_int16 duty )
{
	u_int32 freq = 0;
	u_int32 dcycle = 0;

	freq = (((FPGA_CLK_HZ * 10) / (u_int32)(period) )+ 5)/10;
	dcycle = ((((u_int32)(duty) * 1000)/(u_int32)(period)) + 5)/10;

	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	printf("%8dHz %3d%%", (int)freq, (int)dcycle );
}

/********************************* PrintError ******************************/
/** Print MDIS error message
 *
 *  \param info       \IN  info string
*/
static void PrintError(char *info)
{
	printf("*** can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
}
