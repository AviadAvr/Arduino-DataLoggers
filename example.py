from matplotlib import pyplot as plt
import pandas as pd

filename='path/example.csv'
df = pd.read_csv(filename)
df['Date & Time'] = pd.to_datetime(df['Date & Time'], dayfirst=True)
df = df.set_index('Date & Time')
pressure = pd.DataFrame()
pressure['pressure'] = df[' Pressure (mBar)']
pressure['pressure_mean'] = pressure['pressure'].resample('0.5T').mean() # measurements are taken every ~seconds, the resampling takes the mean of every 30 seconds (~6 data points) to average out measurement issues.
pressure['pressure_med'] = pressure['pressure_mean'].resample('6T').median() # further smoothing using median is important to remove negative values. i found that downsampling to every 6-8 minutes gave the smoothest trends, while staying true to the original data's form.
pressure.pressure_med.dropna().plot()
pressure.pressure.plot(alpha=0.3) # Just to compare the smoothed trend to the original data.
plt.legend()
plt.ylim(0,500) # Change the y axis limit accordingly
plt.show()
