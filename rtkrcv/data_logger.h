#include "rtklib.h"

void init_logger();
void close_logger();
void log_ssr(gtime_t *time, nav_t *nav);
void log_obs(obsd_t *obs, int n);
void log_satpos(obsd_t *obs, int nobs, gtime_t *time, double *rs, double *dts, double *var, int *svh);
