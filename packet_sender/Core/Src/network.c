#include "network.h"
#include "wizchip_conf.h"
#include "w5500.h"
#include "stm32f4xx_hal.h"
#include "fonts.h"
#include "st7789.h"
#include <math.h>
#include "socket.h"
#include "dhcp.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// External declaration of the timer handle for RTT measurement
extern TIM_HandleTypeDef htim2;

// Global variable to hold RTT statistics
RTT_Stats rtt_stats;

// DHCP callback functions
void cb_ip_assign(void) {}
void cb_ip_update(void) {}
void cb_ip_conflict(void) {}

/*
Initializes the network interface, including the W5500 chip and DHCP client. This should be called once at startup.
*/
void Network_Init(void) {

    // Hardware reset W5500
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(100);

    // Initialize W5500 buffers
    wizchip_init(NULL, NULL);
    memset(&rtt_stats, 0, sizeof(RTT_Stats));

    // Set MAC address first
    uint8_t mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01};
    setSHAR(mac);

    // Apply MAC to netinfo
    wiz_NetInfo pre_netinfo = {
        .mac  = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01},
        .dhcp = NETINFO_DHCP
    };
    wizchip_setnetinfo(&pre_netinfo);

    // Set subnet to 0 so broadcast packets are accepted before DHCP
    uint8_t zero_ip[4] = {0, 0, 0, 0};
    setSUBR(zero_ip);
    setSIPR(zero_ip);

    // Verify W5500 version
    uint8_t version = getVERSIONR();
    char ver_str[32];
    sprintf(ver_str, "W5500 Ver: %d", version);
    ST7789_WriteString(10, 110, ver_str, Font_11x18, WHITE, BLACK);
    HAL_Delay(500);

    // Wait for PHY link
    while(wizphy_getphylink() == PHY_LINK_OFF) {
        ST7789_WriteString(10, 60, "No Link...", Font_11x18, RED, BLACK);
        HAL_Delay(500);
    }
    ST7789_WriteString(10, 60, "Link OK!  ", Font_11x18, GREEN, BLACK);
    HAL_Delay(500);

    // Register DHCP callbacks
    reg_dhcp_cbfunc(cb_ip_assign, cb_ip_update, cb_ip_conflict);

    // Open socket 0 for DHCP on port 68
    socket(0, Sn_MR_UDP, 68, 0);

    // Initialize DHCP with buffer
    uint8_t dhcp_buffer[548];
    DHCP_init(0, dhcp_buffer);

    // Run DHCP loop
    uint8_t dhcp_result = 0;
    uint32_t dhcp_tick = HAL_GetTick();
    while(dhcp_result != DHCP_IP_ASSIGN &&
        dhcp_result != DHCP_IP_CHANGED &&
        dhcp_result != DHCP_IP_LEASED) {

        dhcp_result = DHCP_run();

        char dhcp_str[32];
        sprintf(dhcp_str, "DHCP: %d    ", dhcp_result);
        ST7789_WriteString(10, 85, dhcp_str, Font_11x18, YELLOW, BLACK);

        if(HAL_GetTick() - dhcp_tick >= 1000) {
            DHCP_time_handler();
            dhcp_tick = HAL_GetTick();
        }
        HAL_Delay(10);
    }
    ST7789_WriteString(10, 85, "DHCP OK!  ", Font_11x18, GREEN, BLACK);
    HAL_Delay(500);

    // Extract assigned network info
    uint8_t ip[4], gw[4], sn[4];
    getIPfromDHCP(ip);
    getGWfromDHCP(gw);
    getSNfromDHCP(sn);

    // Apply full network info to W5500
    wiz_NetInfo netinfo = {
        .mac  = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01},
        .dhcp = NETINFO_DHCP
    };
    memcpy(netinfo.ip, ip, 4);
    memcpy(netinfo.gw, gw, 4);
    memcpy(netinfo.sn, sn, 4);
    wizchip_setnetinfo(&netinfo);

    // Close DHCP socket, open RTT socket
    DHCP_stop();
    close(0);
    socket(0, Sn_MR_UDP, 4000, 0);
}

/*
Method to run RTT measurement. This will be called in the main loop and will handle sending packets and measuring RTT.
*/
void Network_RunRTT(void) {
    // Capture send Timestamp
    uint32_t t1 = __HAL_TIM_GET_COUNTER(&htim2);

    // build the packet
    uint8_t packet[4];
    packet[0] = (t1 >> 24) & 0xFF;
    packet[1] = (t1 >> 16) & 0xFF;
    packet[2] = (t1 >> 8)  & 0xFF;
    packet[3] = (t1)       & 0xFF;

    // send the packet
    uint8_t dest_ip[4] = {129, 158, 210, 211};
    sendto_W5x00(0, packet, 4, dest_ip, 5000);

    // wait for echo with timeout
    uint32_t timeout_start = HAL_GetTick();
    uint16_t received_size = 0;

    while(received_size == 0) {
        received_size = getSn_RX_RSR(0);
        if(HAL_GetTick() - timeout_start > 5000) {
            rtt_stats.lost_count++;
            return;
        }
        HAL_Delay(1);
    }

    // Receive the echo
    uint8_t recv_buf[4];
    uint8_t recv_ip[4];
    uint16_t recv_port;
    recvfrom_W5x00(0, recv_buf, 4, recv_ip, &recv_port);

    // Capture receive Timestamp
    uint32_t t2 = __HAL_TIM_GET_COUNTER(&htim2);

    // handle timer overflow
    uint32_t ticks;
    if(t2 >= t1) {
        ticks = t2 - t1;
    } else {
        ticks = (0xFFFFFFFF - t1) + t2 + 1;
    }

    // Convert ticks to microseconds
    uint32_t rtt_us = ticks / 84;

    // Update RTT statistics
    rtt_stats.current_us = rtt_us;
    rtt_stats.packet_count++;

    // min/max
    if(rtt_stats.packet_count == 1) {
        rtt_stats.min_us = rtt_us;
        rtt_stats.max_us = rtt_us;
    } else {
        if(rtt_us < rtt_stats.min_us) rtt_stats.min_us = rtt_us;
        if(rtt_us > rtt_stats.max_us) rtt_stats.max_us = rtt_us;
    }

    // circular buffer
    rtt_stats.samples[rtt_stats.sample_index] = rtt_us;
    rtt_stats.sample_index = (rtt_stats.sample_index + 1) % 32;

    // rolling mean
    uint32_t sum = 0;
    for(int i = 0; i < 32; i++) sum += rtt_stats.samples[i];
    uint8_t sample_count = rtt_stats.packet_count < 32 ? rtt_stats.packet_count : 32;
    rtt_stats.mean_us = sum / sample_count;

    // stddev
    uint32_t variance = 0;
    for(int i = 0; i < 32; i++) {
        int32_t diff = (int32_t)rtt_stats.samples[i] - (int32_t)rtt_stats.mean_us;
        variance += (diff * diff);
    }
    rtt_stats.stddev_us = sqrt(variance / 32);

}