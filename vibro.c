#include "common.h"
#include "maple.h"
#include "vibro.h"
#include "report.h"

static unsigned int read_belong(unsigned int *l)
{
	unsigned char *b = (unsigned char *)l;
	return (b[0]<<24)|(b[1]<<16)|(b[2]<<8)|b[3];
}

static void write_belong(unsigned int *l, unsigned int v)
{
	unsigned char *b = (unsigned char *)l;
	b[0] = v>>24;
	b[1] = v>>16;
	b[2] = v>>8;
	b[3] = v;
}

/*! Returns 1 if a @[unit] can function as memcard.
 *!
 */
int vibro_check_unit(int unit, struct vibroinfo *info)
{
	unsigned int func;
	unsigned char *res;
	
	info->port = unit/6;
	info->dev = unit - info->port*6;
	
	if(info->port<0 || info->port>3 || info->dev<0 || info->dev>5)
		return 0;

	//FIXME: Timer problem if docmd was done somewhere else within 15000µs?
	res = maple_docmd(info->port, info->dev, MAPLE_COMMAND_DEVINFO, 0, NULL);

#ifndef NOSERIAL
	if(!res)
		report("P1 ");
	else if(res[0] != MAPLE_RESPONSE_DEVINFO) {
		report("P2 ");
		report(itoa_ronin(res[0]));
		report(" ");
	}
	else if(!(res[3]>=28))
		report("P3 ");
	else if(!((func=read_belong((unsigned int *)(res+4)))&MAPLE_FUNC_PURUPURU))
		report("P4 ");
#endif
	
	if (res && res[0] == MAPLE_RESPONSE_DEVINFO && res[3] >= 28 &&
		((func=read_belong((unsigned int *)(res+4))) == MAPLE_FUNC_PURUPURU)) 
	{
		info->func = func;
		
		return 1;
	}
	
	return 0;
}

int vibro_const_vibration(struct vibroinfo *info, int on)
{
	unsigned int param[2];
	unsigned char *res;
	int retr;
	
	if(!(info->func & MAPLE_FUNC_PURUPURU))
		return 0;
	
	for(retr = 0; retr < 5; retr++) 
	{
		write_belong(&param[0], MAPLE_FUNC_PURUPURU);
		write_belong(&param[1], (on? 0x11702700 : 0x10002700));
		
		//FIXME: Timer problem if docmd was done somewhere else within 15000µs?
		//(In reality we don't care. It's just a beep...)
		if((res = maple_docmd(info->port, info->dev, MAPLE_COMMAND_SETCOND, 2, param)) && res[0] == MAPLE_RESPONSE_OK)
			return 1;
	}
	
	return 0;
}

