/**
 * Autogenerated by Thrift Compiler (0.12.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef service_17_H
#define service_17_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "auto_microservices_types.h"

namespace auto_microservices {

#ifdef _MSC_VER
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class service_17If {
 public:
  virtual ~service_17If() {}
  virtual void rpc_17(const std::map<std::string, std::string> & carrier) = 0;
};

class service_17IfFactory {
 public:
  typedef service_17If Handler;

  virtual ~service_17IfFactory() {}

  virtual service_17If* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(service_17If* /* handler */) = 0;
};

class service_17IfSingletonFactory : virtual public service_17IfFactory {
 public:
  service_17IfSingletonFactory(const ::apache::thrift::stdcxx::shared_ptr<service_17If>& iface) : iface_(iface) {}
  virtual ~service_17IfSingletonFactory() {}

  virtual service_17If* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(service_17If* /* handler */) {}

 protected:
  ::apache::thrift::stdcxx::shared_ptr<service_17If> iface_;
};

class service_17Null : virtual public service_17If {
 public:
  virtual ~service_17Null() {}
  void rpc_17(const std::map<std::string, std::string> & /* carrier */) {
    return;
  }
};

typedef struct _service_17_rpc_17_args__isset {
  _service_17_rpc_17_args__isset() : carrier(false) {}
  bool carrier :1;
} _service_17_rpc_17_args__isset;

class service_17_rpc_17_args {
 public:

  service_17_rpc_17_args(const service_17_rpc_17_args&);
  service_17_rpc_17_args& operator=(const service_17_rpc_17_args&);
  service_17_rpc_17_args() {
  }

  virtual ~service_17_rpc_17_args() throw();
  std::map<std::string, std::string>  carrier;

  _service_17_rpc_17_args__isset __isset;

  void __set_carrier(const std::map<std::string, std::string> & val);

  bool operator == (const service_17_rpc_17_args & rhs) const
  {
    if (!(carrier == rhs.carrier))
      return false;
    return true;
  }
  bool operator != (const service_17_rpc_17_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const service_17_rpc_17_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class service_17_rpc_17_pargs {
 public:


  virtual ~service_17_rpc_17_pargs() throw();
  const std::map<std::string, std::string> * carrier;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _service_17_rpc_17_result__isset {
  _service_17_rpc_17_result__isset() : se(false) {}
  bool se :1;
} _service_17_rpc_17_result__isset;

class service_17_rpc_17_result {
 public:

  service_17_rpc_17_result(const service_17_rpc_17_result&);
  service_17_rpc_17_result& operator=(const service_17_rpc_17_result&);
  service_17_rpc_17_result() {
  }

  virtual ~service_17_rpc_17_result() throw();
  ServiceException se;

  _service_17_rpc_17_result__isset __isset;

  void __set_se(const ServiceException& val);

  bool operator == (const service_17_rpc_17_result & rhs) const
  {
    if (!(se == rhs.se))
      return false;
    return true;
  }
  bool operator != (const service_17_rpc_17_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const service_17_rpc_17_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _service_17_rpc_17_presult__isset {
  _service_17_rpc_17_presult__isset() : se(false) {}
  bool se :1;
} _service_17_rpc_17_presult__isset;

class service_17_rpc_17_presult {
 public:


  virtual ~service_17_rpc_17_presult() throw();
  ServiceException se;

  _service_17_rpc_17_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class service_17Client : virtual public service_17If {
 public:
  service_17Client(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  service_17Client(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void rpc_17(const std::map<std::string, std::string> & carrier);
  void send_rpc_17(const std::map<std::string, std::string> & carrier);
  void recv_rpc_17();
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class service_17Processor : public ::apache::thrift::TDispatchProcessor {
 protected:
  ::apache::thrift::stdcxx::shared_ptr<service_17If> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (service_17Processor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_rpc_17(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  service_17Processor(::apache::thrift::stdcxx::shared_ptr<service_17If> iface) :
    iface_(iface) {
    processMap_["rpc_17"] = &service_17Processor::process_rpc_17;
  }

  virtual ~service_17Processor() {}
};

class service_17ProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  service_17ProcessorFactory(const ::apache::thrift::stdcxx::shared_ptr< service_17IfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::apache::thrift::stdcxx::shared_ptr< service_17IfFactory > handlerFactory_;
};

class service_17Multiface : virtual public service_17If {
 public:
  service_17Multiface(std::vector<apache::thrift::stdcxx::shared_ptr<service_17If> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~service_17Multiface() {}
 protected:
  std::vector<apache::thrift::stdcxx::shared_ptr<service_17If> > ifaces_;
  service_17Multiface() {}
  void add(::apache::thrift::stdcxx::shared_ptr<service_17If> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void rpc_17(const std::map<std::string, std::string> & carrier) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->rpc_17(carrier);
    }
    ifaces_[i]->rpc_17(carrier);
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class service_17ConcurrentClient : virtual public service_17If {
 public:
  service_17ConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  service_17ConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void rpc_17(const std::map<std::string, std::string> & carrier);
  int32_t send_rpc_17(const std::map<std::string, std::string> & carrier);
  void recv_rpc_17(const int32_t seqid);
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
  ::apache::thrift::async::TConcurrentClientSyncInfo sync_;
};

#ifdef _MSC_VER
  #pragma warning( pop )
#endif

} // namespace

#endif
