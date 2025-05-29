# ../../bin/${EPICS_HOST_ARCH}/xxx st.cmd
< envPaths

dbLoadDatabase("../../dbd/iocxxxLinux.dbd")
iocxxxLinux_registerRecordDeviceDriver(pdbbase)

epicsEnvSet("IOCSH_PS1", "$(IOC)>")
epicsEnvSet("PREFIX", "xxx:")

# Instantiate the client with a name
AsynHttpClientConfig("client1")

# Example: Control robot arm with REST API
iocshLoad("examples/ur_module/ur_module.iocsh", "PORT=client1")

###############################################################################
iocInit
###############################################################################

# print the time our boot was finished
date
