# Generation of lookup tables for the temperature sensors
# Copyright (c) 2022, Ralf Gerlich
import pandas as pd
import numpy as np
# Location of datasheet data
datasheet_data = '../../../datasheets/PTC_Data.ods'
# Selected sensor
sensor = 'KTY81-121'
# Selected column
column = 'Rtyp'

# Resistances [Ohm]]
R54 = 24000
R56 = 11950
R82 = 2015
R63 = 492

# Reference voltage on ADC [V]
vref = 2.5

# Maximum ADC value
adc_max = 1023

# Maximum number of entries in table
max_entries = 16

# Read raw data
data_raw = pd.read_excel(datasheet_data, sheet_name=sensor)

# Extract temperature and resistance
data = data_raw[['Temp_C', column]]

# Calculate the ADC values for Temp1
vR82 = -5*R82/(data[column]+R82)
vTemp1 = -vR82 * R56 / R54;
adcTemp1 = (np.round(vTemp1 / vref * adc_max)).astype(int)
data['adcTemp1'] = adcTemp1

# Calculate the ADC values for Temp2
vR63 = 5*R63/(data[column]+R63)
adcTemp2 = (np.round(vR63 / vref * adc_max)).astype(int)
data['adcTemp2'] = adcTemp2

dataTemp1 = data.sort_values(by='adcTemp1')
dataTemp2 = data.sort_values(by='adcTemp2')

# Determine minimum and maxmimum values
minADC = np.min((adcTemp1, adcTemp2), axis=1)
maxADC = np.max((adcTemp1, adcTemp2), axis=1)

# Determine minimum step value
minStep = (maxADC-minADC)/max_entries
actStepShift = np.ceil(np.log2(minStep)).astype(int)
actSteps = np.ceil((maxADC-minADC)/(1<<actStepShift)).astype(int)
maxSteps = np.max(actSteps)

# Determine ADC values
minTable = minADC
maxTable = minADC + (maxSteps<<actStepShift)
adcTable = np.linspace(minTable, maxTable, maxSteps, endpoint=False)

# Interpolate associated temperatures
temp1Table = np.round(np.interp(adcTable[:, 0], dataTemp1['adcTemp1'], dataTemp1['Temp_C']))
temp2Table = np.round(np.interp(adcTable[:, 1], dataTemp2['adcTemp2'], dataTemp2['Temp_C']))

# Determine gradient table
temp1Gradient = np.diff(temp1Table)
temp2Gradient = np.diff(temp2Table)

print(f"minADCTemp1={minADC[0]}")
print(f"maxADCTemp1={maxADC[0]}")
print(f"shiftADCTemp1={actStepShift[0]}")
print(f"offsetADCTemp1={temp1Table.astype(int)}")
print(f"gradientADCTemp1={temp1Gradient.astype(int)}")
print(f"minADCTemp2={minADC[1]}")
print(f"maxADCTemp2={maxADC[1]}")
print(f"shiftADCTemp2={actStepShift[1].astype(int)}")
print(f"offsetADCTemp2={temp2Table.astype(int)}")
print(f"gradientADCTemp2={temp2Gradient.astype(int)}")
