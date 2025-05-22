# ../../bin/${EPICS_HOST_ARCH}/xxx st.cmd
< envPaths

dbLoadDatabase("../../dbd/iocxxxLinux.dbd")
iocxxxLinux_registerRecordDeviceDriver(pdbbase)

epicsEnvSet("IOCSH_PS1", "$(IOC)>")
epicsEnvSet("PREFIX", "xxx:")

AsynHttpClientConfig("client1")
dbLoadRecords("$(ASYN_HTTP_CLIENT)/db/asynHttpClient.db","P=$(PREFIX),PORT=client1")

# Example that interacts with REST API for robot arm
< examples/ur_module/ur_module.iocsh

###############################################################################
iocInit
###############################################################################

# print the time our boot was finished
date
