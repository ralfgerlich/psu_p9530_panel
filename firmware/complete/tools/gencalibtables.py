# Generation of calibration tables for ADC and DAC
# Copyright (c) 2022, Ralf Gerlich
import pandas as pd
import numpy as np
from scipy.interpolate import interp1d
import os.path


def to_c_array(values, formatter=str, colcount=15):
    # apply formatting to each element
    values = [formatter(v) for v in values]

    # split into rows with up to `colcount` elements per row
    rows = [values[i:i+colcount] for i in range(0, len(values), colcount)]

    # separate elements with commas, separate rows with newlines
    body = ',\n'.join([', '.join(r) for r in rows])

    # assemble components into the complete string
    return '{%s}' % body

datasheets_path = '../../../datasheets'

# Temperature sensor
# Selected sensor
sensor = 'KTY81-121'
# Location of datasheet data
datasheet_data_src = os.path.join(datasheets_path,sensor+".csv")
# Selected column
column = 'Rtyp'

# Resistor Values [Ohm] for Temp1 & Temp2
R54 = 24.E3
R56 = 12.E3
R63 = 680
R82 = 2.55e3

# Reference voltage on ADC [V]
vref = 2.5

# Maximum ADC value
adc_max = 1023

# Number of steps to generate for voltage and current ADC
adcSteps = 32

# Maximum number of steps to generate for temperature tables
max_temp_entries = 16

# Temperature resistance data
temp_data = pd.read_csv(datasheet_data_src)

# Calculate voltages for temperature sensors
voltageTemp1 = 5*R82/(R82 + temp_data[column]) * R56 / R54
voltageTemp2 = 5*R63/(R63 + temp_data[column])

# Calculate ADC values for temperature sensors
adcTemp1 = voltageTemp1 / vref * adc_max
adcTemp2 = voltageTemp2 / vref * adc_max

# Interpolate ADC values to temperatures
adc2temp1 = interp1d(adcTemp1, temp_data['Temp_C'], fill_value='extrapolate', kind='quadratic')
adc2temp2 = interp1d(adcTemp2, temp_data['Temp_C'], fill_value='extrapolate', kind='quadratic')

# Determine minimum and maxmimum values
minTempADC = np.floor(np.min((adcTemp1, adcTemp2), axis=1)).astype(int)
maxTempADC = np.ceil(np.max((adcTemp1, adcTemp2), axis=1)).astype(int)

# Determine minimum step value
minTempStep = (maxTempADC-minTempADC)/max_temp_entries
actTempStepShift = np.ceil(np.log2(minTempStep)).astype(int)
actTempSteps = np.ceil((maxTempADC-minTempADC)/(1<<actTempStepShift)).astype(int)
maxTempSteps = np.max(actTempSteps)

# Determine ADC values to consider
minTempADCTable = minTempADC
maxTempADCTable = minTempADC + (maxTempSteps<<actTempStepShift)
tempADCTable = np.linspace(minTempADCTable, maxTempADCTable, maxTempSteps, endpoint=False)

# Interpolate associated temperatures
temp1Table = adc2temp1(tempADCTable[:, 0])
temp2Table = adc2temp2(tempADCTable[:, 1])

# Data for the voltage and current setpoints/measurements
voltageCalibrationData = pd.read_csv(os.path.join(datasheets_path, 'voltageCalibrationData.csv'), decimal=',')
# currentCalibrationData = pd.read_csv(os.path.join(datasheets_path, 'currentCalibrationData.csv'), decimal=',')
currentMeasurementData = pd.read_csv(os.path.join(datasheets_path, 'currentMeasurementData.csv'), decimal=',')
currentSetpointData = pd.read_csv(os.path.join(datasheets_path, 'currentSetpointData.csv'), decimal=',')

# Interpolation functions
adc2voltage = interp1d(voltageCalibrationData.ADC, voltageCalibrationData.U, fill_value="extrapolate", kind="linear")
voltage2dac = interp1d(voltageCalibrationData.U, voltageCalibrationData.DAC, fill_value="extrapolate", kind="linear")
adc2current = interp1d(currentMeasurementData.ADC, currentMeasurementData.I, fill_value="extrapolate", kind="linear")
current2dac = interp1d(currentSetpointData.I, currentSetpointData.DAC, fill_value="extrapolate", kind="linear")

# Measurement calibration tables
# ADC Values to consider
adc_values = np.linspace(start=0, stop=adc_max+1, endpoint=True, num=adcSteps+1)
# Voltages [mV] and currents [mA] for the ADC values to consider
measuredVoltageOffsets = np.round(1000.0*adc2voltage(adc_values)).astype(int)
measuredCurrentOffsets = np.round(1000.0*adc2current(adc_values)).astype(int)

measuredVoltageShift = np.ceil(np.log2((adc_max+1)/adcSteps)).astype(int)
measuredCurrentShift = measuredVoltageShift

# Setpoint calibration tables
dacSteps = 32

# Voltage calibration table
# Minimum maximum voltage to consider
maxVoltageMillivolts = 30.E3
# Bits required to express maximum voltage
voltageDACBits = np.ceil(np.log2(maxVoltageMillivolts)).astype(int)
# Actual maximum value that can be expressed (+1)
maxVoltageMilliVoltsActual = 1<<voltageDACBits
# Voltage values to be considered for interpolate
voltageValues = np.linspace(start=0, stop=maxVoltageMilliVoltsActual, endpoint=True, num=dacSteps+1)
# Voltage offsets
setpointVoltageOffsets = np.clip(a=np.round(voltage2dac(voltageValues/1.E3)), a_min=0, a_max=16383).astype(int)
# FIXME: We might want to calculate dacSteps from the bit width instead...
setpointVoltageShift = voltageDACBits - np.ceil(np.log2(dacSteps)).astype(int)

# Current calibration table
# Minimum maximum current to consider
maxCurrentMilliAmps = 10.E3
# Bits required to express maximum current
currentDACBits = np.ceil(np.log2(maxCurrentMilliAmps)).astype(int)
# Actual maximum value that can be expressed (+1)
maxCurrentMilliAmpsActual = 1<<currentDACBits
# Current values to be considered for interpolate
currentValues = np.linspace(start=0, stop=maxCurrentMilliAmpsActual, endpoint=True, num=dacSteps+1)
# Current offsets
setpointCurrentOffsets = np.clip(a=np.round(current2dac(currentValues/1.E3)), a_min=0, a_max=16383).astype(int)
# FIXME: We might want to calculate dacSteps from the bit width instead...
setpointCurrentShift = currentDACBits - np.ceil(np.log2(dacSteps)).astype(int)

print("#define PS9530_ADC_VOLTAGE_SHIFT %s" % measuredVoltageShift)
print("#define PS9530_ADC_CURRENT_SHIFT %s" % measuredCurrentShift)
print("#define PS9530_DAC_VOLTAGE_SHIFT %s" % setpointVoltageShift)
print("#define PS9530_DAC_CURRENT_SHIFT %s" % setpointCurrentShift)
print("#define PS9530_ADC_TEMP1_SHIFT %d" % actTempStepShift[0])
print("#define PS9530_ADC_TEMP2_SHIFT %d" % actTempStepShift[1])
print("const uint16_t PS9530_Ctrl::adcVoltageOffset[%d] PROGMEM = \n%s;" % (adcSteps+1, to_c_array(measuredVoltageOffsets, colcount=10)))

print("const uint16_t PS9530_Ctrl::adcCurrentOffset[%d] PROGMEM = \n%s;" % (adcSteps+1, to_c_array(measuredCurrentOffsets, colcount=10)))

print("const uint16_t PS9530_Ctrl::voltageDACOffset[%d] PROGMEM = \n%s;" % (dacSteps+1, to_c_array(setpointVoltageOffsets, colcount=10)))

print("const uint16_t PS9530_Ctrl::currentDACOffset[%d] PROGMEM = \n%s;" % (dacSteps+1, to_c_array(setpointCurrentOffsets, colcount=10)))

print("const uint16_t PS9530_Ctrl::minTempADC[2] PROGMEM = %s;" % to_c_array(minTempADC))
print("const uint16_t PS9530_Ctrl::maxTempADC[2] PROGMEM = %s;" % to_c_array(maxTempADC))
print("const int16_t PS9530_Ctrl::tempOffset[2][%d] PROGMEM = {" % maxTempSteps);
print("    %s," % to_c_array(temp1Table.astype(int)))
print("    %s," % to_c_array(temp2Table.astype(int)))
print("};");

# Generate random test points
numTestpoints = 10

# Generate random values in the ADC range
testADCValues = np.sort(np.random.randint(0, adc_max, 10))
testADCVoltages = np.round(interp1d(adc_values, measuredVoltageOffsets)(testADCValues)).astype(int)
testADCCurrents = np.round(interp1d(adc_values, measuredCurrentOffsets)(testADCValues)).astype(int)

voltageADCTests = [f"    TEST_ASSERT_UINT16_WITHIN(2, {voltage}, PS9530_Ctrl::interpolateADCVoltage({adc}));\n" for adc, voltage in zip(testADCValues, testADCVoltages)]
currentADCTests = [f"    TEST_ASSERT_UINT16_WITHIN(2, {current}, PS9530_Ctrl::interpolateADCCurrent({adc}));\n" for adc, current in zip(testADCValues, testADCCurrents)]
print("".join(voltageADCTests))
print()
print("".join(currentADCTests))
print()

# Generate random values in the voltage range
testDACVoltages = np.sort(np.random.randint(0, maxVoltageMillivolts, 10))
testDACVoltageValues = np.round(interp1d(voltageValues, setpointVoltageOffsets)(testDACVoltages)).astype(int)
testDACCurrents = np.sort(np.random.randint(0, maxCurrentMilliAmps, 10))
testDACCurrentValues = np.round(interp1d(currentValues, setpointCurrentOffsets)(testDACCurrents)).astype(int)

voltageDACTests = [f"    TEST_ASSERT_UINT16_WITHIN(2, {dac}, PS9530_Ctrl::interpolateDACVoltage({voltage}));\n" for dac, voltage in zip(testDACVoltageValues, testDACVoltages)]
currentDACTests = [f"    TEST_ASSERT_UINT16_WITHIN(2, {dac}, PS9530_Ctrl::interpolateDACCurrent({current}));\n" for dac, current in zip(testDACCurrentValues, testDACCurrents)]
print("".join(voltageDACTests))
print()
print("".join(currentDACTests))
print()
