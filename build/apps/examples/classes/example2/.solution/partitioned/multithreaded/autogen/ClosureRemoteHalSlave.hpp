#include <string>

#include "codec.h"
#include "Extra.hpp"

using namespace std;

std::map<int, void *> instanceMap;

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
            // String inUri = IPC.getInUri();
            // String outUri = IPC.getOutUri();
            // GapsTag outTag = RPCObjectTag.lookup("response"); 
            // HalZmq halzmq = new HalZmq(outUri, inUri, outTag); // outTag will be replaced later
            // SDH sdh = halzmq.xdc_blocking_recv();
            // sdh.getData()
            // 
            string buf; // (data.begin(), data.end());
            auto message = std::make_unique<closure::ClosureMessage>();
            message->ParseFromString(buf);

            if (message->has_extra_constructor()) {
                const closure::ExtraConstructor &ctor = message->extra_constructor();
                uint32_t oid = ctor.oid();

                void *obj = instanceMap[oid];
                if (obj != NULL) {
                    std::cerr << "Extra object already exists: " << oid << endl;
                }
                Extra *extra = new Extra();

                instanceMap[oid] = extra;
            }
            else if (message->has_extra_getvalue()) {
                const closure::ExtraGetValue &getValue = message->extra_getvalue();
                uint32_t oid = getValue.oid();

                void *obj = instanceMap[oid];
                if (obj == NULL) {
                    std::cerr << "Extra object does not exist: " << oid << endl;
                    return;
                }

                Extra *extra = (Extra *) obj;
                int value = extra->getValue();

                closure::ClosureMessage message;
                closure::ExtraGetValueRet *ret = new closure::ExtraGetValueRet();
                ret->set_oid(value);
                message.set_allocated_extra_getvalue_ret(ret);

                string buf;
                message.SerializeToString(&buf);

                // GapsTag tag = RPCObjectTag.lookup(method.getUidResponse());
                int tag = 0; // TODO
                std::cout << "RPC response " << tag << endl;
                // halzmq.xdc_asyn_send((char *)buf[0], tag);
            }    
            else if (message->has_extra_getvalue_ret()) {
            }
            // else if (o instanceof RPCField) {
            //     RPCField field = (RPCField) o;
            //     if (field.isWrite()) {
            //         if (field.isStatic())
            //             writeStaticField(field);
            //         else
            //             writeField(field);
            //     }
            //     else {
            //         Object obj;
            //         if (field.isStatic())
            //             obj = readStaticField(field);
            //         else
            //             obj = readField(field);
                
            //         byte[] adu = Serializer.serialize(obj);
            //         GapsTag tag = RPCObjectTag.lookup(field.getUidResponse());
            //         System.out.println("RPC response " + tag.toString());
            //         halzmq.xdc_asyn_send(adu, tag);
            //     }
            // }
            // halzmq.shutdown();
        }
    }
};

