# Hardware Network Latency Analyzer

A bare-metal embedded systems project implementing a real-time WAN latency measurement device using an STM32F446RE microcontroller and W5500 Ethernet controller. Designed as a portable, plug-anywhere network diagnostic tool relevant to high-frequency trading infrastructure.

---

## Demo
https://github.com/user-attachments/assets/d2d8b7ce-8f79-447e-b33a-76989d13375f

The device boots, negotiates a DHCP IP address on any network, and immediately begins measuring round-trip latency to a cloud echo server — displaying live statistics and a scrolling jitter graph on a 1.54" TFT display.

**Measured metrics:**
- Round-trip time (RTT) in microseconds
- Rolling mean, min, max
- Jitter (standard deviation over last 32 samples)
- Packet count and loss rate
- Scrolling bar graph of last 32 RTT samples, color-coded by latency

---

## Hardware

| Component | Part |
|-----------|------|
| Microcontroller | STM32F446RE (NUCLEO-F446RE) |
| Ethernet Controller | WIZnet W5500 |
| Display | 1.54" IPS TFT LCD, 240×240 RGB (ST7789) |
| Cable | Cat6 Ethernet |
| Clock | 84MHz system clock, TIM2 hardware timer |

**Pin Mapping:**

| W5500 Pin | STM32 Pin | Function |
|-----------|-----------|----------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SCLK | PA5 | SPI1 Clock |
| MOSI | PA7 | SPI1 MOSI |
| MISO | PA6 | SPI1 MISO |
| CS | PB6 | Chip Select |
| RST | PA9 | Hardware Reset |
| INT | PA8 | Packet Arrival Interrupt |

---

## Architecture

### Firmware (STM32 — bare metal C, STM32 HAL)

**Network layer (`network.c`)**
- W5500 hardware reset and SPI initialization via WIZnet ioLibrary
- DHCP client for automatic IP assignment on any network
- UDP socket management
- RTT measurement loop with hardware interrupt timestamping

**Display layer (`st7789.c`)**
- Full ST7789 driver implementation
- Live stats rendering with in-place updates (no flicker)
- Scrolling bar graph using circular sample buffer

**Timestamping**
- TIM2 configured as a 32-bit free-running counter at 84MHz (11.9ns resolution)
- W5500 INT pin wired to PA8 as EXTI interrupt
- On packet arrival, INT pin goes LOW → EXTI fires → TIM2 captured in ISR
- This eliminates software polling jitter, achieving hardware-level timestamp precision

### Cloud (Oracle Cloud — Always Free tier)
- Lightweight UDP echo server running as a persistent systemd service
- Receives packets, echoes immediately with zero processing overhead
- Permanently hosted at a reserved public IP

---

## How It Works

```
STM32 captures TIM2 (t1)
        │
        ▼
W5500 sends UDP packet ──► [Internet] ──► Oracle echo server
                                                │
                                          echo immediately
                                                │
W5500 INT pin fires ◄──── [Internet] ◄─────────┘
        │
STM32 ISR captures TIM2 (t2)
        │
RTT = (t2 - t1) / 84 microseconds
```

---

## Why This Is Relevant to HFT

High-frequency trading firms colocate servers in data centers physically adjacent to exchange matching engines to minimize network latency. Microseconds directly translate to trading advantage — a firm with 10μs lower latency than a competitor will consistently win order priority.

This device demonstrates the same measurement principles used in HFT network infrastructure:

- **Hardware timestamping** rather than software clocks eliminates OS scheduling jitter from measurements
- **Dedicated network controller** (W5500) offloads TCP/IP stack to hardware, same principle as kernel-bypass networking (DPDK, RDMA) used in production HFT
- **Jitter measurement** (stddev of RTT) is more operationally relevant than raw latency in HFT — predictable latency matters as much as low latency
- **Nanosecond resolution** using hardware timers mirrors exchange co-location timestamping standards (PTP/IEEE 1588)

---

## Technical Highlights

- Written entirely in **bare metal C** with STM32 HAL — no RTOS, no middleware
- **Interrupt-driven receive timestamping** via EXTI on W5500 INT pin — timestamp captured within ~100ns of packet arrival
- **DHCP client** implemented using WIZnet ioLibrary — device negotiates IP automatically on any network
- **Shared SPI bus** management between W5500 and ST7789 with proper CS arbitration
- **Circular buffer** with rolling statistics (mean, stddev) computed on-device
- **Persistent cloud infrastructure** — Oracle Always Free instance running UDP echo server as systemd service, zero ongoing cost

---

## Build Environment

- STM32CubeIDE
- STM32CubeMX for peripheral configuration
- WIZnet ioLibrary Driver (W5500, DHCP)
- ARM GCC toolchain

---

## Author

Tommy Ghaly  
Northeastern University — CS & Mathematics  
ghaly.t@northeastern.edu
