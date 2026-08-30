# STM32G0 & Arduino Bi-Directional CAN Bus Communication

This project implements a reliable, **bi-directional CAN Bus (Controller Area Network)** communication protocol between an **STM32G070RBT6** microcontroller and an **Arduino Uno** using **MCP2515 CAN Controllers** and **TJA1050 Transceivers**.

## 📌 Project Overview
The system establishes a Master-Slave (Query-Response) structure over a physical CAN bus network running at **500 kbps**:
1. **Arduino (Master/Sender):** Periodically transmits a data frame with `ID: 0x123` to the CAN network.
2. **STM32 (Slave/Receiver):** Listens to the CAN bus using low-level SPI register operations, parses incoming frames, and immediately sends back an acknowledgment response frame with `ID: 0x321`.

---

## 🛠️ Hardware & Network Specifications

- **Baud Rate:** 500 kbps
- **Clock Source:** 8 MHz Crystal
- **Bus Termination:** 120 Ω termination resistors enabled at both physical node ends.
- **Transceiver Power:** 5V DC (Required for TJA1050 operation).

### Pin Mapping

#### **STM32G070RBT6 <---> MCP2515 (SPI1)**
| MCP2515 Pin | STM32 Pin | Function |
| :--- | :--- | :--- |
| **VCC** | `5V` / `VIN` | 5V Power Supply |
| **GND** | `GND` | Common Ground |
| **CS** | `PB6` | SPI Chip Select (GPIO Output) |
| **SCK** | `PA1` | SPI Clock |
| **SI (MOSI)** | `PA2` | Serial Data In |
| **SO (MISO)** | `PA6` | Serial Data Out |

#### **Physical Bus Connections**
| Node 1 (Arduino MCP2515) | Node 2 (STM32 MCP2515) |
| :--- | :--- |
| `CANH` | `CANH` |
| `CANL` | `CANL` |
| `GND` | `GND` (Common Reference) |

---

## 💻 Firmware Architecture

- **STM32 Driver:** Custom register-level driver built over STM32 HAL `SPI1` peripheral to directly manipulate MCP2515 control, status, and transmission registers (`TXB0`, `RXB0`, `CANINTF`).
- **Arduino Firmware:** Developed using standard `mcp_can` library operating in `MCP_NORMAL` mode with explicit 500ms timeout handling for response frames.

---

## 📊 Verification & Test Results

The bi-directional link was validated via Serial Monitor logs at 115200 baud, demonstrating consistent frame transmission and frame acknowledgment:

```text
[TX] -> Sending request to STM32 (ID: 0x123)...
[RX] <- Response received from STM32! ID: 0x321 | Data: AA BB CC DD 01 02 03 04
