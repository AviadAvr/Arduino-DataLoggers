# DataLoggers_Arduino
Arduino data logging units for field measurements and lab filtration experiments

These are the codes i used for the Arduino data loggers used in my MSc thesis research. 
One data logger was deployed in the field, and used to collect information from a wastewater treatment system year-round: ORP (mV), Pressure (mBar), and Temperature (C). This Arduino data logger offers an inexpensive (~10$) alternative to commercial data loggers (+1000$). The code for the different sensors can, of course, be utilized for any type of analog/digital sensor as long as a specific library is available, or a calibration curve is made (welcome to seek advice regarding this).

The other data logger was used in my lab-scale membrane filtration experiments. 
This one not only gathered data throughout the experiment (pressure, and weight), but also activated an external pump (via relay) that would activate whenever certain parameters were met, to ensure the physical safety of the experiment.

Attached below are examples of data gathered from one of the filtration experiments. In this experiment, I wanted to test the effects of ozonation treatment on the reduction of membrane fouling during filtration of biologically-treated wastewater (i.e., achieving a stable pressure profile over the course of filtration).
The semi-transparent trend is the full data, including negative pressure values from the backwash cycles, while the full blue trend represents the moving median of that same data (example of calculation and plotting in example.py file, using example.csv).

![2-08-22 ozonated water (24 hours) full comparison](https://user-images.githubusercontent.com/124437932/217281316-d7eb3e44-8743-4ae6-83b8-e8e91bd451df.png)

Below, is a comparison of 4 different experiments (moving medians. 2 ozonated, 2 untreated).
A clear winner (i.e., stable trend), is the ozonated effluent, showing relatively stable performance over the course of 70 hours of filtration!



![Comparison](https://user-images.githubusercontent.com/124437932/217282043-3ab6e67a-c1f7-4b8c-86a7-b1327ab76783.png)
