-- Correct path to ASYN_HTTP_CLIENT
local ASYN_HTTP_CLIENT = "/home/beams/NMARKS/devel/asynHttpClient/"
package.path = ASYN_HTTP_CLIENT .. "asynHttpClientApp/src/lua/?.lua;" .. package.path

epics = require("epics")
osi = require("osi")
json = require("lunajson")

local function urlencode(str)
    return (str:gsub("[^%w%-._~]", function(c)
        return string.format("%%%02X", string.byte(c))
    end))
end

function string_to_table(str)
    local chars = {}
    for i = 1, #str do
        chars[i] = str:sub(i, i)
    end
    return chars
end

function string_strip(s)
    return s:match("^%s*(.-)%s*$")
end

function set_points_url()
    local url = string.format("https://api.weather.gov/points/%f,%f", A, B)
    return string_to_table(url)
end

function set_hourly_url()
    local url = string_strip(table.concat(AA))
    if #url > 0 then
        url = url .. "/hourly"
        return string_to_table(url)
    end
end

function parse_weekly_forecast(args)
    local periods_string = string_strip(table.concat(AA))
    if #periods_string > 0 then
        local periods, err = json.decode(periods_string)
        if not periods then
            print("Error decoding JSON: " .. tostring(err))
            return
        end
        for i,p in ipairs(periods) do
            epics.put(string.format("%sForecast%d:Name",args.P, i), p.name)
            epics.put(string.format("%sForecast%d:Temperature",args.P, i), p.temperature)
            epics.put(string.format("%sForecast%d:ProbPrecip",args.P, i), p.probabilityOfPrecipitation.value)
        end
    end
end
