/*******************************************************************************
 *
 *******************************************************************************/
#include <string>
#include <map>
#include <iostream>

#include "codec.h"
// #include "ExtraShadow.hpp"

// std::map<void *, int> instanceMap;

using namespace std;

class ClosureRemoteHalMaster
{
private:
    static int nextOid;

public:
    static int instantiateExtra() { /// int oid, String fqcn, Class<?>[] argTypes, Object[] args) {
        uint32_t oid = nextOid++;
        
        request_extraconstructor_datatype data;
        request_extraconstructor_output serialized;
        size_t len_out;
        data.oid = oid;
        request_extraconstructor_data_encode(&serialized, &data, &len_out);

        cout << "RPC instantiateExtra" <<  serialized.trailer.seq << endl;
         
        // String inUri = IPC.getInUri();
        // String outUri = IPC.getOutUri();
        // HalZmq halzmq = new HalZmq(outUri, inUri, outTag);
        // halzmq.xdc_asyn_send((char *)payload[0], outTag);
        // halzmq.shutdown();

        return oid;
    }
    
    static int invokeExtraGetValue(int oid) {
        request_extragetvalue_datatype data;
        request_extragetvalue_output   serialized;
        size_t len_out;
        data.oid = oid;

        request_extragetvalue_data_decode(&serialized, &data, &len_out);

        // GapsTag tag; //  = RPCObjectTag.lookup(obj.getUid());
        int tag = 0;
        cout << "RPC instantiateExtra" <<  serialized.trailer.seq << endl;
         
        // String inUri = IPC.getInUri();
        // String outUri = IPC.getOutUri();
        // HalZmq halzmq = new HalZmq(outUri, inUri, outTag);
        // halzmq.xdc_asyn_send((char *)payload[0], outTag);

        // SDH sdh = halzmq.xdc_blocking_recv();
        
        // return getValue.value();
        return 0;
    }
};