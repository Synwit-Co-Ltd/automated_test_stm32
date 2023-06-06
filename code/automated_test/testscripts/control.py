import pyvisa as visa

rm = visa.ResourceManager()
print(rm.list_resources()) #打印当前计算机下所有的Visa能检测到的通信口

# smu = rm.open_resource("ASRL3::INSTR")
smu = rm.open_resource("USB0::0xCAFE::0x4000::12345678::INSTR")

print(smu.query("*IDN?"))

# print(smu.query("CONFigure:VOLTage:DC 6.17,8.14"))
# print(smu.query("MEASure:VOLTage:DC? 617,814"))
# print(smu.query("MEASure:VOLTage:DC:RATio?"))
# print(smu.query("TEST:BOOL ON"))
# print(smu.query("TEST:CHOice? IMMediate"))
# print(smu.query("TEST2:NUMbers3"))
# print(smu.query("TEST:TEXT 'HelloWorld'"))
# print(smu.query("TEST:ARBitrary? #204ABCD"))
# print(smu.query("TEST:CHANnellist (@1!3:3!1)"))