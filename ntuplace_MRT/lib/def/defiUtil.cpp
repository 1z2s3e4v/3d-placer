/*
 *     This  file  is  part  of  the  Cadence  LEF/DEF  Open   Source
 *  Distribution,  Product Version 5.5, and is subject to the Cadence
 *  LEF/DEF Open Source License Agreement.   Your  continued  use  of
 *  this file constitutes your acceptance of the terms of the LEF/DEF
 *  Open Source License and an agreement to abide by its  terms.   If
 *  you  don't  agree  with  this, you must remove this and any other
 *  files which are part of the distribution and  destroy any  copies
 *  made.
 *
 *     For updates, support, or to become part of the LEF/DEF Community,
 *  check www.openeda.org for details.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#   include <time.h>
#else /* not WIN32 */
#   include <sys/time.h>
#endif
#if ibmrs
#include <time.h>
#endif


/* Not needed
time_t defiCalcTime() {
    * Calculate the number for the given date
    * The date is 5/1/99

    * Used to calculate the UTC for a time bomb date in libcai.a
    * see caiInitWork() function
    *
    struct tm	ts;

    ts.tm_sec = 0;
    ts.tm_min = 0;
    ts.tm_hour = 0;
    ts.tm_mday = 1;
    ts.tm_mon = 5;
    ts.tm_year = 1999 - 1900;
    ts.tm_wday = 0;
    ts.tm_yday = 0;
    ts.tm_isdst = 0;

*
    printf("May 1, 1999 in UTC is %d\n", mktime(&ts));
    ts.tm_mday = 2;
    printf("May 2, 1999 in UTC is %d\n", mktime(&ts));

    printf("Right now is %d\n", time(0));
*
    return(mktime(&ts));
}
*/

////////////////////////////////////////////////////
//
//    defiTimeBomb
//
////////////////////////////////////////////////////

// Check the current date against the date given
/*
int defiValidTime() {
    time_t	bombTime = defiCalcTime();
    time_t	curTime;

    curTime = time((time_t *)NULL);
    if (curTime == -1 || curTime > bombTime)
    {
	(void)printf("The demonstration version of this code is no longer\n"
		     "available.  Please contact your Lef/Def Parser\n"
		     "software provider for up to date code.\n");
	return(0);
    }
    return (1);
}
*/

////////////////////////////////////////////////////
//
//    Convert the orient from integer to string
//
////////////////////////////////////////////////////
char* defiOrientStr(int orient) {
  switch (orient) {
      case 0: return ((char*)"N");
      case 1: return ((char*)"W");
      case 2: return ((char*)"S");
      case 3: return ((char*)"E");
      case 4: return ((char*)"FN");
      case 5: return ((char*)"FW");
      case 6: return ((char*)"FS");
      case 7: return ((char*)"FE");
  };
  return ((char*)"");
}
