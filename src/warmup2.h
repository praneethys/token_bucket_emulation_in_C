/*
 * Token Bucket Emulation in C (Multi-threading)
 * Author: Praneeth Yerrapragada
 */

#ifndef WARMUP2_H_
#define WARMUP2_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "my402list.h"
#include "globals.h"

typedef struct Packet
{
    int    interArrivalTimeMs;
    int    numTokens;
    int    serviceTimeMs;
    int    pktSeqNum;
    double timeofdayPacketThread;
    double timeofdayServerThread;
    double timeQ1Entry;
    double timeQ1Exit;
    double timeQ2Entry;
    double timeQ2Exit;
} packet;

void  *InterruptHandler();
void  *ServicePacketArrival(void*);
void  *ServiceTokenBucket();
void  *ServiceTransmissionServer();
void   WorkFunction(void*);
void   PrintStatistics();
double GetTimeUs();
double GetSimTimeMs();

#endif /* WARMUP2_H_ */

