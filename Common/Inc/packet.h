#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

/* =======================
   Packet Constants
   ======================= */

#define PACKET_START_BYTE   0xAA
#define PACKET_END_BYTE     0x55

#define PACKET_TYPE_REQUEST  0x01
#define PACKET_TYPE_RESPONSE 0x02

#define PACKET_SIZE 9   // total bytes


/* =======================
   Packet Structure (Logical)
   ======================= */
/*
   Byte layout (DO NOT rely on compiler layout):

   [0] START       (1B)
   [1] TYPE        (1B)
   [2] ID          (2B)
   [4] TIMESTAMP   (4B)
   [8] END         (1B)
*/

typedef struct {
    uint8_t  type;
    uint16_t id;
    uint32_t timestamp;
} Packet;


/* =======================
   Serialization API
   ======================= */

/* Build raw byte buffer from Packet */
void packet_encode(Packet* pkt, uint8_t* buffer);

/* Parse raw byte buffer into Packet */
int packet_decode(uint8_t* buffer, Packet* pkt);

/* Validate raw packet buffer */
int packet_validate(uint8_t* buffer);


#endif
