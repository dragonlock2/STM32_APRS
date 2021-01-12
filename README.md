# STM32 APRS

APRS modulation and demodulation with a UI. Made for STM32L433.

https://matthewtran.dev/2021/01/stm32-aprs/

Potentially reusable modules (.c/.h) in Core/:
* BitFIFO/IntFIFO - FIFO queue implemented with a ring buffer
* SSD1306 - I2C driver for SSD1306 OLED w/ compact font
* Keyboard - ASCII keyboard using T9 button layout
* AFSK - High speed AFSK modulation and demodulation to and from bit streams
* APRS - High speed APRS packet generation and decoding to and from bit streams
* APRSGUI - TUI implementation for displaying and configuring APRS packets

## How to Use

### iPython

ipython/aprs.ipynb is a Python implementation of the APRS encoding and decoding algorithms written for easy conversion to C.

### KiCad

The PCB design files for the hardware are located in kicad/.

### STM32CubeIDE

APRSTest.ioc is a STM32CubeMX configuration file and was used to generate the project starter code in STM32CubeIDE. Everything in Core/ should be copied over to the generated Core/ folder, overwriting as necessary.