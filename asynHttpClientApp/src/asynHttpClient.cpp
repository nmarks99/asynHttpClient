#include <cstdio>
#include <epicsExport.h>
#include <epicsThread.h>
#include <iocsh.h>

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
                     ASYN_MULTIDEVICE | ASYN_CANBLOCK, 1, 0, 0) // autoConnect, priority, stackSize
{

    createParam(FULL_URL_STRING, asynParamOctet, &fullUrlIndex_);
    createParam(HTTP_METHOD_STRING, asynParamInt32, &httpMethodIndex_);
    createParam(RESPONSE_STRING, asynParamOctet, &responseIndex_);
    createParam(RESPONSE_FORMAT_STRING, asynParamInt32, &responseFormatIndex_);
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
        full_url_ = value;
        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER, "Setting URL to %s\n", full_url_.c_str());
        *nActual = full_url_.length();
    }

    callParamCallbacks();
    return status;
}

asynStatus AsynHttpClient::writeInt32(asynUser *pasynUser, epicsInt32 value) {
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;

    if (function == executeIndex_) {
        if (full_url_.length() > 0) {
            cpr::Response response;
            switch (method_) {
            case HTTPMethod::GET:
                response = cpr::Get(cpr::Url{full_url_});
                break;
            case HTTPMethod::HEAD:
                response = cpr::Head(cpr::Url{full_url_});
                break;
            case HTTPMethod::POST:
                response = cpr::Post(cpr::Url{full_url_});
                break;
            case HTTPMethod::PUT:
                response = cpr::Put(cpr::Url{full_url_});
                break;
            case HTTPMethod::DELETE:
                response = cpr::Delete(cpr::Url{full_url_});
                break;
            case HTTPMethod::OPTIONS:
                response = cpr::Options(cpr::Url{full_url_});
                break;
            case HTTPMethod::PATCH:
                response = cpr::Patch(cpr::Url{full_url_});
                break;
            case HTTPMethod::CONNECT:
                asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "CONNECT method not implemented\n");
                break;
            case HTTPMethod::TRACE:
                asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "TRACE method not implemented\n");
                break;
            case HTTPMethod::NONE:
                asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "No HTTP method specified\n");
                break;
            }
            setIntegerParam(statusCodeIndex_, response.status_code);

            std::string response_str = "";
            if (response.status_code != 200) {
                asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "Request failed with status code: %ld\n",
                          response.status_code);
                setStringParam(responseIndex_, response_str);
                callParamCallbacks();
                return asynStatus::asynError;
            }

            if (format_json_) {
                nlohmann::basic_json data;
                try {
                    data = nlohmann::json::parse(response.text);
                    response_str = data.dump(2);
                } catch (const std::exception &e) {
                    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s\n", e.what());
                    response_str = "";
                }
            } else {
                response_str = response.text;
            }
            setStringParam(responseIndex_, response_str);
        }
    }

    else if (function == httpMethodIndex_) {
        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER, "Setting HTTP method to %d\n", value);
        method_ = static_cast<HTTPMethod>(value);
    }

    else if (function == responseFormatIndex_) {
        format_json_ = static_cast<bool>(value);
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
