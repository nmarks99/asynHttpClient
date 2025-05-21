epics = require("epics")
json = require("lunajson")

local function urlencode(str)
    return (str:gsub("[^%w%-._~]", function(c)
        return string.format("%%%02X", string.byte(c))
    end))
end

-- function test()
    -- local openval = true
    -- local closeval = false
--
    -- local url = "http://localhost:3030/action?action_name=toggle_gripper"
    -- local action_vars = {
        -- open = openval,
        -- close = closeval,
    -- }
--
    -- local json_args = json.encode(action_vars)
    -- local encoded_args = urlencode(json_args)
--
    -- local full_url = url .. "&args=" .. encoded_args .. "\n\n"
    -- return full_url
-- end

function string_to_table(str)
    local chars = {}
    for i = 1, #str do
        chars[i] = str:sub(i, i)
    end
    return chars
end

function test()
    local openval = true
    local closeval = false

    local url = "http://localhost:3030/action?action_name=toggle_gripper"
    local action_vars = {
        open = openval,
        close = closeval,
    }

    local json_args = json.encode(action_vars)
    local encoded_args = urlencode(json_args)

    full_url = url .. "&args=" .. encoded_args .. "\n\n"
    io.write("Full URL: ")
    print(full_url)
    -- return string_to_table("Hello world this is a long string that is longer than you might usually be allowed to do")
    return string_to_table(full_url)
end
