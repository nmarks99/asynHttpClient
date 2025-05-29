#ifndef _DRIVER_HPP_
#define _DRIVER_HPP_

#include <asynPortDriver.h>
#include "nlohmann/json.hpp"

static constexpr char FULL_URL_STRING[] = "FULL_URL";
static constexpr char HTTP_METHOD_STRING[] = "HTTP_METHOD";
static constexpr char RESPONSE_STRING[] = "RESPONSE";
static constexpr char RESPONSE_FORMAT_STRING[] = "RESPONSE_FORMAT";
static constexpr char EXECUTE_STRING[] = "EXECUTE";
static constexpr char STATUS_CODE_STRING[] = "STATUS_CODE";

static constexpr char JSON_PARSER_KEY_STRING[] = "JSON_PARSER_KEY";
static constexpr char JSON_PARSER_VALUE_STRING[] = "JSON_PARSER_VALUE";

// HACK: should be a argument to constructor?
constexpr int MAX_ADDR = 10;

enum class HTTPMethod {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH,
    NONE,
};

class AsynHttpClient : public asynPortDriver {
  public:
    AsynHttpClient(const char *port_name);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);

  private:
    double poll_time_;
    bool format_json_ = false;
    std::string full_url_;
    nlohmann::json response_json_;
    HTTPMethod method_ = HTTPMethod::GET;

  protected:
    asynUser *pasynUserAsynHttpClient_;
    int fullUrlIndex_;
    int httpMethodIndex_;
    int responseIndex_;
    int responseFormatIndex_;
    int executeIndex_;
    int statusCodeIndex_;
    int jsonParserKeyIndex_;
    int jsonParserValueIndex_;
};

#endif
