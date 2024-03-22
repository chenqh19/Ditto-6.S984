local _M = {}

local function _StrIsEmpty(s)
  return s == nil or s == ''
end

function _M.rpc_0()
  -- local bridge_tracer = require "opentracing_bridge_tracer"
  local ngx = ngx
  local GenericObjectPool = require "GenericObjectPool"
  local entryClient = require "auto_microservices_service_0"

  -- local tracer = bridge_tracer.new_from_global()
  -- local parent_span_context = tracer:binary_extract(
  --     ngx.var.opentracing_binary_context)
  -- local span = tracer:start_span("rpc_0_client",
  --     {["references"] = {{"child_of", parent_span_context}}})
  local carrier = {}
  -- tracer:text_map_inject(span:context(), carrier)

  local client = GenericObjectPool:connection(
    entryClient, "0.0.0.0", 9000)

  local status
  local err
  status, err = pcall(client.rpc_0, client, carrier)

  if not status then
    ngx.status = ngx.HTTP_INTERNAL_SERVER_ERROR
    ngx.say("rpc_0 Failed: " .. err.message)
    ngx.log(ngx.ERR, "rpc_0 Failed: " .. err.message)
    ngx.exit(ngx.HTTP_INTERNAL_SERVER_ERROR)
  else
    ngx.say(err)
  end
  GenericObjectPool:returnConnection(client)
  -- span:finish()
end

return _M