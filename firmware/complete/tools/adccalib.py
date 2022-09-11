# Generation of lookup tables for the temperature sensors
# Copyright (c) 2022, Ralf Gerlich
import pandas as pd
import numpy as np
import scipy
from scipy.interpolate import interp1d


def to_c_array(values, formatter=str, colcount=15):
    # apply formatting to each element
    values = [formatter(v) for v in values]

    # split into rows with up to `colcount` elements per row
    rows = [values[i:i+colcount] for i in range(0, len(values), colcount)]

    # separate elements with commas, separate rows with newlines
    body = ',\n'.join([', '.join(r) for r in rows])

    # assemble components into the complete string
    return '{%s}' % body


# Temperature sensor
# Location of datasheet data
datasheet_data_src = '../../../datasheets/PTC_Data.ods'
# Selected sensor
sensor = 'KTY81-121'
# Selected column
column = 'Rtyp'

# Voltage and current
calibration_data_src = '../../../datasheets/calibration_data.ods'

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
temp_data_raw = pd.read_excel(datasheet_data_src, sheet_name=sensor)

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
voltageSetpointData = pd.read_excel(calibration_data_src, sheet_name="VoltageSetPoint")
currentSetpointData = pd.read_excel(calibration_data_src, sheet_name="CurrentSetPoint")
voltageMeasurementData = pd.read_excel(calibration_data_src, sheet_name="VoltageMeasurement")
currentMeasurementData = pd.read_excel(calibration_data_src, sheet_name="CurrentMeasurement")

# Measurement calibration tables
# ADC Values to consider
adc_values = np.linspace(start=0, stop=adc_max+1, endpoint=True, num=adcSteps+1)

# Sort measurements by rawADC value
voltageMeasurementData.sort_values(by='rawADC', inplace=True)
currentMeasurementData.sort_values(by='rawADC', inplace=True)

# Interpolate along the ADC values to consider
adc2Voltage = interp1d(voltageMeasurementData.rawADC, voltageMeasurementData.Vmeas, fill_value="extrapolate", kind="quadratic")
adc2Current = interp1d(currentMeasurementData.rawADC, currentMeasurementData.Imeas, fill_value="extrapolate", kind="quadratic")
measuredVoltageOffsets = np.round(1000.0*adc2Voltage(adc_values)).astype(int)
measuredCurrentOffsets = np.round(1000.0*adc2Current(adc_values)).astype(int)

measuredVoltageGradient = np.diff(measuredVoltageOffsets)
measuredCurrentGradient = np.diff(measuredCurrentOffsets)

measuredVoltageShift = np.ceil(np.log2((adc_max+1)/adcSteps)).astype(int)
measuredCurrentShift = measuredVoltageShift

# Setpoint calibration tables
dacSteps = 32

# Voltage calibration table
print(voltageSetpointData)
# Minimum maximum voltage to consider
maxVoltageMillivolts = 30.E3
# Bits required to express maximum voltage
voltageDACBits = np.ceil(np.log2(maxVoltageMillivolts)).astype(int)
# Actual maximum value that can be expressed (+1)
maxVoltageMilliVoltsActual = 1<<voltageDACBits
# Voltage values to be considered for interpolate
voltageValues = np.linspace(start=0, stop=maxVoltageMilliVoltsActual, endpoint=True, num=dacSteps+1)
# Interpolateion function
voltage2DAC = interp1d(voltageSetpointData.Vmeas*1.E3, voltageSetpointData.rawDAC, fill_value="extrapolate", kind="quadratic")
# Voltage offsets
setpointVoltageOffsets = np.clip(a=np.round(voltage2DAC(voltageValues)), a_min=0, a_max=16383).astype(int)
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
# Interpolateion function
current2DAC = interp1d(currentSetpointData.Imeas*1.E3, currentSetpointData.rawDAC, fill_value="extrapolate", kind="quadratic")
# Current offsets
setpointCurrentOffsets = np.clip(a=np.round(current2DAC(currentValues)), a_min=0, a_max=16383).astype(int)
setpointCurrentGradients = np.diff(setpointCurrentOffsets)
# FIXME: We might want to calculate dacSteps from the bit width instead...
setpointCurrentShift = currentDACBits - np.ceil(np.log2(dacSteps)).astype(int)

print("#define PS9530_ADC_VOLTAGE_SHIFT %s" % measuredVoltageShift)
print("#define PS9530_ADC_CURRENT_SHIFT %s" % measuredCurrentShift)
print("#define PS9530_DAC_VOLTAGE_SHIFT %s" % setpointVoltageShift)
print("#define PS9530_DAC_CURRENT_SHIFT %s" % setpointCurrentShift)
print("const uint16_t PS9530_Ctrl::adcVoltageOffset[%d] PROGMEM = \n%s;" % (adcSteps, to_c_array(measuredVoltageOffsets[:-1], colcount=10)))
print("const uint16_t PS9530_Ctrl::adcVoltageGradient[%d] PROGMEM = \n%s;" % (adcSteps, to_c_array(measuredVoltageGradient, colcount=10)))

print("const uint16_t PS9530_Ctrl::adcCurrentOffset[%d] PROGMEM = \n%s;" % (adcSteps, to_c_array(measuredCurrentOffsets[:-1], colcount=10)))
print("const uint16_t PS9530_Ctrl::adcCurrentGradient[%d] PROGMEM = \n%s;" % (adcSteps, to_c_array(measuredCurrentGradient, colcount=10)))

print("const uint16_t PS9530_Ctrl::voltageDACOffset[%d] PROGMEM = \n%s;" % (dacSteps, to_c_array(setpointVoltageOffsets[:-1], colcount=10)))
print("const uint16_t PS9530_Ctrl::voltageDACGradient[%d] PROGMEM = \n%s;" % (dacSteps, to_c_array(setpointVoltageGradients, colcount=10)))

print("const uint16_t PS9530_Ctrl::currentDACOffset[%d] PROGMEM = \n%s;" % (dacSteps, to_c_array(setpointCurrentOffsets[:-1], colcount=10)))
print("const uint16_t PS9530_Ctrl::currentDACGradient[%d] PROGMEM = \n%s;" % (dacSteps, to_c_array(setpointCurrentGradients, colcount=10)))

print("const uint16_t PS9530_Ctrl::minTempADC[2] PROGMEM = %s;" % to_c_array(minTempADC))
print("const uint16_t PS9530_Ctrl::maxTempADC[2] PROGMEM = %s;" % to_c_array(maxTempADC))
print("const uint8_t PS9530_Ctrl::shiftTempADC[2] PROGMEM = %s;" % to_c_array(actTempStepShift))
print("const int16_t PS9530_Ctrl::tempOffset[2][%d] PROGMEM = {" % (maxTempSteps-1));
print("    %s," % to_c_array(temp1Table[:-1].astype(int)))
print("    %s," % to_c_array(temp2Table[:-1].astype(int)))
print("};");
print("const int16_t PS9530_Ctrl::tempGradient[2][%d] PROGMEM = {" % (maxTempSteps-1));
print("    %s," % to_c_array(temp1Gradient.astype(int)))
print("    %s," % to_c_array(temp2Gradient.astype(int)))
print("};");
