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

function string_strip(s)
    return s:match("^%s*(.-)%s*$")
end

function toggle_gripper_set_post_url(args)
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

    -- io.write("URL: ")
    -- print(full_url)

    return string_to_table(full_url)
end

function toggle_gripper_set_get_url(args)
    local action_id = string_strip(table.concat(AA))
    if (#action_id > 0) then
        local url = string.format("http://%s/action/%s", args.host, action_id)
        return string_to_table(url)
    end
end
