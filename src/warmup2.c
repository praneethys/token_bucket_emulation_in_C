/*
 * Token Bucket Emulation in C (Multi-threading)
 * Author: Praneeth Yerrapragada
 */

#include "warmup2.h"

// Current time in microseconds
double GetTimeUs()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (double) tv.tv_sec * 1000000 + tv.tv_usec;
}

// Current simulation time in milli seconds
double GetSimTimeMs()
{
  return (GetTimeUs() - initSimTime) / 1000;
}

// Work function for packet arrival thread
void *ServicePacketArrival(void* fp_)
{
  char* linePtr = NULL;
  size_t len = 0;
  ssize_t line;
  packet *pkt = NULL;

  FILE* fp = (FILE*) fp_;

  int packetCount = 0;
  double currArrivalTime;
  double prevArrivalTime = initSimTime;

  if (fp == NULL) {
    int interArrivalTimeMs;
    int serviceTimeMs;

    if ((int) (1 / lambda) > 10)
      interArrivalTimeMs = 10000;
    else
      interArrivalTimeMs = (int) ((1 / lambda) * 1000);

    if ((int) (1 / mu) > 10)
      serviceTimeMs = 10000;
    else
      serviceTimeMs = (int) ((1 / mu) * 1000);

    while (packetCount < numPackets) {
      pkt = (packet*) malloc(sizeof(packet));
      if (pkt == NULL) {
        fprintf(stderr, "Memory not allocated for packet structure.\n");
        exit(EXIT_FAILURE);
      }

      pkt->interArrivalTimeMs = interArrivalTimeMs;
      pkt->numTokens = numTokens;
      pkt->serviceTimeMs = serviceTimeMs;

      unsigned int packetSleepTimeSec = (int) pkt->interArrivalTimeMs * 1000;
      usleep(packetSleepTimeSec);

      pthread_mutex_lock(&tmutex);
      if (interrupt == 1) {
        pthread_cancel(tokenarr);
        My402ListUnlinkAll(&q1);
        My402ListUnlinkAll(&q2);
        pthread_mutex_unlock(&tmutex);
        pthread_exit(0);
      }

      packetCount++;
      currArrivalTime = GetTimeUs();
      totalInterArrivalTimeUs += (currArrivalTime - prevArrivalTime);
      prevArrivalTime = currArrivalTime;

      if (pkt->numTokens > bucketCount) {
        // Packet requires more than total bucketCount tokens which is not possible.
        // Hence dropping packet.
        dropPacketCount++;
        printf("%012.3fms: %10s p%d arrives, needs %d tokens, dropped\n",
            GetSimTimeMs(), __func__, packetCount, pkt->numTokens);

      } else {
        pkt->timeQ1Entry = currArrivalTime;
        pkt->pktSeqNum = packetCount;
        My402ListAppend(&q1, pkt);
        q1PacketCount++;
        printf("%012.3fms: %10s p%d enters Q1\n", GetSimTimeMs(), __func__,
            packetCount);
      }

      if (dropPacketCount == numPackets) {
        pthread_cancel(tokenarr);
        pthread_cancel(inthandler);
        pthread_mutex_unlock(&tmutex);
        return NULL;
      }

      pthread_mutex_unlock(&tmutex);
    }

  } else {
    /* The trace specification file is an ASCII file containing n+1 lines
     * (each line is terminated with a "\n") where n is the total number of
     * packets to arrive. Line 1 of the file contains an integer which corresponds
     * to the value of n. Line k of the file contains the inter-arrival time in
     * milliseconds (an integer), the number of tokens required (an integer),
     * and service time in milliseconds (an integer) for packet k-1. The 3 fields
     * are separated by space or tab characters. There must be no leading or
     * trailing space or tab characters in a line.
     */

    /* You may assume that this file is error-free. (This means that if you detect
     * a real error in this file, you must simply print an error message and call
     * exit(). There is no need to perform error recovery.)
     *
     * TODO: Perform error checking on the file format
     */

    if ((line = getline(&linePtr, &len, fp)) != -1) {
      numPackets = atoi(strtok(linePtr, "\n"));
    }

    while ((line = getline(&linePtr, &len, fp)) != -1) {

      pkt = (packet*) malloc(sizeof(packet));
      if (pkt == NULL) {
        fprintf(stderr, "Memory not allocated for packet structure.\n");
        exit(EXIT_FAILURE);
      }

      pkt->interArrivalTimeMs = atoi(strtok(linePtr, " "));
      pkt->numTokens = atoi(strtok(NULL, " "));
      pkt->serviceTimeMs = atoi(strtok(NULL, " "));

      unsigned int packetSleepTimeSec = (int) pkt->interArrivalTimeMs * 1000;
      usleep(packetSleepTimeSec);

      pthread_mutex_lock(&tmutex);
      if (interrupt == 1) {
        pthread_cancel(tokenarr);
        My402ListUnlinkAll(&q1);
        My402ListUnlinkAll(&q2);
        pthread_mutex_unlock(&tmutex);
        pthread_exit(0);
      }

      packetCount++;
      currArrivalTime = GetTimeUs();
      totalInterArrivalTimeUs += (currArrivalTime - prevArrivalTime);
      prevArrivalTime = currArrivalTime;

      if (pkt->numTokens > bucketCount) {
        // Packet requires more than total bucketCount tokens which is not possible.
        // Hence dropping packet.
        dropPacketCount++;
        printf("%012.3fms: %10s p%d arrives, needs %d tokens, dropped\n",
            GetSimTimeMs(), __func__, packetCount, pkt->numTokens);

      } else {
        pkt->timeQ1Entry = currArrivalTime;
        pkt->pktSeqNum = packetCount;
        My402ListAppend(&q1, pkt);
        q1PacketCount++;
        printf("%012.3fms: %10s p%d enters Q1\n", GetSimTimeMs(), __func__,
            packetCount);
      }

      if (dropPacketCount == numPackets) {
        pthread_cancel(tokenarr);
        pthread_cancel(inthandler);
        pthread_mutex_unlock(&tmutex);
        return NULL;
      }

      pthread_mutex_unlock(&tmutex);
    }
  }

  return NULL;
}

// Work function for Token bucket thread
void *ServiceTokenBucket()
{
  My402ListElem *q1Front = NULL;
  packet* pkt = NULL;

  if (1 / arrivalRate <= 10)
    interTokenArrivalTimeMs = (1 / arrivalRate) * 1000;
  else
    interTokenArrivalTimeMs = 10000;
  unsigned int tokenSleep = (int) (interTokenArrivalTimeMs * (double) 1000);
  availableTokens = numTokens;

  // This thread keeps running until it is cancelled by server thread
  // or packet thread
  while (TRUE) {
    pthread_mutex_lock(&tmutex);
    if (interrupt == 1) {
      pthread_cancel(packetarr);
      My402ListUnlinkAll(&q1);
      My402ListUnlinkAll(&q2);
      pthread_mutex_unlock(&tmutex);
      pthread_exit(0);
    }

    // Move packets from Q1 to Q2
    while (q1.num_members > 0) {
      q1Front = My402ListFirst(&q1);
      pkt = q1Front->obj;
      if (pkt->numTokens <= availableTokens) {
        double currTime = GetTimeUs();
        pkt->timeQ1Exit = currTime;
        pkt->timeQ2Entry = currTime;
        pkt->timeofdayServerThread = currTime;

        if (q2.num_members == 0) {
          pthread_cond_signal(&cond);
        }

        My402ListAppend(&q2, pkt);
        q2PacketCount++;

        My402ListUnlink(&q1, q1Front);
        availableTokens -= pkt->numTokens;
        timeInQ1 += pkt->timeQ1Exit - pkt->timeQ1Entry;

        printf("%012.3fms: %10s p%d leaves Q1, time in Q1 = %.3fms, ",
            GetSimTimeMs(), __func__, pkt->pktSeqNum,
            (pkt->timeQ1Exit - pkt->timeQ1Entry) / 1000);
        if (availableTokens < 2)
          printf("token bucket now has %d token \n", availableTokens);
        else
          printf("token bucket now has %d tokens\n", availableTokens);

        printf("%012.3fms: %10s p%d enters Q2\n", GetSimTimeMs(), __func__,
            pkt->pktSeqNum);

      } else {
        break;
      }
    }
    pthread_mutex_unlock(&tmutex);

    usleep(tokenSleep);

    // Accept new tokens
    pthread_mutex_lock(&tmutex);
    if (interrupt == 1) {
      pthread_cancel(packetarr);
      My402ListUnlinkAll(&q1);
      My402ListUnlinkAll(&q2);
      pthread_mutex_unlock(&tmutex);
      pthread_exit(0);
    }

    tokenCount++;

    if (availableTokens >= bucketCount) {
      // Token bucket is full and can't accept any more tokens
      tokenDropCount++;

      printf("%012.3fms: %10s token t%d arrives, dropped\n", GetSimTimeMs(),
          __func__, tokenCount);

    } else {
      availableTokens++;

      if (availableTokens < 2)
        printf(
            "%012.3fms: %10s token t%d arrives, token bucket now has %d token\n",
            GetSimTimeMs(), __func__, tokenCount, availableTokens);
      else
        printf(
            "%012.3fms: %10s token t%d arrives, token bucket now has %d tokens\n",
            GetSimTimeMs(), __func__, tokenCount, availableTokens);
    }
    pthread_mutex_unlock(&tmutex);
  }

  return NULL;
}

// Work function for transmission server thread
void *ServiceTransmissionServer()
{
  My402ListElem *q2Front = NULL;
  packet *serverPkt = NULL;
  unsigned int sleepMs;

  while (servicedPacketCount < numPackets) {
    pthread_mutex_lock(&tmutex);
    if (interrupt == 1) {
      pthread_cancel(tokenarr);
      pthread_cancel(packetarr);
      My402ListUnlinkAll(&q1);
      My402ListUnlinkAll(&q2);
      pthread_mutex_unlock(&tmutex);
      pthread_exit(0);
    }

    while (q2.num_members == 0) {
      pthread_cond_wait(&cond, &tmutex);
    }

    // Pop front of the queue, Q2 for transmission
    q2Front = My402ListFirst(&q2);
    serverPkt = q2Front->obj;
    My402ListUnlink(&q2, q2Front);

    servicedPacketCount++;

    // Stats
    serverPkt->timeQ2Exit = GetTimeUs();
    double pktDuration = serverPkt->timeQ2Exit - serverPkt->timeQ2Entry;
    timeInQ2 += pktDuration;
    printf("%012.3fms: %10s p%d begin service as S, time in Q2 = %.3fms\n",
        GetSimTimeMs(), __func__, serverPkt->pktSeqNum, pktDuration / 1000);

    pthread_mutex_unlock(&tmutex);

    // Transmitting packet...
    sleepMs = (int) (serverPkt->serviceTimeMs * (double) 1000);
    usleep(sleepMs);

    pthread_mutex_lock(&tmutex);
    // Transmission complete. Record stats
    double departTime = GetTimeUs();
    double serviceTime = (departTime - serverPkt->timeQ2Exit) / 1000;
    printf(
        "%012.3fms: %10s p%d departs from service as S, service time = %.3fms, time in the system = %.3fms\n",
        GetSimTimeMs(), __func__, serverPkt->pktSeqNum, serviceTime,
        (departTime - serverPkt->timeofdayPacketThread) / 1000);

    totalServiceTimeSec += (departTime - serverPkt->timeQ2Exit);
    totalSystemTimeSec += (departTime - serverPkt->timeofdayPacketThread);
    totalSystemTimeSecSq += pow((departTime - serverPkt->timeofdayPacketThread),
        (double) 2);

    if (interrupt == 1) {
      pthread_cancel(tokenarr);
      pthread_cancel(packetarr);
      My402ListUnlinkAll(&q1);
      My402ListUnlinkAll(&q2);
      pthread_mutex_unlock(&tmutex);
      pthread_exit(0);
    }

    if (servicedPacketCount == numPackets) {
      assert(q1.num_members == 0);
      assert(q1.num_members == 0);
      pthread_cancel(tokenarr);
      pthread_cancel(packetarr);
      pthread_cancel(inthandler);
    }

    pthread_mutex_unlock(&tmutex);
  }

  return NULL;
}

// Work function for interrupt handling thread
void *InterruptHandler()
{
  int signal;
  sigwait(&new, &signal);
  pthread_sigmask(SIG_UNBLOCK, &new, NULL);
  pthread_mutex_lock(&tmutex);
  interrupt = 1;
  pthread_mutex_unlock(&tmutex);
  return NULL;
}

// Print detailed summary statistics for the emulation
void PrintStatistics()
{
  printf("\nStatistics:\n");
  if (q1PacketCount != 0)
    printf("    average packet inter-arrival time = %.6gs\n",
        fabs(totalInterArrivalTimeUs / (double) (q1PacketCount * 1000000)));
  else
    printf(
        "    average packet inter-arrival time = (not available because no packet was served during the entire emulation)\n");

  if (servicedPacketCount != 0)
    printf("    average packet service time = %.6gs\n\n",
        totalServiceTimeSec / (double) (servicedPacketCount * 1000000));
  else
    printf(
        "    average packet service time = (not available because no packet was served during the entire emulation)\n\n");

  if (q1PacketCount != 0)
    printf("    average number of packets in Q1 = %.6g\n",
        timeInQ1 / endSimTime);
  else
    printf(
        "    average number of packets in Q1 = (not available because no packet was served during the entire emulation)\n");

  if (q2PacketCount != 0)
    printf("    average number of packets in Q2 = %.6g\n",
        timeInQ2 / endSimTime);
  else
    printf(
        "    average number of packets in Q2 = (not available because no packet was served during the entire emulation)\n");

  if (servicedPacketCount != 0) {
    printf("    average number of packets at S = %.6g\n\n",
        totalServiceTimeSec / endSimTime);
    printf("    average time a packet spent in system = %.6gs\n",
        totalSystemTimeSec / (servicedPacketCount * 1000000));
    double variance = ((totalSystemTimeSecSq / servicedPacketCount)
        - pow((totalSystemTimeSec / servicedPacketCount), (double) 2));
    printf("    standard deviation for time spent in system = %.6gs\n\n",
        sqrt(variance) / 1000000);
  } else {
    printf(
        "    average number of packets at S = (not available because no packet was served during the entire emulation)\n");
    printf(
        "    average time a packet spent in system = (not available because no packet was served during the entire emulation)\n");
    printf(
        "    standard deviation for time spent in system = (not available because no packet was served during the entire emulation)\n\n");
  }

  if (tokenCount != 0)
    printf("    token drop probability = %.6g\n",
        (double) tokenDropCount / (double) tokenCount);
  else
    printf(
        "    token drop probability = (not available because no token has arrived into the token bucket)\n");

  if (numPackets != 0)
    printf("    packet drop probability = %.6g\n",
        (double) dropPacketCount / (double) numPackets);
  else
    printf(
        "    packet drop probability = (not available because no packet was served during the entire emulation)\n");
}

// Main work function
void WorkFunction(void* fp)
{
  memset(&q1, 0, sizeof(My402List));
  (void) My402ListInit(&q1);
  memset(&q2, 0, sizeof(My402List));
  (void) My402ListInit(&q2);

  sigemptyset(&new); /* initializes a null signal set, new */
  sigaddset(&new, SIGINT); /* packs the signal, SIGINT into the new set */

  initSimTime = GetTimeUs();
  printf("%012.3fms: emulation begins\n", initSimTime - initSimTime);

  pthread_sigmask(SIG_BLOCK, &new, NULL);

  if (pthread_create(&packetarr, 0, ServicePacketArrival, fp)) {
    fprintf(stderr, "Error in creating packet arrival thread.\n");
    exit(1);
  }

  if (pthread_create(&tokenarr, 0, ServiceTokenBucket, 0)) {
    fprintf(stderr, "Error in creating token bucket thread.\n");
    exit(1);
  }

  if (pthread_create(&server, 0, ServiceTransmissionServer, 0)) {
    fprintf(stderr, "Error in creating server thread.\n");
    exit(1);
  }

  if (pthread_create(&inthandler, 0, InterruptHandler, 0)) {
    fprintf(stderr, "Error in creating interrupt handler thread.\n");
    exit(1);
  }

  pthread_join(packetarr, NULL);
  pthread_join(tokenarr, NULL);
  pthread_join(server, NULL);
  pthread_join(inthandler, NULL);
  pthread_mutex_destroy(&tmutex);
  pthread_cond_destroy(&cond);

  endSimTime = GetTimeUs();
  printf("%012.3fms: emulation ends\n", GetSimTimeMs());
  PrintStatistics();

  pthread_exit(0);
}

int main(int argc, char *argv[])
{
  int opt;
  int readFromFile = FALSE;

  /* TODO: Use GetLongOptions to get -lambda and -mu */

  while ((opt = getopt(argc, argv, "l:m:n:r:B:P:t:")) != -1) {
    switch (opt) {
      case 'l':
        if (atoi(optarg) < 0) {
          fprintf(stderr, "Lambda cannot be Negative.\n");
          exit(EXIT_FAILURE);
        } else {
          lambda = atof(optarg);
        }
        break;

      case 'm':
        if (atoi(optarg) < 0) {
          fprintf(stderr, "mu cannot be Negative.\n");
          exit(EXIT_FAILURE);
        } else {
          mu = atof(optarg);
        }
        break;

      case 'n': {
        int n_ = atoi(optarg);
        if (n_ > 2147483647 || n_ < 0) {
          fprintf(stderr,
              "Total number of packets to arrive cannot be Negative or larger than 2147483647.\n");
          exit(EXIT_FAILURE);
        } else {
          numPackets = n_;
        }
        break;
      }

      case 'r':
        if (atoi(optarg) < 0) {
          fprintf(stderr, "Total arrival rate cannot be Negative.\n");
          exit(EXIT_FAILURE);
        } else {
          arrivalRate = atof(optarg);
        }
        break;

      case 't': {
        FILE *fp = fopen(optarg, "r");
        if (fp == NULL) {
          fprintf(stderr, "Cannot open %s for reading.\n", optarg);
          exit(EXIT_FAILURE);
        }

        printf("Emulation Parameters:\n");
        printf("    r = %.1f\n", arrivalRate);
        printf("    B = %d\n", bucketCount);
        printf("    tsfile = %s\n\n", optarg);
        WorkFunction((void*) fp);
        readFromFile = TRUE;
        break;
      }

      case 'B': {
        int B_ = atoi(optarg);
        if (B_ > 2147483647 || B_ < 0) {
          fprintf(stderr,
              "Total bucket count cannot be Negative or larger than 2147483647.\n");
          exit(EXIT_FAILURE);
        } else {
          bucketCount = B_;
        }
        break;
      }

      case 'P': {
        int P_ = atoi(optarg);
        if (P_ > 2147483647 || P_ < 0) {
          fprintf(stderr,
              "Total number of tokens for a packet cannot be Negative or larger than 2147483647.\n");
          exit(EXIT_FAILURE);
        } else {
          numTokens = P_;
        }
        break;
      }

      default:
        fprintf(stderr, "Usage: %s [-lmnrBPt:] [file...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (readFromFile == FALSE) {
    printf("Emulation Parameters:\n");
    printf("    lambda = %.2f\n", lambda);
    printf("    mu = %2.f\n", mu);
    printf("    r = %.1f\n", arrivalRate);
    printf("    B = %d\n", bucketCount);
    printf("    P = %d\n", numTokens);
    printf("    number to arrive = %d\n\n", numPackets);
    WorkFunction(NULL);
  }

  return 0;
}
