namespace cpp auto_microservices
namespace lua auto_microservices

enum ErrorCode {
  SE_CONNPOOL_TIMEOUT,
  SE_THRIFT_CONN_ERROR,
  SE_THRIFT_HANDLER_ERROR
}
exception ServiceException {
  1: ErrorCode errorCode;
  2: string message;
}
service service_0
{
  void rpc_0(1: map<string, string> carrier) throws (1: ServiceException se)
}

service service_16
{
  void rpc_16(1: map<string, string> carrier) throws (1: ServiceException se)
}

service service_17
{
  void rpc_17(1: map<string, string> carrier) throws (1: ServiceException se)
}

service service_18
{
  void rpc_18(1: map<string, string> carrier) throws (1: ServiceException se)
}

