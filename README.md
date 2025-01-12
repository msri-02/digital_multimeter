# Digital Multimeter
Digital multimeter from embedded and microcontroller applications (CPE 316)

The digital multimeter operates on the STM32L476G MCU and utilizes the built-in comparator, analog-to-digital converter (ADC), and universal asynchronous receiver / transmitter (UART). The digital multimeter is displayed using the VT100 Serial Terminal. There are two modes for the multimeter: AC and DC. The user can switch to each mode using the ‘a’ and ‘d’ keys on the laptop keyboard that the program is being run on. For AC mode, the user can see the signal’s frequency in Hz, Vrms, and peak-to-peak voltage. For DC mode, the user can see the DC voltage. There is a low-pass filter in this system, which is built on the breadboard.
