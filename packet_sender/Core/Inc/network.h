#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <stdint.h>

typedef struct {
    uint32_t current_us;
    uint32_t min_us;
    uint32_t max_us;
    uint32_t mean_us;
    uint32_t stddev_us;
    uint32_t packet_count;
    uint32_t lost_count;
    uint32_t samples[32];
    uint8_t  sample_index;
} RTT_Stats;

extern RTT_Stats rtt_stats;

void Network_Init(void);
void Network_RunRTT(void);

#endif