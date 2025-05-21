local script_dir = debug.getinfo(1, "S").source:match("@(.*/)")
package.path = script_dir .. "?.lua;" .. package.path

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

function get_url_toggle_gripper()
    epics.put("xxx:HTTPMethod", 2)
    local openval
    local closeval

    if A == 1 then
        print("Getting url for closing gripper")
        openval = false
        closeval = true
    else
        print("Getting url for opening gripper")
        openval = true
        closeval = false
    end

    local url = "http://localhost:3030/action?action_name=toggle_gripper"
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
