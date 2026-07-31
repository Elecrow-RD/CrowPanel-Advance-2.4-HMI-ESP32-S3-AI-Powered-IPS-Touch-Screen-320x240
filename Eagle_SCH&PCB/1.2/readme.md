# 2.4-Inch ESP32-S3 Display Product Hardware Driver Guide

| Item | Details |
|---|---|
| Document Version | V1.0 |
| Applicable Hardware | ESP32 Display 2.4 inch V1.2 |
| Preparation Date | 2026-07-30 |
| Author | OpenAI Codex (cross-validated against project materials) |
| Document Status | Baseline version suitable for maintenance, porting, and handoff |

## 1. Document Purpose and Determination Rules

This document cross-validates the onboard and supported peripherals based on the mainboard Eagle schematic/PCB, schematic PDF, and verified Arduino examples in the project. Its purpose is not to repeat component datasheets, but to specify the pins, buses, initialization sequence, software dependencies, and resource conflicts actually used by this product.

The evidence priority is as follows:

1. Actual execution paths and parameters in verified examples;
2. Net connections in `1.2/ESP32 Display 2.4 inch V1.2.sch`;
3. `.brd`, PDF, code comments, and unexecuted candidate configurations;
4. Conventional component usage is used only as supporting information and does not override the configuration verified in this project.

If the code and schematic are inconsistent, this document follows the verified code as required and records the differences in Chapter 6. Components that exist in third-party libraries but are neither connected to this board nor called by the examples are not considered peripherals of this product.

## 2. Reference Baseline

| Type | Path | Purpose |
|---|---|---|
| Mainboard schematic source file | `1.2/ESP32 Display 2.4 inch V1.2.sch` | Primary evidence for components, net names, and electrical connections |
| PCB source file | `1.2/ESP32 Display 2.4 inch V1.2.brd` | Verification of assembled components, NC status, and layout |
| Mainboard schematic PDF | `1.2/ESP32-Display-2.4-inch-V1.2.pdf` | Manual graphical verification |
| LCD/LVGL/touch | `2.4_2.8_Arduino/lesson-03/2.4_2.8_LVGL/` | Verified configuration for LCD, backlight, GT911, and LVGL |
| Backlight PWM | `2.4_2.8_Arduino/lesson-03/pwn version/pwm/pwm.ino` | LEDC PWM parameters |
| SD card | `2.4_2.8_Arduino/lesson-04/SD_CrowPanel_ESP32_Advance_HMI_2.4_2.8/` | Verified SPI SD configuration |
| Audio output | `2.4_2.8_Arduino/lesson-02/OnlineAudio_small/OnlineAudio_small.ino` | I2S amplifier driver configuration |
| nRF24 | `2.4_2.8_Arduino/lesson-06/READ/`, `WRITE/` | External nRF24L01 transceiver configuration |
| LoRaWAN | `2.4_2.8_Arduino/lesson-07/code/sendATcommands_2.4_2_8/` | External SX1262 and RadioLib parameters |
| Zigbee/UART | `2.4_2.8_Arduino/lesson-08/zigbee_2.4_2_8/` | External wireless module UART configuration |
| Zigbee module schematic | `2.4_2.8_Arduino/lesson-08/Wireless_Module_ESP32-H2_V1.1_Schematic.pdf` | Reference for the supported external module; not an onboard mainboard component |

Note: The repository does not contain Git metadata, so commit history cannot be used to establish the historical “verified” version. This document treats the current tutorial example code as the user-specified verified baseline.

## 3. Peripheral Overview

| Category | Peripheral/Component | Onboard | Primary Interface | Key Pins | Software Status |
|---|---|---:|---|---|---|
| MCU | ESP32-S3-WROOM-1 | Yes | GPIO Matrix | See Chapter 4 | Arduino-ESP32 |
| Display | 2.4-inch 320×240 TFT, handled as ST7789 in code | Yes | SPI2, Mode 0 | SCK42/MOSI39/DC41/CS40 | LovyanGFX verified |
| Touch | Capacitive touch, handled as GT911 in code | On display FPC | I2C0 | SDA15/SCL16; verified reset sequence uses 1/2 | TAMC_GT911/LovyanGFX paths coexist |
| Backlight | TFT LED + transistor | Yes | GPIO/PWM | GPIO38 | Digital switching and LEDC verified |
| Storage | MicroSD | Yes | SPI/HSPI | MISO4/SCK5/MOSI6/CS7 | Arduino SD/FS verified |
| Audio output | NS4168 digital amplifier + speaker connector | Yes | I2S | BCLK13/LRCLK11/DATA12/CTRL21 | ESP32-audioI2S verified |
| Audio input | PDM microphone | Yes | PDM | CLK9/DATA10 | Confirmed by schematic; no capture example found |
| Alert device | Active/passive buzzer B1 | Yes | GPIO | GPIO8, through NPN | Confirmed by schematic; no example found |
| USB debugging | CH340K + USB-C J3 | Yes | UART0/USB | ESP TXD0/RXD0, through level shifting | Automatic download circuitry implemented |
| Native USB | USB-C J4 | Yes | USB 2.0 | GPIO19 D- / GPIO20 D+ | Arduino USB CDC/JTAG available; no dedicated project example |
| Expansion UART | HY2.0 J6 | Yes | UART1 | TX17/RX18 | Confirmed by schematic |
| Expansion I2C | HY2.0 J7 | Yes | I2C | SDA15/SCL16 | Shares the bus with touch |
| Wireless expansion SPI | J8/J9 | Yes | SPI + control | MISO9/MOSI3/SCK10; control pins vary by module | nRF24/SX1262 verified |
| nRF24L01 | External module | No | SPI | CE1/CSN2/MISO9/MOSI3/SCK10 | RF24 verified |
| SX1262 LoRa | External module | No | SPI | NSS0/DIO1=1/NRST2/BUSY46 | RadioLib verified |
| Zigbee/ESP32-H2 | External module | No | UART1 | ESP RX2/TX1 | Serial reception verified |
| Wi-Fi/BLE | Integrated into ESP32-S3 | Yes | On-chip radio | No external GPIO | Wi-Fi audio example verified |
| Battery charging | TP4059 + battery connector J10 | Yes | Analog power | CHRG/DONE monitored by STC8 | Autonomous hardware operation |
| Boost/system power | RY3420, AO3401, Schottky OR | Yes | Analog power | No ESP initialization | Autonomous hardware operation |
| Power/bus switches | SGM3799, MOSFET switches | Yes | GPIO control | GPIO45, 9, 10, 14, 21 | Partially controlled by examples |
| Battery/charging indicator | STC8G1K08A + bi-color LED | Yes | Independent MCU | CHRG/DONE/TXD/RXD | Firmware not included in this repository |
| Buttons | RESET K2, BOOT K1 | Yes | Active-low | EN, GPIO0 | Hardware boot/download |

## 4. Unified GPIO Resource Table

| GPIO | Schematic Net/Connection | Verified Use | Electrical/Multiplexing Notes |
|---:|---|---|---|
| 0 | `IO0_W_CS`, BOOT button, J8 | SX1262 NSS | Boot strapping pin; peripherals must not hold it low during power-up/reset |
| 1 | `IO1_ESP_TXD2`, J9 | nRF24 CE; SX1262 DIO1; Zigbee TX; temporarily used by the verified GT911 reset sequence | Severe multi-scenario conflict; cannot be used concurrently |
| 2 | `IO2_ESP_RXD2`, J8 | nRF24 CSN; SX1262 NRST; Zigbee RX; temporarily used by the verified GT911 reset sequence | Severe multi-scenario conflict; cannot be used concurrently |
| 3 | `IO3_W_MOSI`, J9 | Wireless expansion MOSI | SPI push-pull output |
| 4 | `IO4_SD_MISO` | MicroSD MISO | SPI input; card socket includes pull-up |
| 5 | `IO5_SD_SCK` | MicroSD SCK | SPI clock output |
| 6 | `IO6_SD_MOSI` | MicroSD MOSI | SPI push-pull output |
| 7 | `IO7_SD_CS` | MicroSD CS | Active-low; keep high when idle |
| 8 | `IO8_BEEP` | Buzzer | Drives an NPN through 1 kΩ; should default low |
| 9 | SGM3799 COM2 | Wireless MISO or PDM MIC CLK | Physical path selected by GPIO45; direction changes by scenario |
| 10 | SGM3799 COM3 | Wireless SCK or PDM MIC DATA | Physical path selected by GPIO45; direction changes by scenario |
| 11 | `IO11_I2S_LRCLK` | Amplifier LRCLK | I2S output |
| 12 | `IO12_I2S_SDIN` | Amplifier serial audio data | I2S output; net name is assigned from the amplifier’s perspective |
| 13 | `IO13_I2S_BCLK` | Amplifier BCLK | I2S output |
| 14 | `IO14_TFT_PWR` | TFT/touch power control circuit | Current display examples do not configure it actively; verify the assembly revision before making changes |
| 15 | `IO15_SDA` | Touch and J7 SDA | I2C open-drain; schematic shows 4.7 kΩ pull-up to 3.3 V |
| 16 | `IO16_SCL` | Touch and J7 SCL | I2C open-drain; schematic shows 4.7 kΩ pull-up to 3.3 V |
| 17 | `ESP_TXD1` | J6 UART TX | 3.3 V CMOS |
| 18 | `ESP_RXD1` | J6 UART RX | 3.3 V CMOS; the LVGL example only configures it as an unused output, and this ineffective configuration should be removed |
| 19 | `ESP_D-` | Native USB D- | Do not use as a general-purpose GPIO |
| 20 | `ESP_D+` | Native USB D+ | Do not use as a general-purpose GPIO |
| 21 | `IO21_NS_CTRL` | NS4168 control | Example drives it low to enable audio; polarity follows the code |
| 38 | `IO38_LED_BK` | Backlight enable/PWM | Active-high; driven through a transistor and does not carry LED current directly |
| 39 | `IO39_TFT_SDA` | LCD MOSI | Write-only path; code sets MISO=-1 |
| 40 | `IO40_TFT_CS` | LCD CS | Active-low |
| 41 | `IO41_TFT_RS` | LCD D/C | Command/data selection |
| 42 | `IO42_TFT_SCK` | LCD SCK | Highest verified configuration is 80 MHz |
| 45 | `IO45_SPISW_ITCHING` | SGM3799 bus selection | Wireless examples drive it low; boot strapping pin, so external voltage must be handled carefully during reset |
| 46 | `IO46_BOOT_BUSY` | SX1262 BUSY, J8 | ESP32-S3 input-only/strapping-related pin; suitable for BUSY input and must not be used as a general-purpose output |
| 47 | `IO47_TP_INT` | Touch INT in schematic | Current GT911 `touch.h` path configures it as -1 and uses polling; LovyanGFX candidate configuration specifies 47 |
| 48 | `IO48_TP_RST` | Touch RST in schematic | Not used by the currently verified code; major discrepancy exists |

## 5. Detailed Driver Instructions for Each Peripheral

### 5.1 ESP32-S3-WROOM-1 MCU

- Power: The module’s `3V3` is connected to `ESP_3V3` through FB1; EN is controlled by a 10 kΩ pull-up, a 1 µF RC circuit, and the RESET button.
- Boot: The BOOT button pulls GPIO0 low; the RESET button pulls EN low. For downloading, keep GPIO0 low before resetting.
- Software layer: Arduino-ESP32, with ESP-IDF HAL/drivers underneath. The graphics example does not explicitly create RTOS tasks, but the Arduino core runs FreeRTOS internally.
- Memory: The LVGL example allocates two 320×240 RGB565 full-screen buffers in PSRAM. Each buffer is 153,600 bytes, for a total of approximately 300 KiB. PSRAM must be enabled for the target module/in the Arduino menu.
- Serial logging: All examples use `Serial.begin(115200)`.

### 5.2 ST7789 TFT LCD

**Connections and Protocol**

| Signal | GPIO | Configuration |
|---|---:|---|
| SCK | 42 | SPI2_HOST clock, push-pull |
| MOSI/SDA | 39 | Unidirectional write data, push-pull |
| D/C/RS | 41 | GPIO output |
| CS | 40 | GPIO/SPI chip select, active-low |
| RST | No software GPIO | `pin_rst=-1`; handled by the RC/power network in the schematic |
| BL | 38 | Active-high or PWM |

Key configuration: SPI Mode 0, 80 MHz write frequency, 16 MHz read-frequency field, but `MISO=-1` and `readable=false`; automatic DMA channel; panel memory size of 240×320; logical display resolution of 320×240; `invert=true`, `rgb_order=false`, and `offset_rotation=3`.

Software dependencies: LovyanGFX `lgfx::Panel_ST7789` + `lgfx::Bus_SPI`. The UI example additionally uses LVGL with RGB565, dual full-screen PSRAM buffers, and full-refresh mode, with `lv_timer_handler()` called approximately every 5 ms.

Key configuration example:

```cpp
cfg.spi_host = SPI2_HOST;
cfg.spi_mode = 0;
cfg.freq_write = 80000000;
cfg.pin_sclk = 42; cfg.pin_mosi = 39; cfg.pin_dc = 41;
panel.pin_cs = 40; panel.pin_rst = -1;
```

Initialization sequence: Call `gfx.init()` first. If DMA is required, then call `gfx.initDMA()` and `gfx.startWrite()`. Finally, drive GPIO38 high to turn on the backlight. When a white or black screen occurs, first distinguish between “LCD not initialized” and “backlight not enabled.”

### 5.3 Capacitive Touch (Handled as GT911 in the Actual Code)

- Bus: I2C0, SDA=15, SCL=16; 3.3 V open-drain bus with a 4.7 kΩ pull-up on each line on the mainboard.
- Verified address: `0x5D`, passed to TAMC_GT911 by `touch_init(0x5D)`.
- Acquisition method: The current `touch.h` sets both INT and RST to `-1` and polls through repeated `ts.read()` calls without relying on GPIO interrupts.
- Coordinates: The raw range is configured as 480×320 and then mapped to 320×240. The primary LVGL path ultimately uses `x=320-touchX, y=touchY`. The coordinates must be recalibrated when porting to a different rotation.
- Software dependencies: Arduino `Wire` and TAMC_GT911. The LovyanGFX file also contains a candidate `Touch_FT5x06` configuration that is not part of the current primary path.

Special power-up sequence in the verified code:

```cpp
Wire.begin(15, 16);
pinMode(1, OUTPUT); pinMode(2, OUTPUT);
digitalWrite(1, LOW); digitalWrite(2, LOW); delay(20);
digitalWrite(2, HIGH); delay(100); pinMode(1, INPUT);
touch_init(0x5D);
```

GPIO1/2 above are inconsistent with `TP_INT=47` and `TP_RST=48` in the schematic. They are retained based on the verified code, but must be revalidated with an oscilloscope/logic analyzer before porting or combining wireless functionality. See Section 6.1.

### 5.4 LCD Backlight

- GPIO38 is active-high and drives the onboard NPN/current-limiting network; the ESP32 does not power the LED directly.
- Basic switching: `pinMode(38, OUTPUT); digitalWrite(38, HIGH);`.
- Dimming: Arduino-ESP32 LEDC, 5 kHz, 8 bit, duty cycle 0–255. After `ledcAttach(38, 5000, 8)`, use `ledcWrite(38, duty)`.
- Risk: A 0 duty cycle makes the screen completely dark and can easily be mistaken for an application crash. During debugging, set it to 255 first.

### 5.5 MicroSD

| Signal | GPIO | Direction |
|---|---:|---|
| MISO | 4 | Input |
| SCK | 5 | Output |
| MOSI | 6 | Output |
| CS | 7 | Output, active-low |

- Driver method: Dedicated `SPIClass(HSPI)` with Arduino `SD`/`FS`, not SDMMC 4-bit mode.
- Verified mount parameters: `SD.begin(7, SD_SPI, 80000000)`, requesting 80 MHz.
- Initialization: Call `SD_SPI.begin(5, 4, 6)` before mounting; call `SD_SPI.end()` if mounting fails.
- Electrical: The schematic provides 10 kΩ pull-ups for DATA/CS and related signals; the card socket is powered by 3.3 V.
- Risk: Operation at 80 MHz is sensitive to card quality, routing, and the Arduino core’s actual frequency limits. If porting produces errors, first reduce the frequency to 20–40 MHz for verification rather than immediately concluding that the card is defective.

### 5.6 NS4168 I2S Amplifier and Speaker

| Signal | GPIO | Description |
|---|---:|---|
| BCLK | 13 | I2S bit clock |
| LRCLK/LRC | 11 | Left/right-channel frame clock |
| SDATA | 12 | ESP32 output to amplifier |
| CTRL | 21 | Amplifier control; verified as active-low |

- Software dependencies: The `Audio` class from `ESP32-audioI2S` and Arduino Wi-Fi. The example decodes an HTTP MP3 stream and outputs it over I2S.
- Initialization sequence: Drive GPIO21 low → connect to Wi-Fi → `audio.setPinout(13,11,12)` → `audio.setVolume(20)` → `connecttohost()` → continuously call `audio.loop()` in the main loop.
- The volume range is defined by the current library; the example comment specifies 0–21. A value of 20 is close to maximum. The production default should be reduced, and speaker temperature rise/distortion should be tested.
- The output is differential `ROUT+/-` to the 2-pin speaker connector. Neither terminal may be treated as ground, and it must not be connected to the input of a single-ended powered speaker.

### 5.7 PDM Microphone

- Component: LMD3526B261-OFA01, powered by 3.3 V filtered through FB5.
- Signals: GPIO9=`MIC_CLK`, GPIO10=`MIC_SD`; the L/R hardware level selects a fixed channel.
- Software status: The schematic connections have been confirmed, but the repository contains no board-level PDM capture example. The sampling rate, PDM clock, and DMA parameters have not yet formed a “verified baseline.”
- Multiplexing: GPIO9/10 are multiplexed through SGM3799 with wireless expansion MISO/SCK. Wireless examples drive GPIO45 low to select the wireless path. Before enabling the microphone, switch to the other path and verify the selection polarity through testing.
- Porting recommendation: Use the Arduino/ESP-IDF I2S PDM RX driver, starting with 16 kHz, 16 bit, mono for validation. These are recommended values, not parameters verified by the project.

### 5.8 Buzzer

- GPIO8 drives an SS8050 NPN through a 1 kΩ resistor. The other end of the buzzer is connected to 3.3 V and protected by a reverse diode.
- Logic: When GPIO8 is high, the transistor conducts and powers the buzzer. Drive it low by default to turn the buzzer off.
- For a passive buzzer, use LEDC PWM to generate a tone. For an active buzzer, only high and low levels are required. The schematic part number is insufficient to reliably determine the acoustic type; verify it against the physical BOM/with an oscilloscope during maintenance.

### 5.9 USB, Downloading, and Debug UART

**USB-C J3 + CH340K**: USB D+/D- from J3 connect to the CH340K. The CH340K UART connects to ESP UART0 through BSS138 level shifting. DTR/RTS automatically control EN/GPIO0 through UMH3NTN, implementing automatic download mode. The J3 CC pins use 5.1 kΩ pull-downs, configuring it as a USB Device/power sink.

**USB-C J4 Native USB**: GPIO19=D- and GPIO20=D+, connected to J4 through 0 Ω series resistors. Arduino-ESP32 can configure this path as a USB CDC/JTAG/OTG device. J4 is also configured as a Type-C power sink; the two USB-C ports must not be treated as isolated power inputs.

- Debug baud rate: All examples use `115200`.
- The external UART0 header J5 also carries 5 V/GND. Although the logic side passes through onboard level conversion, verify the voltage level of the external device before connecting it.
- Once GPIO19/20 are assigned to native USB, they must not be allocated to other functions.

### 5.10 Expansion UART and I2C

- J6: ESP TXD1=GPIO17, RXD1=GPIO18, controllable 3.3V_OUT1, and GND.
- J7: SCL=GPIO16, SDA=GPIO15, controllable 3.3V_OUT2, and GND. It shares the I2C bus with the touch controller; additional devices on the bus must not conflict with the GT911 address.
- The two 3.3 V outputs are controlled by AO3401 high-side switches, but the corresponding gate networks are not explicitly managed by the existing examples. Do not assume that power is always present after startup; measure the 3.3 V pins on J6/J7 during maintenance.
- Keep I2C at the 400 kHz upper limit already shown in the code. Reduce it to 100 kHz for long cables or multiple modules connected in parallel, and avoid excessively strong pull-ups caused by duplicate resistors.

### 5.11 External nRF24L01 Module

| Function | GPIO |
|---|---:|
| CE | 1 |
| CSN | 2 |
| MISO | 9 |
| MOSI | 3 |
| SCK | 10 |
| IRQ | Not used |

- Initialization: Drive GPIO45 low to select wireless SPI; call `SPIClass(HSPI).begin(10,9,3,46)`, followed by `radio.begin(hspi)`. The supplied SS=46 is only the default SS for SPIClass; the RF24’s actual CSN is GPIO2.
- Radio parameters: Address `"00001"` (5 bytes), PA=`RF24_PA_MAX`, data rate 250 kbps, and channel 50 (center frequency approximately 2450 MHz). The transmitter calls `stopListening()`, while the receiver configures pipe 0 and then calls `startListening()`.
- Software layer: Arduino SPI + RF24 library.
- Power risk: PA_MAX produces relatively high transient current. The external module must use 3.3 V and have adequate decoupling placed near the module. Connecting VCC to 5 V is strictly prohibited.

### 5.12 SX1262 LoRa/LoRaWAN External Module

| Function | GPIO |
|---|---:|
| NSS | 0 |
| DIO1 | 1 |
| NRST | 2 |
| BUSY | 46 |
| MISO/MOSI/SCK | 9/3/10 |

- Driver: Arduino SPI + RadioLib `SX1262`/`LoRaWANNode`.

- Initialization: Drive GPIO45 low to select the wireless path; call `SPI.begin(10,9,3,0)` and `radio.begin()`; set the current limit to 140 mA and the TCXO voltage to 3.3 V.
- The default LoRaWAN region object is EU868 with subBand=1; the code also supports switching to US915. OTAA/ABP, ADR, DR, duty cycle, TX power, RX2 DR, and 400 ms dwell time settings are supported.
- Regulatory risk: The region, frequency, power, and duty cycle must be configured for the sales region. The default EU868/US915 values must not be used directly in China.
- GPIO0 is a bootstrapping pin. The SX1262/adapter board must release NSS while the ESP is resetting to prevent entry into download mode or boot failure.

### 5.13 Zigbee/ESP32-H2 External Module

- Interface: UART1, with ESP32-S3 RX=GPIO2 and TX=GPIO1, configured as `115200, 8N1`.
- Driving GPIO45 low selects/enables the wireless expansion path; the code receives data line by line from `Serial1` and forwards it to the USB serial port.
- Software dependency: Arduino HardwareSerial. The Zigbee stack runs on the external ESP32-H2 module, not on the main ESP32-S3.
- The schematic of the external module is supporting documentation. Its MCU, LEDs, buttons, and other components must not be incorrectly registered as onboard components of the main board.

### 5.14 Power Management and Charging

- Input: The two USB-C VBUS inputs and the J5 5 V input are combined through Schottky diodes into `VC_IN/VIN`, reducing the risk of reverse current but not providing complete isolation.
- Battery: J10 is a single-cell lithium battery connector, with a TP4059 linear charger. The `CHRG` and `DONE` status signals are routed to a dedicated STC8G1K08A, which drives a two-color LED.
- System power: An AO3401 power path and RY3420 boost converter, with 45.3 kΩ/10 kΩ feedback resistors, form the VIN/5 V system rail. The onboard power chain then generates 3.3 V.
- Software dependency: Charging and boost conversion operate autonomously in hardware and require no ESP32 initialization. The repository provides no interface for battery ADC voltage sampling or charging-status retrieval.
- Safety: Connect only a single-cell lithium battery. Verify polarity and the protection board. Do not simultaneously power multiple USB/5 V connectors from different voltage sources. Disconnect power before hot-plugging the speaker, display FPC, or wireless module.

### 5.15 STC8 Charging Indicator Coprocessor

- U14=STC8G1K08A, powered from VIN, reads the TP4059 `CHRG/DONE` signals and controls the two-color LED.
- TXD/RXD are routed to J11, which also includes VIN/GND and may be used for factory programming/debugging.
- The firmware source code and protocol are not included in this repository. Maintenance documentation must not claim that it can be upgraded online by the ESP32, and J11 must not be treated as a general-purpose ESP UART.

## 6. Schematic and Code Discrepancy Log

### 6.1 Confirmed Discrepancies

| ID | Item | Schematic/Candidate Configuration | Validated Code | Decision in This Document | Possible Cause/Action |
|---|---|---|---|---|---|
| D01 | Touch controller model | The LovyanGFX candidate class is FT5x06 at address 0x38 | The main program enables TAMC_GT911 at address 0x5D | Use GT911/0x5D | Multiple display configurations remain from the generic 2.4/2.8 project; remove the invalid FT5x06 block during porting |
| D02 | Touch INT/RST | Schematic: GPIO47=INT, GPIO48=RST | `touch.h` sets INT/RST=-1; the main program uses GPIO1/2 to perform an address-selection reset sequence | Document the GPIO1/2 sequence and classify it as the highest risk | Possible panel/adapter revision, example rewiring, or unsynchronized code; must be verified on physical hardware before mass production |
| D03 | Touch I2C frequency | The LovyanGFX candidate configuration specifies 400 kHz | The TAMC_GT911 path only calls `Wire.begin(15,16)` without explicitly setting the frequency | Do not claim a validated 400 kHz rate; treat 400 kHz only as the hardware-supported upper limit | Two driver paths coexist |
| D04 | LCD RST | The schematic FPC includes a `TFT_RST` RC network | The code uses `pin_rst=-1` | Use no software reset | Hardware RC reset prevents accidental GPIO allocation |
| D05 | LCD read frequency | The configuration includes `freq_read=16 MHz` | MISO=-1 and readable=false | Treat the LCD as write-only | The undeleted template field does not indicate readback capability |
| D06 | SD comments | Some `pins_config.h` files retain SDMMC comments for GPIO10–13 | The validated SPI pins are 4/5/6/7 | Use 4/5/6/7 | Generic template remnants |
| D07 | I2S comments | Some files retain comments specifying GPIO17/0/18 | The audio examples use DATA12/BCLK13/LRC11 | Use 12/13/11 | Old-board or generic-template remnants |
| D08 | SX1262 comments | The commented example specifies 10/2/3/9 | The actual instance uses NSS0/DIO1=1/RST2/BUSY46 | Use the actual constructor parameters | The comment applies to another board model |
| D09 | LCD/touch resolution | The touch file contains mapping limits of 480×320 | The product LCD/LVGL resolution is 320×240 | Use the final mapped resolution of 320×240 | Remnants from shared 2.4/2.8 multi-size code |

### 6.2 Discrepancy-Handling Principles

1. Runtime parameters must be derived only from code that is actually compiled and called. Comments, commented-out blocks, and uninstantiated objects must not be treated as valid configuration.
2. When combining multiple course examples, do not simply copy `setup()`; first allocate GPIO ownership as described in Chapter 4.
3. For conflicts involving hardware revisions, such as D02, prioritizing the code indicates only the current software baseline and does not mean that the schematic must be changed to match the code. Decide whether to revise the schematic or the driver only after measurements have been completed on physical hardware.
4. Update this table with all newly confirmed results, including the board number, prototype serial number, test firmware hash, and measurement evidence.

## 7. Key Conflicts and Risks

| Severity | Risk | Impact | Mitigation |
|---|---|---|---|
| High | GPIO1/2 are shared by the touch sequence, nRF24, SX1262, and Zigbee UART | Bus contention, peripheral resets, and boot failure | Make these functions mutually exclusive; even if GPIO ownership is transferred after touch initialization, electrical isolation of the peripherals must be confirmed |
| High | GPIO9/10 are switched between wireless SPI and the PDM microphone through an analog switch | Bidirectional conflicts; both the microphone and wireless interface may fail | Before switching GPIO45, disable both drivers and place the pins in a high-impedance state; verify the selection polarity through measurement |
| High | GPIO0/45/46 are subject to ESP32-S3 boot restrictions | Unable to boot or enter download mode | Ensure peripherals do not actively drive these pins during reset; do not use GPIO46 as an output |
| High | Touch connections differ between schematic GPIO47/48 and code GPIO1/2 | Touch may fail on new production batches | Perform address-scan and reset-waveform acceptance testing for every hardware batch |
| High | Power can be supplied through multiple connectors | Reverse current, circulating current, and overheating | Prohibit simultaneous connection of different power sources; verify the diodes and temperature rise |
| Medium | Both the LCD and SD request 80 MHz SPI | Signal-integrity and card-compatibility issues | Use separate buses; if problems occur, first verify behavior at a lower frequency and ensure proper CS management |
| Medium | Audio example volume is 20/21 | Speaker overload, clipping, and EMI | Reduce the default product volume and perform continuous sine-wave/music temperature-rise testing |
| Medium | I2C expansion and touch share the same lines | Address conflicts, excessive pull-up strength, and long-wire glitches | Maintain an address table; reduce the frequency to 100 kHz if necessary; calculate the effective parallel pull-up resistance |
| Medium | Two full-screen LVGL buffers depend on PSRAM | Allocation failure, crashes, or a black screen | Check the return value of `heap_caps_malloc`; enable PSRAM |
| Medium | Unclear external wireless VCC voltage may cause incorrect connection | Damage to 3.3 V devices | Power nRF24/SX1262 only from 3.3 V; measure the connector pinout before power-up |
| Low | The LVGL example unnecessarily configures GPIO18 as an output | Interference with the J6 UART RX signal | Remove the `pinMode(18, OUTPUT)` call or restore the pin to input mode before UART initialization |

## 8. Recommended Unified Initialization Sequence

1. Keep the wireless expansion and high-current loads disabled, and establish serial logging at 115200.
2. Determine ownership of GPIO1/2, 9/10, 45, and 46 once, based on the product feature matrix.
3. Configure the power supply/analog switch and wait for power to stabilize. Do not switch the SGM3799 while bus activity is in progress.
4. Initialize I2C and perform the touch reset/address-selection sequence; confirm a response at 0x5D.
5. Initialize LCD SPI2, clear the screen, and then enable the GPIO38 backlight.
6. Initialize independent SPI devices such as the SD card and wireless modules; read each device ID/status individually.
7. Initialize I2S/PDM. Keep the amplifier disabled or muted until the clocks are stable to prevent popping.
8. Initialize upper-layer services such as LVGL and networking; check PSRAM allocation and every driver return value.
9. After entering the main loop/RTOS tasks, ensure service functions such as `audio.loop()` and `lv_timer_handler()` are called at the required intervals.

## 9. Porting Checklist

- [ ] Confirm that both the hardware silkscreen and schematic version are V1.2.
- [ ] Confirm that the Arduino-ESP32 target is ESP32-S3 and that PSRAM is enabled.
- [ ] Fix the LCD configuration at SPI Mode 0, GPIO42/39/41/40, with an upper limit of 80 MHz.
- [ ] Measure and confirm that GPIO38 is active-high and verify the PWM polarity.
- [ ] Perform an I2C scan to confirm the touch address at 0x5D, and save the GPIO1/2 and 47/48 waveforms.
- [ ] Verify that the SD card mounts, reads, writes, and operates continuously at 80 MHz with the target card; if it fails, compare operation at a lower frequency.
- [ ] Confirm the amplifier GPIO21 polarity and the I2S 13/11/12 channel and sample format.
- [ ] Select either the PDM microphone or wireless SPI according to the product SKU; simultaneous use of GPIO9/10 is prohibited.
- [ ] Design the nRF24, SX1262, and Zigbee functions to be mutually exclusive for GPIO1/2 resources.
- [ ] Verify GPIO0/45/46 levels during cold boot, warm reset, and download mode.
- [ ] Test separate and abnormal combinations of USB-C, 5 V, and battery power; do not use unevaluated parallel inputs.
- [ ] Add failure handling and explicit logs for all `begin()` calls, memory allocations, device IDs, and filesystem mounts.
- [ ] Save the test firmware version, board number, prototype number, test date, and test results.

## 10. Recommended Minimum Board-Level Self-Test

Mass-production or service firmware should output the following results in sequence:

```text
[PASS] PSRAM allocation 2 x 153600 bytes
[PASS] LCD ST7789 SPI2 80MHz / backlight GPIO38
[PASS] Touch GT911 I2C 0x5D
[PASS] SD SPI 4/5/6/7, capacity=...
[PASS] Audio I2S 13/11/12, amp GPIO21
[SKIP] PDM microphone (wireless path selected)
[PASS] External radio ..., GPIO45=LOW
[PASS] USB/UART log 115200
```

The self-test must distinguish among `PASS`, `FAIL`, and `SKIP`. Mutually exclusive peripherals that were not tested must be marked `SKIP`, not `PASS`.

## 11. Maintenance Conclusion

The board’s fixed core drivers are: ESP32-S3 + ST7789 SPI LCD + GPIO38 backlight + GPIO15/16 touch I2C + GPIO4–7 MicroSD + GPIO11–13/21 I2S audio. The wireless expansion and PDM microphone form a switchable resource domain, while GPIO1/2, 9/10, and 45/46 are the primary risk points for maintenance and feature integration.

The only high-risk item that cannot currently be fully resolved using repository information alone is the touch reset/interrupt connection: the schematic specifies GPIO47/48, while the validated code uses GPIO1/2 to perform the address-selection sequence. Future hardware maintenance should prioritize physical waveform and continuity verification for this item. Until it is confirmed, retain the code-first policy documented here and do not arbitrarily change the driver back to the schematic pins.