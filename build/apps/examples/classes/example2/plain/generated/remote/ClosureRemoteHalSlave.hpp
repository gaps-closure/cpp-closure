// #include ".hpp"

// generated
class ClosureRemoteHalSlave
{

public:
  ClosureRemoteHalSlave() {
  }

  static void init() {
      ClosureRemoteHalSlave slave;
      slave.listen();
  }

private:
  void listen() {
      while (true) {
          String inUri = IPC.getInUri();
          String outUri = IPC.getOutUri();
          GapsTag outTag = RPCObjectTag.lookup("response"); 
          HalZmq halzmq = new HalZmq(outUri, inUri, outTag); // outTag will be replaced later
          SDH sdh = halzmq.xdc_blocking_recv();

          RPCObject o = (RPCObject) Serializer.deserialize(sdh.getData());
          
          if (o instanceof RPCMethod) {
              RPCMethod method = (RPCMethod) o;
              Object obj = invokeMethod(method);
              byte[] adu = Serializer.serialize(obj);
              GapsTag tag = RPCObjectTag.lookup(method.getUidResponse());
              System.out.println("RPC response " + tag.toString());
              halzmq.xdc_asyn_send(adu, tag);
          }
          else if (o instanceof RPCField) {
              RPCField field = (RPCField) o;
              if (field.isWrite()) {
                  if (field.isStatic())
                      writeStaticField(field);
                  else
                      writeField(field);
              }
              else {
                  Object obj;
                  if (field.isStatic())
                      obj = readStaticField(field);
                  else
                      obj = readField(field);
              
                  byte[] adu = Serializer.serialize(obj);
                  GapsTag tag = RPCObjectTag.lookup(field.getUidResponse());
                  System.out.println("RPC response " + tag.toString());
                  halzmq.xdc_asyn_send(adu, tag);
              }
          }
          else if (o instanceof RPCConstructor) {
              RPCConstructor ctor = (RPCConstructor) o;
              instantiate(ctor);
          }
          else {
              System.err.println("unsupported " + o);
          }
          
          halzmq.shutdown();
      }
  }
};

