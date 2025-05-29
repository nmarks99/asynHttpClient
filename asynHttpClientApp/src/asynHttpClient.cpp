#include <cstdio>
#include <epicsExport.h>
#include <epicsThread.h>
#include <iocsh.h>

#include "asynHttpClient.hpp"

#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

AsynHttpClient::AsynHttpClient(const char *port_name)
    : asynPortDriver(port_name, MAX_ADDR,
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
    createParam(JSON_PARSER_KEY_STRING, asynParamOctet, &jsonParserKeyIndex_);
    createParam(JSON_PARSER_VALUE_STRING, asynParamOctet, &jsonParserValueIndex_);
}

std::vector<std::string> get_json_path_keys(const std::string &path) {
    std::vector<std::string> keys;
    std::string segment;
    std::istringstream path_stream(path);
    while (std::getline(path_stream, segment, '.')) {
	if (!segment.empty()) { // Handle cases like "key1..key2" or trailing/leading dots
	    keys.push_back(segment);
	}
    }
    return keys;
}

asynStatus AsynHttpClient::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars,
                                      size_t *nActual) {
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;

    int addr;
    getAddress(pasynUser, &addr);

    if (function == fullUrlIndex_) {
        setStringParam(addr, fullUrlIndex_, value);
        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER, "Setting URL to %s\n", value);
    }
    else if (function == jsonParserKeyIndex_) {        

        nlohmann::json response_json;
        if (json_response_map_.count(addr) > 0) {
            response_json = json_response_map_.at(addr);
        }

        std::string json_val_out = "";
        if (!response_json.empty() and strlen(value) > 0) {
            std::vector<std::string> keys = get_json_path_keys(value);

            nlohmann::json node = response_json;
            bool found = true;
            for (const auto &k : keys) {
                if (node.contains(k)) {
                    node = node.at(k);
                } else {
                    found = false;
                    break;
                }
            }

            if (found) {
                json_val_out = node.dump(2);
                if (json_val_out.front() == '"' and json_val_out.back() == '"') {
                    json_val_out.erase(0,1);
                    json_val_out.pop_back();
                }
            } else {
                asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "Key '%s' not found in latest JSON response\n", value);
            }
        }

        setStringParam(addr, jsonParserValueIndex_, json_val_out);
    } 

    *nActual = strlen(value);
    callParamCallbacks(addr);
    return status;
}

asynStatus AsynHttpClient::writeInt32(asynUser *pasynUser, epicsInt32 value) {
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;

    int addr;
    getAddress(pasynUser, &addr);

    if (function == executeIndex_) {
        std::string full_url;
        getStringParam(addr, fullUrlIndex_, full_url);

        if (full_url.length() > 0) {
            cpr::Response response;

            int method;
            getIntegerParam(addr, httpMethodIndex_, &method);
            switch (static_cast<HTTPMethod>(method)) {
            case HTTPMethod::GET:
                response = cpr::Get(cpr::Url{full_url});
                break;
            case HTTPMethod::HEAD:
                response = cpr::Head(cpr::Url{full_url});
                break;
            case HTTPMethod::POST:
                response = cpr::Post(cpr::Url{full_url});
                break;
            case HTTPMethod::PUT:
                response = cpr::Put(cpr::Url{full_url});
                break;
            case HTTPMethod::DELETE:
                response = cpr::Delete(cpr::Url{full_url});
                break;
            case HTTPMethod::OPTIONS:
                response = cpr::Options(cpr::Url{full_url});
                break;
            case HTTPMethod::PATCH:
                response = cpr::Patch(cpr::Url{full_url});
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
            setIntegerParam(addr, statusCodeIndex_, response.status_code);

            std::string response_str = "";
            if (response.status_code != 200) {
                asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "Request failed with status code: %ld\n",
                          response.status_code);
                setStringParam(addr, responseIndex_, response_str);
                callParamCallbacks(addr);
                return asynStatus::asynError;
            }

            int format_json;
            getIntegerParam(addr, responseFormatIndex_, &format_json);
            if (static_cast<bool>(format_json)) {
                nlohmann::json json_data;
                try {
                    json_data = nlohmann::json::parse(response.text);
                    // response_json_ = json_data;
                    json_response_map_.insert_or_assign(addr, json_data);
                    response_str = json_data.dump(2);
                } catch (const std::exception &e) {
                    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s\n", e.what());
                    response_str = "";
                }
            } else {
                response_str = response.text;
            }
            setStringParam(addr, responseIndex_, response_str);
        }
    }

    else if (function == httpMethodIndex_) {
        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER, "Setting HTTP method to %d\n", value);
        setIntegerParam(addr, httpMethodIndex_, value);
    }

    else if (function == responseFormatIndex_) {
        setIntegerParam(addr, responseFormatIndex_, value);
    }

    callParamCallbacks(addr);
    return status;
}

// //////////////////
// Register for iocsh
// //////////////////
extern "C" int AsynHttpClientConfig(const char *asyn_port_name) {
    AsynHttpClient *pAsynHttpClient = new AsynHttpClient(asyn_port_name);
    (void)pAsynHttpClient;
    return (asynSuccess);
}

static const iocshArg httpClientArg0 = {"Asyn port name", iocshArgString};
static const iocshArg *const httpClientArgs[1] = {&httpClientArg0};
static const iocshFuncDef httpClientFuncDef = {"AsynHttpClientConfig", 1, httpClientArgs};

static void httpClientCallFunc(const iocshArgBuf *args) { AsynHttpClientConfig(args[0].sval); }

void AsynHttpClientRegister(void) { iocshRegister(&httpClientFuncDef, httpClientCallFunc); }

extern "C" {
epicsExportRegistrar(AsynHttpClientRegister);
}
