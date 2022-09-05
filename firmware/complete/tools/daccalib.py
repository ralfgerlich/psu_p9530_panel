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

# Voltage Values
voltageValues = np.linspace(start=0, stop=32768, num=steps+1)
voltageDACOffset = np.round(voltageValues / 1.E3 / vref * 16384 / R62 * R64 / (R60 + R61) * R60).astype(int)
voltageDACGradient = np.diff(voltageDACOffset)
print(f"voltageDACShift={np.ceil(np.log2(32768/steps)).astype(int)}")
print(f"voltageDACOffset={voltageDACOffset[:-1]}")
print(f"voltageDACGradient={voltageDACGradient}")

# Current Values
currentValues = np.linspace(start=0, stop=16384, num=steps+1)
currentDACOffset = np.round(currentValues / 1.E3 * Rsense * (1+R331/R332) / vref * 16384).astype(int)
currentDACGradient = np.diff(currentDACOffset)
print(f"currentDACShift={np.ceil(np.log2(16384/steps)).astype(int)}")
print(f"currentDACOffset={currentDACOffset[:-1]}")
print(f"currentDACGradient={currentDACGradient}")
