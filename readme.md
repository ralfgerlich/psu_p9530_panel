[Toolbox Bodensee](https://toolbox-bodensee.de) presents: The home-made UI board replacement for the ELV PS9530 Power Supply Unit.

Because the microcontroller on our PS9530 unit broke and no replacement parts were available, we created
a replacement for the user interface based on an Arduino Uno R3 and a Seedstudio TFT Shield.
Most of the original circuit for the front-end was kept intact and connected to the Arduino.
The time-based ADC implemented by ELV on the board was replaced by the 10-bit ADC in the AVR microcontroller.

Of course, with a pixel-based display available, we had to add a graph-display for voltage, current and power.

The front-end shall
- measure the actual voltage and current via the UMess and IMess lines from the main board,
- measure the current temperatures via the TEMP_1 (heatsink) and TEMP_2 (coil) lines from the main board,
- provide the setpoints/limits for voltage and current via the USoll and ISoll lines to the main board,
- set both setpoints to zero in case of standby,
- initiate standby automatically in case of over-temperature on the TEMP_1 or TEMP_2 lines,
- display measurement, setpoint and status information on the display, and
- allow changing of setpoints via the keyboard and the encoder.

# Usage
Turn on the power supply using the power button on the lower left and admire the wonderful Toolbox emblem for a few seconds.

DANGER: The output lines my supply spurious voltage for a few seconds while the Arduino boots up. This probably can only be solved using a modified bootloader.

The power supply provides current and power limiting.
The current is limited is to the maximum value that will be at most the current limit and will result in a power (P=V*I) that is at most the given power limit.

## Standard Display Mode
After reset, the display will be in standard mode, displaying the voltage, current and power setpoints (small font) and measurements (large font), as well as the status ("Standby", "Overtemp" and "Limited").

The power supply will start up and set standby mode.
In standby mode, the power supply will set the voltage and the current to "0".
To toggle standby mode, press the "Standby" button.

To change any of the numeric settings, short press either "U" (voltage), "I" (current) or "P" (power) for at most 500ms.

The currently selected digit will be highlighted in green.
To change the cursor position, use the arrow keys above the rotary knob.

To change the value, use either the number keys or the rotary knob.
Turning the knob counter-clockwise will reduce the number displayed, turning it clock-wise will increase it.

The amount of decrease/increase will depend on the currently selected cursor position.
If the cursor is on the "ones" position, turning the knob by one step will change the value by 1V/A/W.
If the cursor is in the "tenths" position, turning the know will change the value by 0.1V/A/W, and so on.

Pressing a digit key will change the currently selected position to that digit value and advance the cursor to the right.
If the cursor is already in the last position, it will stay there.
Pressing "CE" will reset the currently selected value to "0" and set the selected position to the fist digit.

Values outside the valid ranges will be limited to the valid range:
- 0 to 30V for the voltage,
- 0 to 10A for the current, and
- 0 to 300W for the power.

In order for the new value to become active, "Enter" must be pressed.

## Graph Display Mode
The replacement/enhancement also provides a graph display mode.
A long press of the "U" button (600ms-2s press duration) cycles through standard display mode, voltage graph display mode and voltage/current display mode. A long press of "I"/"P" will cycle similarly but the first graph would be a amps/power graph instead.

In the graph display modes, either only one out of the measured voltage/amps/power or both the measured voltage and current are shown. Numerically at the top of the display and graphically at the bottom of the display.
In combined mode, the voltage graph is green and the current graph is shown in yellow.

To change either of the voltage or current setpoints, press the "U" or "I" buttons for at most 500ms.
Note that setting the current setpoint is not supported in voltage graph mode.
The respective number will change to white with the cursor position displayed in green - as in standard display mode.

The actual value can be set either by the number keys or the rotary knob and must be confirmed by the "Enter" key, just as in standard display mode.

# Repairs to the main PCB
One of the power Transistors was fried and another one was not soldered in properly so we replaced all of them with new ones.

The three inline diodes at the control circit got also replaced since one or two where damaged as well.

Some (most) of the smaller capacitors got replaced since they were off and affected the controls negatively.

# Design changes
(For the original schematics, refer to the PS9530 manual, which can be found [here](https://www.mikrocontroller.net/attachment/70639/PS9530_KM_G_071030.pdf), for example).

The schematic for our version can be found in psu_p9530_panel.pdf or by opening the KiCAD-project in the project root.
The parts kept from the original are numbered in the range 300-399, parts introduced in the modified design are numbered in the range 400-499.

We simply cut off the part containing the display and the microcontroller to make space for the new display.

From the original design we kept
- the keyboard matrix (TA1-22),
- the rotary encoder (S300) with debouncing (C325/C326),
- the digital-to-analog-converter/DAC (IC306) with multiplexer (IC308) and supporting circuitry,
- the sample-and-hold circuitry (IC310A/B) for voltage and current setpoints,
- the voltage reference (D305),
- the measurement amplifier (IC312A with supporting circuitry), and
- the power-LED (D306).

We added/replaced
- the microcontroller (A401),
- the display (A402), and
- the shift-register for scanning the keyoard matrix (U401).

# Theory of Operation
The main PCB of the PSU does the heavy lifting, regulating the voltage and current according to the setpoints defined by the USoll and ISoll lines.
Voltage and current are limited to the values given by the setpoints (adjusted by a scaling factor), with the lower limit controlling the voltage on the output lines.

## Voltage Reference
For both Analog-to-Digital- and Digital-to-Analog-Conversion, a prevision 2.5V voltage reference (D305) is used.
The current for the voltage reference is supplied via the series resistor R337.

In the original design, this is a 27kOhm resistor, providing about 100uA to the reference from the 5V supply line, with 2.5V dropping off at the voltage reference.
This was replaced by an 8.2kOhm resistor, providing about 300uA.

Of this, 100uA is actually used by the reference itself.
In addition, about 80uA are used by the digital-to-analog-converter (IC308), and an additional 110uA are used by the ADC in the AVR.

NOTE: The replacement is necessary, as otherwise the voltage of the voltage reference will drop below 2.5V due to insufficient current supply.

## Voltage and Current Measurement
The voltage is measured via a voltage divider on the output, connected to the UMess line.
Current is measured via a shunt resistor on the output line, connected to the IMess_Ein line.

The voltage range for the UMess line is 0 to 2.5V.
The voltage range for the IMess_Ein line is 0 to 0.5V.
After the low-pass-filter (R350/C332), the current measurement voltage is amplified by a factor of about 4.9 by IC312A and R331/R332, resulting in a range of about 0 to 2.5V, which is passed on via the IMess line.

Both of these are passed on to the ADC channels A0 and A1, respectively, of the AVR microcontroller on the Arduino board.
The AVR ADC only provides a resolution of 10bit, but using oversampling, this is extended to a resolution of 13 bits.
For details on the theory and implementation, see [Microchip Application Note 8003](https://www.microchip.com/en-us/application-notes/an8003).

To determine the actual voltage and current values in Millivolt and Milliampere, respectively, calibration tables defined in ps9530_ctrl.cpp are used.
See "Interpolation" for more details on the interpolation procedure.

## Temperature Measurement
The temperatures of the coil and the heatsink are measured using [KTY81-121](https://www.nxp.com/docs/en/data-sheet/KTY81_SER.pdf) positive temperature coefficient of resistance sensors.
The voltages are in the range between 0 and 2.5V and are fed directly into the AVR ADC channels A2 and A3, respectively.

To determine the actual temperatures, calibration tables defined in ps9530_ctrl.cpp are used.
See "Interpolation" for more details on the interpolation procedure.
The calibration tables were determined from the resistances of the sensors according to the datasheet.

Neither temperature is displayed.
However, if any of the sensors is in overtemperature mode, current and voltage setpoints will be internally set to zero and the overtemperature warning will be displayed.

Overtemperature mode for the heatsink temperature will be entered if the temperature in degrees Celsius exceeds the limit given by TEMP1_UPPER_LIMIT (default: 100C).
Overtemperature mode will be left only once the temperature has fallen below TEMP1_LOWER_LIMIT (default: 80C).

Similarly, overtemperature mode for the coil temperature will be entered if the temperatured in degrees Celsius exceeds the limit given by TEMP2_UPPER_LIMIT (default: 60C), and will be left once the temperature has fallen below TEMP2_LOWER_LIMIT (default: 50C).

The limits are defined in ps9530_ctrl.cpp and can be changed.

## Voltage and Current Setpoints
Voltage and current setpoints are supplied to the main PCB via the USoll and ISoll lines, respectively.
These voltages on these lines are in the range from 0 to 2.5V.

Digital-to-analog conversion is handled by the DAC circuitry (IC308) together with the DAC sample-and-hold circuit (IC310 together with R336/C306 for the voltage and R338/C307 for the current).

The DAC is provided a digital value via the SPI bus (SCK/MOSI/~DAC_CS) and will then provide the analog value on is V-OUT line.
For details on the protocol, view the [datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/1658f.pdf).

The analog value is passed on to the analog multiplexer, which will switch it through either to the USoll_raw or the ISoll_raw line, depending on the level on the MUXSEL line.
A low level will pass the voltage to the USoll_raw line, and a high level will pass it to the ISoll_raw line.
The respective other line will be high-impedance.

Depending on the output line selected, either C306 or C307 will be charged via R336 or R338, respectively.
The impedance ... of IC310A and IC310B will sample the voltage on C306/C307 with high-impedance and forward them to USoll and ISoll, respectively, providing the setpoints to the main PCB.

## Power Setpoint and Measurement
The hardware does not provide a direct means for setting the power limit or measuring the current power.
Instead, the power limit is converted into a current limit by dividing it by the voltage setpoint, and the measured current is converted into a measured power by multiplying it with the measured voltage.

The current limit communicated to the main PCB is the minimum of the direct current limit and the current limit derived from the power limit.
The "Limited" flag will be displayed at either the current or the power setpoint, depending on which one is the one currently determining the actual current limit setpoint.

## Interpolation
Interpolation is done using fixed-point interpolation tables.
In general, the input value to be converted is split into a fractional part f consisting of the lower n bits, and an integer part k consisting of the remaining upper bits.
The integer part is used as an index into the calibration table.

The output value is determined by linearly interpolating between the values at indices k and k+1 in the calibration table, with a value of f=0 mapped to the value at index k and the maximum value for f mapped to the value at index k+1.

Both for measurement and setpoints of voltage and current, high non-linearity was observed in the lower portion of the respective range.
For that reason, there are tables for fine and coarse interpolation.
Fine interpolation is applied for the range between the first and the second index of the coarse interpolation table.

For more details, see PS9530_Ctrl::interpolateADCVoltage, PS9530_Ctrl::interpolateADCCurrent, PS9530_Ctrl::interpolateDACVoltage, PS9530_Ctrl::interpolateDACCurrent and PS9530_Ctrl::interpolateADCTemp as well as the interpolate template in ps9530_ctrl.cpp.

## Keyboard Scan
The keyboard matrix consists of 3 column and 8 row lines.
These are scanned using a 54LS164 shift register connected to the SPI bus.
The shift register is used to shift a single zero surrounded by ones through the row lines, pulling exactly one of the rows to low at a time.
The keys provide a connection between the row and the column lines, so that the keys pressed in the respective row can be seen from the column lines being pulled to low.

The keyboard scan is performed in kbd_scan_matrix (kbd.cpp), which in turn is called by kbd_update (same file).
The keyboard scan is initiated by PS9530_Ctrl::update at a frequency of about 100Hz.

At each period, two complete scans are performed for debouncing.
If the result of the first scan gives different results than the second scan, neiher of the results is used.

In our case, we have used a ribbon cable without considering cross-talk between the lines.
This led to spurious key-presses and signals from the rotary encoder.
To reduce cross-talk, the capacitors C402 to C409 on the column lines were added.

## Rotary Encoder
The rotary encoder provides two outputs, which are pulled to GND on contact.
Debouncing is achieved via a low-pass through C325 and C326.

In idle state, both outputs are open, and pulled up via the pull-up in the AVR.
When turned counter-clockwise, the signals BA cycle through 01, 00 and 10.
For clockwise rotation, the cycle is reversed.

The level change trigger pin interrupts are INT0 and INT1 in the AVR.

Since very fast or very slow rotation was somethings giving wrong results we expect a more then 50% correct cycle to count for a direction.

## Special SPI handling
The lines for the SPI bus are shared with the display.
As both the keyboard scan and the update of the DAC also use the SPI bus and the Adafruit display library is not interrupt-safe, special handling is necessary:
Before using the SPI bus inside an interrupt, a delay ensures that the current SPI transmission is finished before deactivating the CS pin for the display and storing the current configuration of the SPI bus.
The status of the TFT CS pin and the SPI bus configuration is restored before leaving the interrupt handler.
