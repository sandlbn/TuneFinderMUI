// amigaamp.h
#ifndef AMIGAAMP_H
#define AMIGAAMP_H

#include <exec/types.h>

BOOL IsAmigaAMPRunning(void);
BOOL OpenStreamInAmigaAMP(const char *streamURL);
BOOL OpenStreamInAmigaAMPWithName(const char *streamURL, const char *stationName);
BOOL StopAmigaAMP(void);
BOOL WaitAndIconifyAmigaAMP(void);
BOOL SendCommandToAmigaAMP(const char *command);
#endif