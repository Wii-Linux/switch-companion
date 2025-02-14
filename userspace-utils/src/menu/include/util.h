#ifndef UTIL_H
#define UTIL_H

extern void UTIL_GetCPUName(char *cpuName);
#ifdef CLIENT_APP
extern int UTIL_GetModelFromCPUInfo(char *model);
#endif

#endif
