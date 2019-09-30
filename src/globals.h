/*
 * Token Bucket Emulation in C (Multi-threading)
 * Author: Praneeth Yerrapragada
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "my402list.h"

int    bucketCount             = 10;   /* Bucket count */
int    numPackets              = 20;   /* Total number of packets */
double arrivalRate             = 1.5;  /* Arrival rate (tokens per second)*/
int    numTokens               = 3;    /* Total number of tokens for a packet */
double lambda                  = 0.5;  /* packets per second */
double mu                      = 0.35; /* packets per second */

int    interrupt               = 0;

int    availableTokens         = 0;
int    tokenDropCount          = 0;
int    tokenCount              = 0;

int    currPacketCount         = 0;
int    servicedPacketCount     = 0;
int    dropPacketCount         = 0;
int    q1PacketCount           = 0;
int    q2PacketCount           = 0;

double timeInQ1                = 0;
double timeInQ2                = 0;
double interTokenArrivalTimeMs = 0;
double totalInterArrivalTimeUs = 0;
double totalServiceTimeSec     = 0;
double totalSystemTimeSec      = 0;
double totalSystemTimeSecSq    = 0;  /* squared */
double initSimTime             = 0;
double endSimTime              = 0;

pthread_t       packetarr;
pthread_t       tokenarr;
pthread_t       server;
pthread_t       inthandler;
pthread_mutex_t tmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond   = PTHREAD_COND_INITIALIZER;

sigset_t        new;

My402List       q1;
My402List       q2;

#endif /* GLOBALS_H_ */
