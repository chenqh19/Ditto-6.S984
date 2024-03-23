/**
 * Autogenerated by Thrift Compiler (0.12.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef service_15_H
#define service_15_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "auto_microservices_types.h"

namespace auto_microservices {

#ifdef _MSC_VER
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class service_15If {
 public:
  virtual ~service_15If() {}
  virtual void rpc_15(const std::map<std::string, std::string> & carrier) = 0;
};

class service_15IfFactory {
 public:
  typedef service_15If Handler;

  virtual ~service_15IfFactory() {}

  virtual service_15If* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(service_15If* /* handler */) = 0;
};

class service_15IfSingletonFactory : virtual public service_15IfFactory {
 public:
  service_15IfSingletonFactory(const ::apache::thrift::stdcxx::shared_ptr<service_15If>& iface) : iface_(iface) {}
  virtual ~service_15IfSingletonFactory() {}

  virtual service_15If* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(service_15If* /* handler */) {}

 protected:
  ::apache::thrift::stdcxx::shared_ptr<service_15If> iface_;
};

class service_15Null : virtual public service_15If {
 public:
  virtual ~service_15Null() {}
  void rpc_15(const std::map<std::string, std::string> & /* carrier */) {
    return;
  }
};

typedef struct _service_15_rpc_15_args__isset {
  _service_15_rpc_15_args__isset() : carrier(false) {}
  bool carrier :1;
} _service_15_rpc_15_args__isset;

class service_15_rpc_15_args {
 public:

  service_15_rpc_15_args(const service_15_rpc_15_args&);
  service_15_rpc_15_args& operator=(const service_15_rpc_15_args&);
  service_15_rpc_15_args() {
  }

  virtual ~service_15_rpc_15_args() throw();
  std::map<std::string, std::string>  carrier;

  _service_15_rpc_15_args__isset __isset;

  void __set_carrier(const std::map<std::string, std::string> & val);

  bool operator == (const service_15_rpc_15_args & rhs) const
  {
    if (!(carrier == rhs.carrier))
      return false;
    return true;
  }
  bool operator != (const service_15_rpc_15_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const service_15_rpc_15_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class service_15_rpc_15_pargs {
 public:


  virtual ~service_15_rpc_15_pargs() throw();
  const std::map<std::string, std::string> * carrier;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _service_15_rpc_15_result__isset {
  _service_15_rpc_15_result__isset() : se(false) {}
  bool se :1;
} _service_15_rpc_15_result__isset;

class service_15_rpc_15_result {
 public:

  service_15_rpc_15_result(const service_15_rpc_15_result&);
  service_15_rpc_15_result& operator=(const service_15_rpc_15_result&);
  service_15_rpc_15_result() {
  }

  virtual ~service_15_rpc_15_result() throw();
  ServiceException se;

  _service_15_rpc_15_result__isset __isset;

  void __set_se(const ServiceException& val);

  bool operator == (const service_15_rpc_15_result & rhs) const
  {
    if (!(se == rhs.se))
      return false;
    return true;
  }
  bool operator != (const service_15_rpc_15_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const service_15_rpc_15_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _service_15_rpc_15_presult__isset {
  _service_15_rpc_15_presult__isset() : se(false) {}
  bool se :1;
} _service_15_rpc_15_presult__isset;

class service_15_rpc_15_presult {
 public:


  virtual ~service_15_rpc_15_presult() throw();
  ServiceException se;

  _service_15_rpc_15_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class service_15Client : virtual public service_15If {
 public:
  service_15Client(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  service_15Client(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
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
  void rpc_15(const std::map<std::string, std::string> & carrier);
  void send_rpc_15(const std::map<std::string, std::string> & carrier);
  void recv_rpc_15();
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class service_15Processor : public ::apache::thrift::TDispatchProcessor {
 protected:
  ::apache::thrift::stdcxx::shared_ptr<service_15If> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (service_15Processor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_rpc_15(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  service_15Processor(::apache::thrift::stdcxx::shared_ptr<service_15If> iface) :
    iface_(iface) {
    processMap_["rpc_15"] = &service_15Processor::process_rpc_15;
  }

  virtual ~service_15Processor() {}
};

class service_15ProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  service_15ProcessorFactory(const ::apache::thrift::stdcxx::shared_ptr< service_15IfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::apache::thrift::stdcxx::shared_ptr< service_15IfFactory > handlerFactory_;
};

class service_15Multiface : virtual public service_15If {
 public:
  service_15Multiface(std::vector<apache::thrift::stdcxx::shared_ptr<service_15If> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~service_15Multiface() {}
 protected:
  std::vector<apache::thrift::stdcxx::shared_ptr<service_15If> > ifaces_;
  service_15Multiface() {}
  void add(::apache::thrift::stdcxx::shared_ptr<service_15If> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void rpc_15(const std::map<std::string, std::string> & carrier) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->rpc_15(carrier);
    }
    ifaces_[i]->rpc_15(carrier);
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class service_15ConcurrentClient : virtual public service_15If {
 public:
  service_15ConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  service_15ConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
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
  void rpc_15(const std::map<std::string, std::string> & carrier);
  int32_t send_rpc_15(const std::map<std::string, std::string> & carrier);
  void recv_rpc_15(const int32_t seqid);
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