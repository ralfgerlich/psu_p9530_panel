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


# Calibration of temperature table
# Maximum number of entries in temperature table
max_temp_entries = 16

# Read raw data
temp_data_raw = pd.read_csv(datasheet_data_src)

# Extract temperature and resistance
temp_data = temp_data_raw[['Temp_C', column]]

# Calculate values for Temp1
rTot = R82 + temp_data[column]
iTot = -5/rTot
vR82 = iTot * R82
vTemp1 = -vR82 * R56 / R54
temp_data['vTemp1'] = vTemp1
adcTemp1 = np.round(vTemp1 / vref * adc_max).astype(int)
temp_data['adcTemp1'] = adcTemp1

# Calculate values for Temp2
rTot = R63 + temp_data[column]
iTot = 5 / rTot
vTemp2 = iTot * R63
temp_data['vTemp2'] = vTemp2
adcTemp2 = np.round(vTemp2 / vref * adc_max).astype(int)
temp_data['adcTemp2'] = adcTemp2

dataTemp1 = temp_data.sort_values(by='adcTemp1')
dataTemp2 = temp_data.sort_values(by='adcTemp2')

# Determine minimum and maxmimum values
minTempADC = np.min((adcTemp1, adcTemp2), axis=1)
maxTempADC = np.max((adcTemp1, adcTemp2), axis=1)

# Determine minimum step value
minTempStep = (maxTempADC-minTempADC)/max_temp_entries
actTempStepShift = np.ceil(np.log2(minTempStep)).astype(int)
actTempSteps = np.ceil((maxTempADC-minTempADC)/(1<<actTempStepShift)).astype(int)
maxTempSteps = np.max(actTempSteps)

# Determine ADC values
minTempADCTable = minTempADC
maxTempADCTable = minTempADC + (maxTempSteps<<actTempStepShift)
tempADCTable = np.linspace(minTempADCTable, maxTempADCTable, maxTempSteps, endpoint=False)

# Interpolate associated temperatures
temp1Table = np.round(np.interp(tempADCTable[:, 0], dataTemp1['adcTemp1'], dataTemp1['Temp_C']))
temp2Table = np.round(np.interp(tempADCTable[:, 1], dataTemp2['adcTemp2'], dataTemp2['Temp_C']))

# Determine gradient table
temp1Gradient = np.diff(temp1Table)
temp2Gradient = np.diff(temp2Table)

# Data for the voltage and current setpoints/measurements
voltageCalibrationData = pd.read_csv(os.path.join(datasheets_path, 'voltageCalibrationData.csv'), decimal=',')
currentCalibrationData = pd.read_csv(os.path.join(datasheets_path, 'currentCalibrationData.csv'), decimal=',')

# Interpolation functions
adc2voltage = interp1d(voltageCalibrationData.ADC, voltageCalibrationData.U, fill_value="extrapolate", kind="quadratic")
voltage2dac = interp1d(voltageCalibrationData.U, voltageCalibrationData.DAC, fill_value="extrapolate", kind="quadratic")
adc2current = interp1d(currentCalibrationData.ADC, currentCalibrationData.I, fill_value="extrapolate", kind="quadratic")
current2dac = interp1d(currentCalibrationData.I, currentCalibrationData.DAC, fill_value="extrapolate", kind="quadratic")

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
setpointVoltageOffsets = np.clip(a=np.round(voltage2dac(voltageValues)), a_min=0, a_max=16383).astype(int)
setpointVoltageGradients = np.diff(setpointVoltageOffsets)
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
setpointCurrentOffsets = np.clip(a=np.round(current2dac(currentValues)), a_min=0, a_max=16383).astype(int)
setpointCurrentGradients = np.diff(setpointCurrentOffsets)
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
