/*******************************************************************************
 *
 *******************************************************************************/
#include <string>
#include <map>

#include "../example2.pb.h"
#include "../../Extra.hpp"

using namespace std;

std::map<void *, int> instanceMap;

class ClosureRemoteHalMaster
{
private:
    static int nextOid;

public:
    static void instantiateExtra() { /// int oid, String fqcn, Class<?>[] argTypes, Object[] args) {
        uint32_t oid = nextOid++;

        closure::ClosureMessage message;
        closure::ExtraConstructor *obj = new closure::ExtraConstructor();
        obj->set_oid(oid);
        message.set_allocated_extra_constructor(obj);

        string buf;
        message.SerializeToString(&buf);

        // GapsTag tag; //  = RPCObjectTag.lookup(obj.getUid());
        int tag = 0;
        cout << "RPC instantiateExtra" <<  tag << endl;
         
        // String inUri = IPC.getInUri();
        // String outUri = IPC.getOutUri();
        // HalZmq halzmq = new HalZmq(outUri, inUri, outTag);
        // halzmq.xdc_asyn_send((char *)payload[0], outTag);
        // halzmq.shutdown();
    }
    
    static int invokeExtraGetValue(int oid) {
        closure::ClosureMessage message;
        closure::ExtraGetValue *obj = new closure::ExtraGetValue();
        obj->set_oid(oid);
        message.set_allocated_extra_getvalue(obj);

        string buf;
        message.SerializeToString(&buf);

        // GapsTag tag; //  = RPCObjectTag.lookup(obj.getUid());
        int tag = 0;
        cout << "RPC invokeExtraGetValue" <<  tag << endl;
         
        // String inUri = IPC.getInUri();
        // String outUri = IPC.getOutUri();
        // HalZmq halzmq = new HalZmq(outUri, inUri, outTag);
        // halzmq.xdc_asyn_send((char *)payload[0], outTag);

        // SDH sdh = halzmq.xdc_blocking_recv();
        
        string buf; // (data.begin(), data.end());
        auto retMmessage = std::make_unique<closure::ClosureMessage>();
        retMmessage->ParseFromString(buf);

        if (!retMmessage->has_extra_getvalue_ret()) {
            cerr << "unexpected return message" << endl;
            return INT_MIN; // TODO
        }
        const closure::ExtraGetValueRet &getValue = retMmessage->extra_getvalue_ret();
        return getValue.value();
    }
};