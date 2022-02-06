#ifndef _RONIN_VIBRO_H
#define _RONIN_VIBRO_H been_here_before
/*
 * VIBRO pack
 */
#include "common.h"

struct vibroinfo {
  int port;
  int dev;
  int pt;
  unsigned long func;
};

START_EXTERN_C
int vibro_check_unit(int unit, struct vibroinfo *info);
int vibro_const_vibration(struct vibroinfo *info, int on);
END_EXTERN_C

#endif //_RONIN_VIBRO_H

