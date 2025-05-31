# ../../bin/${EPICS_HOST_ARCH}/xxx st.cmd
< envPaths

dbLoadDatabase("../../dbd/iocxxxLinux.dbd")
iocxxxLinux_registerRecordDeviceDriver(pdbbase)

epicsEnvSet("IOCSH_PS1", "$(IOC)>")
epicsEnvSet("PREFIX", "xxx:")

AsynHttpClientConfig("client1")
dbLoadTemplate("requests.substitutions", "P=$(PREFIX),PORT=client1")

epicsEnvSet("LUA_SCRIPT_PATH", ".")
dbLoadRecords("weather.db", "P=$(PREFIX),R1=Req1:,R2=Req2:,R3=Req3:")
dbLoadTemplate("bidaily.substitutions", "P=$(PREFIX)")

###############################################################################
iocInit
###############################################################################

# print the time our boot was finished
date
