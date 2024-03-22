--
-- Autogenerated by Thrift
--
-- DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
-- @generated
--


require 'Thrift'
require 'auto_microservices_ttypes'

service_2Client = __TObject.new(__TClient, {
  __type = 'service_2Client'
})

function service_2Client:rpc_2(carrier)
  self:send_rpc_2(carrier)
  self:recv_rpc_2(carrier)
end

function service_2Client:send_rpc_2(carrier)
  self.oprot:writeMessageBegin('rpc_2', TMessageType.CALL, self._seqid)
  local args = rpc_2_args:new{}
  args.carrier = carrier
  args:write(self.oprot)
  self.oprot:writeMessageEnd()
  self.oprot.trans:flush()
end

function service_2Client:recv_rpc_2(carrier)
  local fname, mtype, rseqid = self.iprot:readMessageBegin()
  if mtype == TMessageType.EXCEPTION then
    local x = TApplicationException:new{}
    x:read(self.iprot)
    self.iprot:readMessageEnd()
    error(x)
  end
  local result = rpc_2_result:new{}
  result:read(self.iprot)
  self.iprot:readMessageEnd()
end
service_2Iface = __TObject:new{
  __type = 'service_2Iface'
}


service_2Processor = __TObject.new(__TProcessor
, {
 __type = 'service_2Processor'
})

function service_2Processor:process(iprot, oprot, server_ctx)
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

function service_2Processor:process_rpc_2(seqid, iprot, oprot, server_ctx)
  local args = rpc_2_args:new{}
  local reply_type = TMessageType.REPLY
  args:read(iprot)
  iprot:readMessageEnd()
  local result = rpc_2_result:new{}
  local status, res = pcall(self.handler.rpc_2, self.handler, args.carrier)
  if not status then
    reply_type = TMessageType.EXCEPTION
    result = TApplicationException:new{message = res}
  elseif ttype(res) == 'ServiceException' then
    result.se = res
  else
    result.success = res
  end
  oprot:writeMessageBegin('rpc_2', reply_type, seqid)
  result:write(oprot)
  oprot:writeMessageEnd()
  oprot.trans:flush()
end

-- HELPER FUNCTIONS AND STRUCTURES

rpc_2_args = __TObject:new{
  carrier
}

function rpc_2_args:read(iprot)
  iprot:readStructBegin()
  while true do
    local fname, ftype, fid = iprot:readFieldBegin()
    if ftype == TType.STOP then
      break
    elseif fid == 1 then
      if ftype == TType.MAP then
        self.carrier = {}
        local _ktype17, _vtype18, _size16 = iprot:readMapBegin() 
        for _i=1,_size16 do
          local _key20 = iprot:readString()
          local _val21 = iprot:readString()
          self.carrier[_key20] = _val21
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

function rpc_2_args:write(oprot)
  oprot:writeStructBegin('rpc_2_args')
  if self.carrier ~= nil then
    oprot:writeFieldBegin('carrier', TType.MAP, 1)
    oprot:writeMapBegin(TType.STRING, TType.STRING, ttable_size(self.carrier))
    for kiter22,viter23 in pairs(self.carrier) do
      oprot:writeString(kiter22)
      oprot:writeString(viter23)
    end
    oprot:writeMapEnd()
    oprot:writeFieldEnd()
  end
  oprot:writeFieldStop()
  oprot:writeStructEnd()
end

rpc_2_result = __TObject:new{
  se
}

function rpc_2_result:read(iprot)
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

function rpc_2_result:write(oprot)
  oprot:writeStructBegin('rpc_2_result')
  if self.se ~= nil then
    oprot:writeFieldBegin('se', TType.STRUCT, 1)
    self.se:write(oprot)
    oprot:writeFieldEnd()
  end
  oprot:writeFieldStop()
  oprot:writeStructEnd()
end