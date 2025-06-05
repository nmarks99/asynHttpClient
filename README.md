# asynHttpClient

EPICS support library for sending HTTP requests from EPICS records via a thin
asynPortDriver layer on top of [cpr](https://github.com/libcpr/cpr).

This support provides an EPICS database `http_request.db` which respresents a single HTTP request.
Included in this database are the following records:
```
record(lso, "$(P)$(R)URL") { ... }
record(mbbo, "$(P)$(R)Method") { ... }
record(bo, "$(P)$(R)Execute") { ... }
record(longin, "$(P)$(R)StatusCode") { ... }
record(lsi, "$(P)$(R)Response") { ... }
record(bo, "$(P)$(R)ResponseFormat") { ... }
record(stringout, "$(P)$(R)JSONParserKey") { ... }
record(lsi, "$(P)$(R)JSONParserValue") { ... }
record(longout, "$(P)$(R)Post") { ... }
record(longout, "$(P)$(R)Get") { ... }
```
*Note:* Pay attention to the use of long string records and make sure to use the "$" filter when needed.

## Dependencies

### cpr

Compile cpr as both a shared library and static library. To do this, run `cmake`
with -DCMAKE_BUILD_SHARED_LIBS=ON/OFF and -DCMAKE_POSITION_INDEPENDENT_CODE=ON,
then run `make install` to install to the specified install prefix.

To compile as a static library:
```
cmake .. -DCPR_USE_SYSTEM_CURL=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/home/beams/NMARKS/.local -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON
```

To compile as a shared library:
```
cmake .. -DCPR_USE_SYSTEM_CURL=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=/home/beams/NMARKS/.local -DCMAKE_BUILD_TYPE=Release
```

## Usage

*For a complete example IOC, see ./iocs/wthr*

To send an HTTP request, load an instance of `http_request.db` in your IOC, set the `URL` and `Method` PVs, then
run `Execute` to send the request. The response from the request is stored in `Response` in the format specified
by `ResponseFormat` (raw or JSON), and the status code is stored in `StatusCode`.

Basic JSON parsing is available by setting `JSONParserKey`. When this PV is set,
the value for the given key is extracted from `Response` and stored in `JSONParserValue`.
If `Response` isn't valid JSON or the key is not found, `JSONParserValue` will be empty.
Dot notation to extract nested JSON keys is supported. For example, if the response has
a top level key called "groceries" and within that another key "fruits", you can extract the value of "fruits"
by setting `JSONParserKey` to "groceries.fruits".

In many situations, it can be useful to load several request instances and configure them ahead of time.
To do this you can use a substitutions file. For example:
```
file "$(ASYN_HTTP_CLIENT)/db/http_request.db"
{
pattern
{R,       ADDR, METHOD, FORMAT,  PARSE}
{"Req1:",    0,      0,      1,  "properties.forecast"}
{"Req2:",    1,      0       1,  "properties.periods"}
{"Req3:",    2,      0       1,  "properties.periods"}
}
```
Note that each request must have a unique ADDR, and the METHOD, FORMAT, and PARSE macros are
used to set the `Method`, `ResponseFormat`, and `JSONParserKey` PVs before loading the database.

Many applications may require encoding several PVs into a URL. At this time, URL encoding is left
up to the IOC programmer and not handled by this library.
One way to make constructing URLs from PVs easier is to use the
[luascript](https://github.com/epics-modules/lua) record.
A JSON parsing library [lunajson](https://github.com/grafi-tt/lunajson) is included with this module
and can be used in your luascript records by setting the `LUA_PATH` environment variable. For example,
you might have a luascript record that takes several other PVs as input links, then builds the full
URL in lua code:

EPICS database:
```
record(luascript, "$(P)set_url") {
    field(INPA, "$(P)Value1.VAL")
    field(INPB, "$(P)Value2.VAL")
    field(INPC, "$(P)Value2.VAL")
    field(CODE, "@urlbuilder.lua set_url()")
    field(SYNC, "Sync")
    field(OUT, "$(P)URL.VAL$ PP")
}
```

urlbuilder.lua:
```lua
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

function set_url(args)
    local url = string.format("http://%s/endpoint", args.host)
    local vars = {
	value1 = A,
	value2 = B,
	value3 = C
    }
    local json_args = json.encode(vars)
    local encoded_args = urlencode(json_args)
    full_url = url .. "&args=" .. encoded_args

    -- must return string as table (array)
    return string_to_table(full_url)
end
```

