/**
 * Autogenerated by Thrift Compiler (0.12.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef service_18_H
#define service_18_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "auto_microservices_types.h"

namespace auto_microservices {

#ifdef _MSC_VER
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class service_18If {
 public:
  virtual ~service_18If() {}
  virtual void rpc_18(const std::map<std::string, std::string> & carrier) = 0;
};

class service_18IfFactory {
 public:
  typedef service_18If Handler;

  virtual ~service_18IfFactory() {}

  virtual service_18If* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(service_18If* /* handler */) = 0;
};

class service_18IfSingletonFactory : virtual public service_18IfFactory {
 public:
  service_18IfSingletonFactory(const ::apache::thrift::stdcxx::shared_ptr<service_18If>& iface) : iface_(iface) {}
  virtual ~service_18IfSingletonFactory() {}

  virtual service_18If* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(service_18If* /* handler */) {}

 protected:
  ::apache::thrift::stdcxx::shared_ptr<service_18If> iface_;
};

class service_18Null : virtual public service_18If {
 public:
  virtual ~service_18Null() {}
  void rpc_18(const std::map<std::string, std::string> & /* carrier */) {
    return;
  }
};

typedef struct _service_18_rpc_18_args__isset {
  _service_18_rpc_18_args__isset() : carrier(false) {}
  bool carrier :1;
} _service_18_rpc_18_args__isset;

class service_18_rpc_18_args {
 public:

  service_18_rpc_18_args(const service_18_rpc_18_args&);
  service_18_rpc_18_args& operator=(const service_18_rpc_18_args&);
  service_18_rpc_18_args() {
  }

  virtual ~service_18_rpc_18_args() throw();
  std::map<std::string, std::string>  carrier;

  _service_18_rpc_18_args__isset __isset;

  void __set_carrier(const std::map<std::string, std::string> & val);

  bool operator == (const service_18_rpc_18_args & rhs) const
  {
    if (!(carrier == rhs.carrier))
      return false;
    return true;
  }
  bool operator != (const service_18_rpc_18_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const service_18_rpc_18_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class service_18_rpc_18_pargs {
 public:


  virtual ~service_18_rpc_18_pargs() throw();
  const std::map<std::string, std::string> * carrier;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _service_18_rpc_18_result__isset {
  _service_18_rpc_18_result__isset() : se(false) {}
  bool se :1;
} _service_18_rpc_18_result__isset;

class service_18_rpc_18_result {
 public:

  service_18_rpc_18_result(const service_18_rpc_18_result&);
  service_18_rpc_18_result& operator=(const service_18_rpc_18_result&);
  service_18_rpc_18_result() {
  }

  virtual ~service_18_rpc_18_result() throw();
  ServiceException se;

  _service_18_rpc_18_result__isset __isset;

  void __set_se(const ServiceException& val);

  bool operator == (const service_18_rpc_18_result & rhs) const
  {
    if (!(se == rhs.se))
      return false;
    return true;
  }
  bool operator != (const service_18_rpc_18_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const service_18_rpc_18_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _service_18_rpc_18_presult__isset {
  _service_18_rpc_18_presult__isset() : se(false) {}
  bool se :1;
} _service_18_rpc_18_presult__isset;

class service_18_rpc_18_presult {
 public:


  virtual ~service_18_rpc_18_presult() throw();
  ServiceException se;

  _service_18_rpc_18_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class service_18Client : virtual public service_18If {
 public:
  service_18Client(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  service_18Client(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
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
  void rpc_18(const std::map<std::string, std::string> & carrier);
  void send_rpc_18(const std::map<std::string, std::string> & carrier);
  void recv_rpc_18();
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class service_18Processor : public ::apache::thrift::TDispatchProcessor {
 protected:
  ::apache::thrift::stdcxx::shared_ptr<service_18If> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (service_18Processor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_rpc_18(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  service_18Processor(::apache::thrift::stdcxx::shared_ptr<service_18If> iface) :
    iface_(iface) {
    processMap_["rpc_18"] = &service_18Processor::process_rpc_18;
  }

  virtual ~service_18Processor() {}
};

class service_18ProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  service_18ProcessorFactory(const ::apache::thrift::stdcxx::shared_ptr< service_18IfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::apache::thrift::stdcxx::shared_ptr< service_18IfFactory > handlerFactory_;
};

class service_18Multiface : virtual public service_18If {
 public:
  service_18Multiface(std::vector<apache::thrift::stdcxx::shared_ptr<service_18If> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~service_18Multiface() {}
 protected:
  std::vector<apache::thrift::stdcxx::shared_ptr<service_18If> > ifaces_;
  service_18Multiface() {}
  void add(::apache::thrift::stdcxx::shared_ptr<service_18If> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void rpc_18(const std::map<std::string, std::string> & carrier) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->rpc_18(carrier);
    }
    ifaces_[i]->rpc_18(carrier);
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class service_18ConcurrentClient : virtual public service_18If {
 public:
  service_18ConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  service_18ConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
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
  void rpc_18(const std::map<std::string, std::string> & carrier);
  int32_t send_rpc_18(const std::map<std::string, std::string> & carrier);
  void recv_rpc_18(const int32_t seqid);
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