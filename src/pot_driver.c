#include "pot_driver.h"

static const a2dfile_t POT_INFILE = 0;
/*****************************************/
/********** interface functions **********/

enum a2d_ErrCode pot_readRaw(a2draw_t* output)
{
	const a2dfile_t potInFile = 0;
	return a2d_readRaw(output, potInFile);
}

struct a2dping_Request pot_newPingRequest(const second_t timeS
											   ,const nanosecond_t timeNS
											   ,void (*callback)(const a2draw_t output)
											   ,void (*handleErr)(void))
{
	return a2dping_newRequest(timeS, timeNS, POT_INFILE, callback, handleErr);
}