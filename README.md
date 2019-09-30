# Token Bucket Emulation in C (Multi-threading)

This code emulates a traffic shaper who transmits packets controlled by a token bucket filter depicted below using multi-threading within a single process. 

The token bucket has a capacity (bucket depth) of B tokens. Tokens arrive into the token bucket at a constant rate of r tokens per second. Extra tokens (overflow) would simply disappear if the token bucket is full. A token bucket, together with its control mechanism, is referred as a token bucket filter.

Packets arrive at the token bucket filter at a rate of lambda packets per second (i.e., packets have an inter-arrival time of 1/lambda) and each packet requires P tokens in order for it to be eligiable for transmission. (Packets that are eligiable for transmission are queued at the Q2 facility.) 

When a packet arrives, if Q1 is not empty, it will just get queued onto the Q1 facility. Otherwise, it will check if the token bucket has P or more tokens in it. If the token bucket has P or more tokens in it, P tokens will be removed from the token bucket and the packet will join the Q2 facility (technically speaking, you are required to first add the packet to Q1 and timestamp the packet, remove the P tokents from the token bucket and the packet from Q1 and timestamp the packet, before moving the packet into Q2). If the token bucket does not have enough tokens, the packet gets queued into the Q1 facility. You should also check if there is enough tokens in the bucket so you can move the packet at the head of Q1 into Q2.

The transmission facility S serves packets in Q2 at a service rate of mu per second. When a packet has received 1/mu seconds of service, it leaves our system.

When a token arrives at the token bucket, it will add a token into the token bucket. If the bucket is already full, the token will be lost. It will then check to see if Q1 is empty. If Q1 is not empty, it will see if there is enough tokens to make the packet at the head of Q1 be eligiable for transmissions. If it does, it will remove the corresponding number of tokens from the token bucket, remove that packet from Q1 and move it into Q2, and wake up the server (by broadcasting the corresponding condition). Technically speaking, the "server" is not part of the token bucket filter. Nevertheless, it's part of this assignment to emulation the sever as well.

Our system can run in only one of two modes.

*  Deterministic	 : 	In this mode, all inter-arrival times are equal to 1/lambda seconds, all packets require exactly P tokens, and all service times are equal to 1/mu seconds. If 1/lambda is greater than 10 seconds, please use an inter-arrival time of 10 seconds. If 1/mu is greater than 10 seconds, please use an service time of 10 seconds.
 
*  Trace-driven	 : 	In this mode, we will drive the emulation using a tracefile. Each line in the trace file specifies the inter-arrival time of a packet, the number of tokens it need in order for it to be eligiable for transmission, and its service time.
Your job is to emulate the packet and token arrivals, the operation of the token bucket filter, the first-come-first-served queues Q1 and Q2, and server S. You also must produce a trace of your emulation for every important event occurred in your emulation.

You must use:
* one thread for packet arrival
* one thread for token arrival
* one thread for server
* You must not use one thread for each packet.
* In addition, you must use at least one mutex to protect Q1, Q2, and the token bucket.
* Finally, Q1 and Q2 must have infinite capacity.

# Command line arguments
The lambda, mu, r, B, and P parameters all have obvious meanings. The -n option specifies the total number of packets to arrive. If the -t option is specified, tsfile is a trace specification file that you should use to drive your emulation. In this case, you should ignore the -lambda, -mu, -P, and -num commandline options and run your emulation in the trace-driven mode. You may assume that tsfile conforms to the tracefile format specification. (This means that if you detect an error in this file, you may simply print an error message and call exit(). There is no need to perform error recovery.) If the -t option is not used, you should run your emulation in the deterministic mode.

The default value (i.e., if it's not specified in a commandline option) for lambda is 0.5 (packets per second), the default value for mu is 0.35 (packets per second), the default value for r is 1.5 (tokens per second), the default value for B is 10 (tokens), the default value for P is 3 (tokens), and the default value for num is 20 (packets). B, P, and num must be positive integers with a maximum value of 2147483647 (0x7fffffff). lambda, mu, and r must be positive real numbers.

If 1/r is greater than 10 seconds, please use an inter-token-arrival time of 10 seconds.

# Working
The emulation should go as follows. At emulation time 0, all 3 threads (arrival, token depositing, and server threads) got started. The arrival thread would sleep so that it can wake up at a time such that the inter-arrival time of the first packet would match the specification (either according to lambda or the first record in a tracefile). At the same time, the token depositing thread would sleep so that it can wake up every 1/r seconds and would try to deposit one token into the token bucket. The first packet p1 arrives at time t1 (the 2nd packet p2 arrives at time t2, and so on).
As a packet arrives, you need to follow the operational rules of the token bucket filter and determine if the packet should be queued onto Q1 (and no tokens should be removed) or Q2 (and the correct number of tokens removed). There is one exception to the rules though. If the number of tokens required by a packet is larget than the bucket depth, the packet must be dropped (otherwise, it will block all other packets that follow it). If the packet is to be queued onto Q2, when the mutex is locked, the arrival thread should broadcast the condition to wake up the potentially sleeping server thread.

As a token arrives, you also need to follow the operational rules of the token bucket filter and determine if the token should be added to the token bucket filter or not. You should also check for the condition where Q1 is not empty and there are enough tokens in the token bucket. If this is the case, you should remove the correct number of tokens from the token bucket and move the packet at the head of Q1 into Q2. When this thread has the mutex locked, it should broadcast the condition to wake up the potentially sleeping server thread.

As a server thread wakes up, it should lock the mutex and check if Q2 is empty. If it's not empty, it should remove the first packet in Q2 and sleep for the service time of that packet. If Q2 is empty, it should get blocked waiting for the condition variable (and release the mutex simultaneously).

You are required to produce a detailed trace as the packets move through the system.

# Trace File Format:
The trace specification file is an ASCII file containing n+1 lines (each line is terminated with a "\n") where n is the total number of packets to arrive. Line 1 of the file contains an integer which corresponds to the value of n. Line k of the file contains the inter-arrival time in milliseconds (an integer), the number of tokens required (an integer), and service time in milliseconds (an integer) for packet k-1. The 3 fields are separated by space or tab characters. There must be no leading or trailing space or tab characters in a line. A sample tsfile for n=3 packets is provided. It's content is listed below:

    3
    2716   2    9253
    7721   1   15149
    972    3    2614
In the above example, packet 1 is to arrive 2716ms after emulation starts, it needs 2 tokens to be eligible for transmission, and its service time should be 9253ms; the inter-arrival time between packet 2 and 1 is to be 7721ms, it needs 1 token to be eligible for transmission, and its service time should be 15149ms; the inter-arrival time between packet 3 and 2 is to be 972ms, it needs 3 token to be eligible for transmission, and its service time should be 2614ms.
You may assume that this file is error-free. (This means that if you detect a real error in this file, you must simply print an error message and call exit(). There is no need to perform error recovery.)

# Statistics:
The average number of packets at a facility can be obtained by adding up all the time spent at that facility (for all packets) divided by the total emulation time. The time spent in system for a packet is the difference between the time the packet departed from the server and the time that packet arrived. The token drop probability is the total number of tokens dropped because the token bucket was full divided by the total number of tokens that was produced by the token depositing thread. The packet drop probability is the total number of packets dropped because the number of tokens required is larger than the bucket depth divided by the total number of packets that was produced by the arrival thread.

If token n is dropped because of token bucket overflow, you must print:
    ????????.???ms: token tn arrives, dropped
where the question marks was the time token n arrived (and dropped). Similarlily, if packet n is dropped because the number of token it needs is larger than the bucket depth, you must print:
    ????????.???ms: packet pn arrives, needs ? tokens, dropped
where the question marks was the time packet n arrived (and dropped).

Please note that each departure line in the above trace have been broken up into 2 lines. This is done because of the way web browsers handle pre-formatted text. Please print them all in one line in your program.

All real values in the statistics must be printed with at least 6 significant digits. (If you are using printf(), you can use "%.6g".) A timestamp in the beginning of a line of trace output must be in milliseconds with 8 digits (zero-padded) before the decimal point and 3 digits (zero-padded) after the decimal point.

Please use sample means when you calculated the averages. If n is the number of sample, this mean that you should divide things by n (and not n-1).

The unit for time related statistics must be in seconds (and not milliseconds).

Let X be something you measure. The standard deviation of X is the square root of the variance of X. The variance of X is the average of the square of X minus the square of the average of X. Let E(X) denote the average of X, you can write:

Var(X) = E(X2) - [E(X)]2

If the user presses <Cntrl+C> on the keyboard, you must stop the arrival thread and the token depositing thread, remove all packets in Q1 and Q2, let your server finish serving the current packet in the usual way, and output statistics in the usual way. (Please note that it may not be possible to remove all packets in Q1 at the instance of the interrupt. The idea here is that once the interrupt has occurred, the only packets you should server are the only one in service. All other packets should be removed from the system.)

Finally, when no more packet can arrive into the system, you must stop the arrival thread as soon as possible. Also, when Q1 is empty and no future packet can arrival into Q1, you must stop the token depositing thread as soon as possible.