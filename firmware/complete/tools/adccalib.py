# Calculation of basic calibration for ADC conversion
# Copyright (c) 2022, Ralf Gerlich
import numpy as np

# Resistor Values [Ohm]
Rsense = 0.3 / 6
R60 = 2.7E3
R61 = 33.E3
R62 = 100.E3
R64 = 100.E3
R331 = 22.E3
R332 = 5.6E3

# Reference voltage [V]
vref = 2.5

# Number of steps to generate
steps = 32

adc_values = np.linspace(start=0, stop=1024, num=steps+1)

# Voltage calibration
voltageOffset = np.round(1.E3 * adc_values * vref / 1023 * R62 / R64 * (R60 + R61) / R60).astype(int)
voltageGradient = np.diff(voltageOffset)
print(f"voltageShift={np.ceil(np.log2(1024/steps)).astype(int)}")
print(f"voltageOffset={voltageOffset[:-1]}")
print(f"voltageGradient={voltageGradient}")

# Current calibration
currentOffset = np.round(adc_values * vref / 1023 / (1+R331/R332) / Rsense * 1.E3).astype(int)
currentGradient = np.diff(currentOffset)
print(f"currentShift={np.ceil(np.log2(1024/steps)).astype(int)}")
print(f"currentOffset={currentOffset[:-1]}")
print(f"currentGradient={currentGradient}")
