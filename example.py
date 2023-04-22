from matplotlib import pyplot as plt
import pandas as pd

filename='C:/Users/User/Documents/לימודים/Experiments/Filtration Experiments/New Experiments/example.csv'
df = pd.read_csv(filename)
df['Date & Time'] = pd.to_datetime(df['Date & Time'], dayfirst=True)
df = df.set_index('Date & Time')
pressure = pd.DataFrame()
pressure['pressure'] = df[' Pressure (mBar)']
pressure['pressure_mean'] = pressure['pressure'].resample('0.5T').mean()
pressure['pressure_med'] = pressure['pressure_mean'].resample('6T').median()
pressure.pressure_med.dropna().plot()
pressure.pressure.plot(alpha=0.3)
plt.legend()
plt.ylim(0,150)
plt.show()