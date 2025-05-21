#ifndef _DRIVER_HPP_
#define _DRIVER_HPP_

#include <asynPortDriver.h>

static constexpr char FULL_URL_STRING[] = "FULL_URL";
static constexpr char HTTP_METHOD_STRING[] = "HTTP_METHOD";
static constexpr char RESPONSE_STRING[] = "RESPONSE";
static constexpr char RESPONSE_FORMAT_STRING[] = "RESPONSE_FORMAT";
static constexpr char EXECUTE_STRING[] = "EXECUTE";
static constexpr char STATUS_CODE_STRING[] = "STATUS_CODE";

constexpr int MAX_CONTROLLERS = 1;
constexpr double DEFAULT_POLL_TIME = 1.0; // seconds

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
    AsynHttpClient(const char *asyn_port_name, const char *dev_port_name);
    // virtual void poll(void);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);

  private:
    double poll_time_;
    bool format_json_ = false;
    std::string full_url_;
    HTTPMethod method_ = HTTPMethod::GET;

  protected:
    asynUser *pasynUserAsynHttpClient_;
    int fullUrlIndex_;
    int httpMethodIndex_;
    int responseIndex_;
    int responseFormatIndex_;
    int executeIndex_;
    int statusCodeIndex_;
};

#endif
