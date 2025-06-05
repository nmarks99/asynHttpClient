# ../../bin/${EPICS_HOST_ARCH}/wthr st.cmd
< envPaths

dbLoadDatabase("../../dbd/iocwthrLinux.dbd")
iocwthrLinux_registerRecordDeviceDriver(pdbbase)

epicsEnvSet("IOCSH_PS1", "$(IOC)>")
epicsEnvSet("PREFIX", "wthr:")

AsynHttpClientConfig("client1")
dbLoadTemplate("requests.substitutions", "P=$(PREFIX),PORT=client1")

# We use the lunajson lua library included with ASYN_HTTP_CLIENT,
# so we must append to the lua package path
epicsEnvSet("LUA_PATH", "$(ASYN_HTTP_CLIENT)/asynHttpClientApp/src/lua/?.lua;$(LUA_PATH=)")

dbLoadRecords("weather.db", "P=$(PREFIX),R1=Req1:,R2=Req2:,R3=Req3:")
dbLoadTemplate("bidaily.substitutions", "P=$(PREFIX)")

###############################################################################
iocInit
###############################################################################

# print the time our boot was finished
date
