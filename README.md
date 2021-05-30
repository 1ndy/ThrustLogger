# Thrust Logger
This tool is designed to read load measurements from a Sparkfun 
[HX711](https://www.sparkfun.com/products/13879) through an arduino via serial.
It displays information about the data collected in real time and saves ASCII
numbers to a user-selected log file.

I have plans to add analysis capability once recording is finished. I use this
program to record high frequency load measurements from a rocket motor test stand 
and want to calculate impulse and speicific impulse from the data.

This program uses the Windows API to interface with COM ports and thus is not
portable. The only dependcy at the moment is Qt5.

