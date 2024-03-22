local Thrift = require 'Thrift'
local auto_microservices_constants = require 'auto_microservices_constants'
local TType = Thrift.TType
local __TObject = Thrift.__TObject
local TException = Thrift.TException



local ErrorCode = {
  SE_CONNPOOL_TIMEOUT = 0,
  SE_THRIFT_CONN_ERROR = 1,
  SE_THRIFT_HANDLER_ERROR = 2
}

local ServiceException = TException:new{
  __type = 'ServiceException',
  errorCode,
  message
}

function ServiceException:read(iprot)
  iprot:readStructBegin()
  while true do
    local fname, ftype, fid = iprot:readFieldBegin()
    if ftype == TType.STOP then
      break
    elseif fid == 1 then
      if ftype == TType.I32 then
        self.errorCode = iprot:readI32()
      else
        iprot:skip(ftype)
      end
    elseif fid == 2 then
      if ftype == TType.STRING then
        self.message = iprot:readString()
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

function ServiceException:write(oprot)
  oprot:writeStructBegin('ServiceException')
  if self.errorCode ~= nil then
    oprot:writeFieldBegin('errorCode', TType.I32, 1)
    oprot:writeI32(self.errorCode)
    oprot:writeFieldEnd()
  end
  if self.message ~= nil then
    oprot:writeFieldBegin('message', TType.STRING, 2)
    oprot:writeString(self.message)
    oprot:writeFieldEnd()
  end
  oprot:writeFieldStop()
  oprot:writeStructEnd()
end
return {ErrorCode=ErrorCode, ServiceException=ServiceException}
