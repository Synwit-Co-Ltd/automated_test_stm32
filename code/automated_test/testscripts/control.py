import pyvisa as visa

rm = visa.ResourceManager()
print(rm.list_resources())

smu = rm.open_resource("ASRL3::INSTR")
# smu = rm.open_resource("USB0::0xCAFE::0x4000::12345678::INSTR")

print(smu.query("*IDN?"))
print(smu.query("*ESE?"))
print(smu.query("*ESR?"))
print(smu.query("*OPC?"))
print(smu.query("*SRE?"))
print(smu.query("*STB?"))
print(smu.query("*TST?"))
print(smu.query("SYSTem:ERRor[:NEXT]?"))
print(smu.query("SYSTem:ERRor:COUNt?"))
print(smu.query("SYSTem:VERSion?"))
