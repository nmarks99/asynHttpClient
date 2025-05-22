-- Correct path to ASYN_HTTP_CLIENT
local ASYN_HTTP_CLIENT = "/home/beams/NMARKS/devel/asynHttpClient/"
package.path = ASYN_HTTP_CLIENT .. "asynHttpClientApp/src/lua/?.lua;" .. package.path

epics = require("epics")
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

function get_url_toggle_gripper(args)
    local openval
    local closeval

    if A == 1 then
        openval = false
        closeval = true
    else
        openval = true
        closeval = false
    end

    local url = string.format("http://%s/action?action_name=toggle_gripper", args.host)
    local action_vars = {
        open = openval,
        close = closeval,
    }
    local json_args = json.encode(action_vars)
    local encoded_args = urlencode(json_args)
    full_url = url .. "&args=" .. encoded_args

    io.write("URL: ")
    print(full_url)

    return string_to_table(full_url)
end
