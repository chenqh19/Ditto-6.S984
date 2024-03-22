local Thrift = require 'Thrift'
local auto_microservices_ttypes = require 'auto_microservices_ttypes'
local TType = Thrift.TType
local TMessageType = Thrift.TMessageType
local __TObject = Thrift.__TObject
local TApplicationException = Thrift.TApplicationException
local __TClient = Thrift.__TClient
local __TProcessor = Thrift.__TProcessor
local ttype = Thrift.ttype
local ttable_size = Thrift.ttable_size
local ServiceException = auto_microservices_ttypes.ServiceException

local rpc_0_args = __TObject:new{
  carrier
}

function rpc_0_args:read(iprot)
  iprot:readStructBegin()
  while true do
    local fname, ftype, fid = iprot:readFieldBegin()
    if ftype == TType.STOP then
      break
    elseif fid == 1 then
      if ftype == TType.MAP then
        self.carrier = {}
        local _ktype1, _vtype2, _size0 = iprot:readMapBegin() 
        for _i=1,_size0 do
          local _key4 = iprot:readString()
          local _val5 = iprot:readString()
          self.carrier[_key4] = _val5
        end
        iprot:readMapEnd()
      else
        iprot:skip(ftype)
      end
    else
      iprot:skip(ftype)
    end
    iprot:readFieldEnd()
  end
  iprot:readStructEnd()
end

function rpc_0_args:write(oprot)
  oprot:writeStructBegin('rpc_0_args')
  if self.carrier ~= nil then
    oprot:writeFieldBegin('carrier', TType.MAP, 1)
    oprot:writeMapBegin(TType.STRING, TType.STRING, ttable_size(self.carrier))
    for kiter6,viter7 in pairs(self.carrier) do
      oprot:writeString(kiter6)
      oprot:writeString(viter7)
    end
    oprot:writeMapEnd()
    oprot:writeFieldEnd()
  end
  oprot:writeFieldStop()
  oprot:writeStructEnd()
end

local rpc_0_result = __TObject:new{
  se
}

function rpc_0_result:read(iprot)
  iprot:readStructBegin()
  while true do
    local fname, ftype, fid = iprot:readFieldBegin()
    if ftype == TType.STOP then
      break
    elseif fid == 1 then
      if ftype == TType.STRUCT then
        self.se = ServiceException:new{}
        self.se:read(iprot)
      else
        iprot:skip(ftype)
      end
    else
      iprot:skip(ftype)
    end
    iprot:readFieldEnd()
  end
  iprot:readStructEnd()
end

function rpc_0_result:write(oprot)
  oprot:writeStructBegin('rpc_0_result')
  if self.se ~= nil then
    oprot:writeFieldBegin('se', TType.STRUCT, 1)
    self.se:write(oprot)
    oprot:writeFieldEnd()
  end
  oprot:writeFieldStop()
  oprot:writeStructEnd()
end



local service_0Client = __TObject.new(__TClient, {
  __type = 'service_0Client'
})

function service_0Client:rpc_0(carrier)
  self:send_rpc_0(carrier)
  self:recv_rpc_0(carrier)
end

function service_0Client:send_rpc_0(carrier)
  self.oprot:writeMessageBegin('rpc_0', TMessageType.CALL, self._seqid)
  local args = rpc_0_args:new{}
  args.carrier = carrier
  args:write(self.oprot)
  self.oprot:writeMessageEnd()
  self.oprot.trans:flush()
end

function service_0Client:recv_rpc_0(carrier)
  local fname, mtype, rseqid = self.iprot:readMessageBegin()
  if mtype == TMessageType.EXCEPTION then
    local x = TApplicationException:new{}
    x:read(self.iprot)
    self.iprot:readMessageEnd()
    error(x)
  end
  local result = rpc_0_result:new{}
  result:read(self.iprot)
  self.iprot:readMessageEnd()
end
local service_0Iface = __TObject:new{
  __type = 'service_0Iface'
}


local service_0Processor = __TObject.new(__TProcessor
, {
 __type = 'service_0Processor'
})

function service_0Processor:process(iprot, oprot, server_ctx)
  local name, mtype, seqid = iprot:readMessageBegin()
  local func_name = 'process_' .. name
  if not self[func_name] or ttype(self[func_name]) ~= 'function' then
    iprot:skip(TType.STRUCT)
    iprot:readMessageEnd()
    x = TApplicationException:new{
      errorCode = TApplicationException.UNKNOWN_METHOD
    }
    oprot:writeMessageBegin(name, TMessageType.EXCEPTION, seqid)
    x:write(oprot)
    oprot:writeMessageEnd()
    oprot.trans:flush()
  else
    self[func_name](self, seqid, iprot, oprot, server_ctx)
  end
end

function service_0Processor:process_rpc_0(seqid, iprot, oprot, server_ctx)
  local args = rpc_0_args:new{}
  local reply_type = TMessageType.REPLY
  args:read(iprot)
  iprot:readMessageEnd()
  local result = rpc_0_result:new{}
  local status, res = pcall(self.handler.rpc_0, self.handler, args.carrier)
  if not status then
    reply_type = TMessageType.EXCEPTION
    result = TApplicationException:new{message = res}
  elseif ttype(res) == 'ServiceException' then
    result.se = res
  else
    result.success = res
  end
  oprot:writeMessageBegin('rpc_0', reply_type, seqid)
  result:write(oprot)
  oprot:writeMessageEnd()
  oprot.trans:flush()
end

return service_0Client
