#!/bin/python
import pyvisa as visa
import time

rm = visa.ResourceManager()
smu = rm.open_resource("USB0::0xCAFE::0x4000::12345678::INSTR")

print(smu.query("*IDN?"))

while True:
    starttime = time.time() * 1000
    for i in range(100):
        voltage = float(smu.query("MEASure:VOLTage:DC?"))
    print("Time for 100 queries: %ims" % (time.time() * 1000 - starttime))
