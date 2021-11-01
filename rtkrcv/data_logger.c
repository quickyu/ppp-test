#include <stdio.h>
#include <stdlib.h>

#include "data_logger.h"

#define SSR_LOG_FILE       "ssr_%Y%m%d%h%M.log"
#define OBS_LOG_FILE       "obs_%Y%m%d%h%M.log"
#define SATPOS_LOG_FILE    "satpos_%Y%m%d%h%M.log"

struct ssr_sig {
    uint8_t signal;
    char *str;
};

static struct ssr_sig gps_signal[] = {
    {CODE_L1C, "1C"}, {CODE_L1P, "1P"}, {CODE_L1W, "1W"}, {CODE_L1S, "1S"},
    {CODE_L1L, "1L"}, {CODE_L2C, "2C"}, {CODE_L2D, "2D"}, {CODE_L2S, "2S"},
    {CODE_L2L, "2L"}, {CODE_L2X, "2X"}, {CODE_L2P, "2P"}, {CODE_L2W, "2W"},
    {CODE_L5I, "5I"}, {CODE_L5Q, "5Q"}
};

static struct ssr_sig bds_signal[] = {
    {CODE_L2I, "2I"}, {CODE_L2Q, "2Q"}, {CODE_L6I, "6I"}, {CODE_L6Q, "6Q"}, 
    {CODE_L7I, "7I"}, {CODE_L7Q, "7Q"}, {CODE_L1D, "1D"}, {CODE_L1P, "1P"}, 
    {CODE_L1X, "1X"}, {CODE_L5D, "5D"}, {CODE_L5P, "5P"}, {CODE_L5X, "5X"}, 
    {CODE_L1A, "1A"}, {CODE_L6A, "6A"}
};

static FILE *ssr_log = NULL, *obs_log = NULL, *satpos_log = NULL;

void init_logger()
{
   gtime_t time = utc2gpst(timeget());
   char path[1024];

   reppath(SSR_LOG_FILE, path, time, "", "");
   ssr_log = fopen(path, "w");
   reppath(OBS_LOG_FILE, path, time, "", "");
   obs_log = fopen(path, "w");
   reppath(SATPOS_LOG_FILE, path, time, "", "");
   satpos_log = fopen(path, "w");
}

void close_logger()
{
   if (ssr_log)
      fclose(ssr_log);
   if (obs_log)
      fclose(obs_log);
   if (satpos_log)
      fclose(satpos_log);   
}

void log_ssr(gtime_t *time, nav_t *nav)
{
   char tstr[64], t1str[64], satid[32];

   if (ssr_log == NULL)
      return;
   
   time2str(*time, tstr, 0);
   fprintf(ssr_log, "obs time: %s\n", tstr);

   ssr_t *ssr = nav->ssr;

   for (int i = 0; i < MAXSAT; i++) {
      if (!ssr[i].t0[0].time) 
         continue;

      satno2id(i+1,satid);
      time2str(ssr[i].t0[0], tstr, 0);
      time2str(ssr[i].t0[1], t1str, 0);
      
      fprintf(ssr_log, "%3s iode=%3d ura=%2d t0=%s radial=%6.3f along=%6.3f cross=%6.3f t1=%s dclk=%9.5f",
               satid, ssr[i].iode, ssr[i].ura, tstr, ssr[i].deph[0], ssr[i].deph[1], ssr[i].deph[2],
               t1str, ssr[i].dclk[0]);

      int sig_num = 0;
      struct ssr_sig *sig_array = NULL;

      if (satid[0] == 'G') {
         sig_num = sizeof(gps_signal)/sizeof(gps_signal[0]);
         sig_array = gps_signal;
      } else if (satid[0] == 'C') {
         sig_num =  sizeof(bds_signal)/sizeof(bds_signal[0]);  
         sig_array = bds_signal;
      }    

      fprintf(ssr_log, " cbias= ");

      for (int idx = 0; idx < sig_num; idx++) {
         uint8_t sig = sig_array[idx].signal;
         if (ssr[i].cbias[sig-1] != 0.0f) {
            fprintf(ssr_log, "%s:%6.3f  ", sig_array[idx].str, ssr[i].cbias[sig-1]);
         }    
      }

      fprintf(ssr_log, "\n");   
   }

   fprintf(ssr_log, "\n");
   fflush(ssr_log);  
}

void log_obs(obsd_t *obs, int n)
{
   char str[64], id[16];
   
   for (int i = 0; i < n; i++) {
      time2str(obs[i].time, str, 3);
      satno2id(obs[i].sat, id);
      char *c1 = code2obs(obs[i].code[0]), *c2 = code2obs(obs[i].code[1]);

      fprintf(obs_log, " (%2d) %s %-3s L0=%13.3f L1=%13.3f P0=%13.3f P1=%13.3f "
            "LLI0=0x%02x LLI1=0x%02x SNR0=%3.1f SNR1=%3.1f %s %s\n",
            i+1, str, id, obs[i].L[0], obs[i].L[1], obs[i].P[0],
            obs[i].P[1], obs[i].LLI[0], obs[i].LLI[1],
            obs[i].SNR[0]*SNR_UNIT, obs[i].SNR[1]*SNR_UNIT,
            c1, c2);
   }

   fprintf(obs_log, "\n");
   fflush(obs_log);
}

void log_satpos(obsd_t *obs, int nobs, gtime_t *time, double *rs, double *dts, double *var, int *svh)
{
   char satid[6], obst_str[64], tt_str[64];

   for (int i = 0; i < nobs; i++) {
      satno2id(obs[i].sat, satid);
      time2str(obs[i].time, obst_str, 6);
      time2str(time[i], tt_str, 6);

      fprintf(satpos_log, "%s obs_time=%s t_time=%s rs=%13.3f %13.3f %13.3f dts=%15.12f var=%10.7f svh=%02X\n",
               satid, obst_str, tt_str, rs[i*6], rs[1+i*6], rs[2+i*6],
               dts[i*2], var[i], (uint8_t)svh[i]);
   }

   fprintf(satpos_log, "\n");
   fflush(satpos_log);
}
