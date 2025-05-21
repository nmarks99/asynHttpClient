#include <cstdio>
#include <epicsExport.h>
#include <epicsThread.h>
#include <iocsh.h>
#include <iostream>

#include "asynHttpClient.hpp"

#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

// static void poll_thread_C(void *pPvt) {
// AsynHttpClient *pAsynHttpClient = (AsynHttpClient *)pPvt;
// pAsynHttpClient->poll();
// }

AsynHttpClient::AsynHttpClient(const char *asyn_port, const char *dev_port)
    : asynPortDriver(asyn_port, MAX_CONTROLLERS,
                     asynInt32Mask | asynFloat64Mask | asynDrvUserMask | asynOctetMask | asynInt32ArrayMask,
                     asynInt32Mask | asynFloat64Mask | asynOctetMask | asynInt32ArrayMask,
                     ASYN_MULTIDEVICE | ASYN_CANBLOCK,
                     1, 0, 0) // autoConnect, priority, stackSize
{

    createParam(FULL_URL_STRING, asynParamOctet, &fullUrlIndex_);
    createParam(HTTP_METHOD_STRING, asynParamInt32, &httpMethodIndex_);
    createParam(RESPONSE_STRING, asynParamOctet, &responseIndex_);
    createParam(EXECUTE_STRING, asynParamInt32, &executeIndex_);
    createParam(STATUS_CODE_STRING, asynParamInt32, &statusCodeIndex_);

    // epicsThreadCreate("AsynHttpClientPoller", epicsThreadPriorityLow,
    // epicsThreadGetStackSize(epicsThreadStackMedium), (EPICSTHREADFUNC)poll_thread_C, this);
}

// void AsynHttpClient::poll() {
// while (true) {
// lock();
//
// // if (base_url_.length() > 0 && endpoint_.length() > 0) {
// // cpr::Response status_resp = cpr::Get(cpr::Url{"http://localhost:8000/users"});
// // auto status_data = nlohmann::json::parse(status_resp.text);
// // printf("%s\n", status_data.dump(2).c_str());
// // }
//
// callParamCallbacks();
// unlock();
// epicsThreadSleep(poll_time_);
// }
// }

asynStatus AsynHttpClient::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars,
                                      size_t *nActual) {
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;

    if (function == fullUrlIndex_) {
        printf("Setting base URL to %s\n", value);
        full_url_ = value;
    }


    callParamCallbacks();
    return status;
}

asynStatus AsynHttpClient::writeInt32(asynUser *pasynUser, epicsInt32 value) {
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;

    if (function == executeIndex_) {
        if (full_url_.length() > 0) {
            std::cout << "full url: " << full_url_ << std::endl;
            cpr::Response response;
            switch (method_) {
                case HTTPMethod::POST:
                    response = cpr::Post(cpr::Url{full_url_});
                    break;
                case HTTPMethod::GET:
                    response = cpr::Get(cpr::Url{full_url_});
                    break;
            }
            setIntegerParam(statusCodeIndex_, response.status_code);

            if (response.status_code != 200) {
                std::cerr << "Request failed with status code: " << response.status_code << std::endl;
                response_ = "";
                setStringParam(responseIndex_, response_);
                callParamCallbacks();
                return asynError;
            }
           
            auto data = nlohmann::json::parse(response.text);
            setStringParam(responseIndex_, data.dump(2));

        }
    }

    else if (function == httpMethodIndex_) {
        if (value == 0) {
            std::cout << "Setting HTTP method to POST" << std::endl;
            method_ = HTTPMethod::POST;
        } else if (value == 1) {
            std::cout << "Setting HTTP method to GET" << std::endl;
            method_ = HTTPMethod::GET;
        }
    }

    callParamCallbacks();
    return status;
}

// register function for iocsh
extern "C" int AsynHttpClientConfig(const char *asyn_port_name, const char *robot_ip) {
    AsynHttpClient *pAsynHttpClient = new AsynHttpClient(asyn_port_name, robot_ip);
    (void)pAsynHttpClient;
    return (asynSuccess);
}

static const iocshArg httpClientArg0 = {"Asyn port name", iocshArgString};
static const iocshArg httpClientArg1 = {"Device port name", iocshArgString};
static const iocshArg *const httpClientArgs[2] = {&httpClientArg0, &httpClientArg1};
static const iocshFuncDef httpClientFuncDef = {"AsynHttpClientConfig", 2, httpClientArgs};

static void httpClientCallFunc(const iocshArgBuf *args) { AsynHttpClientConfig(args[0].sval, args[1].sval); }

void AsynHttpClientRegister(void) { iocshRegister(&httpClientFuncDef, httpClientCallFunc); }

extern "C" {
epicsExportRegistrar(AsynHttpClientRegister);
}
