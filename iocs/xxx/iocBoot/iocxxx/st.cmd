# ../../bin/${EPICS_HOST_ARCH}/xxx st.cmd
< envPaths

dbLoadDatabase("../../dbd/iocxxxLinux.dbd")
iocxxxLinux_registerRecordDeviceDriver(pdbbase)

epicsEnvSet("IOCSH_PS1", "$(IOC)>")
epicsEnvSet("PREFIX", "xxx:")
epicsEnvSet("LUA_SCRIPT_PATH", ".")

AsynHttpClientConfig("client1", "asdf")
dbLoadRecords("$(ASYN_HTTP_CLIENT)/asynHttpClientApp/Db/asynHttpClient.db","P=$(PREFIX),PORT=client1")

# dbLoadRecords("ur_module.db", "P=$(PREFIX)")

###############################################################################
iocInit
###############################################################################

dbpf "xxx:FullURL.VAL$" "http://localhost:3030/action?action_name=getj&args=%7B%7D"

# print the time our boot was finished
date
