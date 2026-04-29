# Network Latency Analyzer

This project implements a network latency analyzer using two STM32 microcontroller boards:
- **STM32F411** (Sender): Sends packets over the network.
- **STM32F446** (Receiver/Display): Receives packets and displays latency information.

## Project Structure

- `common/`: Shared code for packet handling, protocol, timing, and transport.
- `send_F411/`: STM32F411 project for sending packets.
- `recieve_display_F446/`: STM32F446 project for receiving and displaying latency data.

## Getting Started

1. Clone the repository.
2. Open the respective STM32CubeIDE projects in `send_F411/` and `recieve_display_F446/`.
3. Build and flash the firmware to the boards.
4. Connect the boards via the network interface.
5. Run the latency analysis.

## Requirements

- STM32CubeIDE
- STM32F411 and STM32F446 development boards
- Network setup for communication between boards

## License

[Add license information here]