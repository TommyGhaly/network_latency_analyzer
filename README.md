# Packet Sender for Network Latency Analyzer

This project implements a packet sender component for a network latency analyzer using an STM32F446 microcontroller board with a WIZnet W5500 Ethernet chip.

## Features

- **Microcontroller**: STM32F446RE
- **Ethernet**: WIZnet W5500 chip for network connectivity
- **Display**: ST7789 LCD display for status and latency information
- **Network Protocols**: Includes libraries for DHCP, DNS, HTTP, MQTT, SNMP, SNTP, TFTP, and more
- **Communication**: SPI for Ethernet, UART for serial communication, I2C for peripherals

## Project Structure

- `packet_sender/`: Main STM32CubeIDE project
  - `Core/`: Core application code
    - `Ethernet/`: WIZnet Ethernet drivers and socket APIs
    - `Internet/`: Network protocol implementations (DHCP, DNS, HTTP, MQTT, etc.)
    - `Inc/`: Header files
    - `Src/`: Source files including main.c
  - `Drivers/`: STM32 HAL drivers and CMSIS
  - `Debug/`: Build artifacts and makefiles

## Getting Started

1. Clone the repository.
2. Open the STM32CubeIDE project in `packet_sender/`.
3. Configure the project settings if necessary (e.g., pin assignments for SPI, I2C, UART).
4. Build and flash the firmware to the STM32F446 board.
5. Connect the board to a network via Ethernet.
6. Power on the board; it will initialize the display and Ethernet chip.

## Requirements

- STM32CubeIDE
- STM32F446 development board
- WIZnet W5500 Ethernet module
- ST7789 LCD display
- Network setup for Ethernet connectivity

## Current Status

The project currently initializes the hardware components (Ethernet chip, display) but the main packet sending logic is not yet implemented. The infinite loop in main.c is empty, awaiting further development for latency measurement functionality.