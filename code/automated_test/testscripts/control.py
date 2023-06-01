import pyvisa as visa

rm = visa.ResourceManager()
print(rm.list_resources()) #打印当前计算机下所有的Visa能检测到的通信口

# smu = rm.open_resource("ASRL3::INSTR")
smu = rm.open_resource("USB0::0xCAFE::0x4000::12345678::INSTR")

print(smu.query("*IDN?"))
