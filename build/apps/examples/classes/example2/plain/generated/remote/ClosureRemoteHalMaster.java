/*******************************************************************************
 * Copyright (c) 2021 Peraton Labs, Inc  - All Rights Reserved.
 * Proprietary and confidential. Unauthorized copy or use of this file, via
 * any medium or mechanism is strictly prohibited. 
 *    
 * @author tchen
 *
 *******************************************************************************/
package com.peratonlabs.closure.remote;

import com.peratonlabs.closure.codegen.rpc.GapsTag;
import com.peratonlabs.closure.codegen.rpc.HalZmq;
import com.peratonlabs.closure.codegen.rpc.IPC;
import com.peratonlabs.closure.codegen.rpc.SDH;
import com.peratonlabs.closure.codegen.rpc.coder.Serializer;
import com.peratonlabs.closure.codegen.rpc.serialization.RPCConstructor;
import com.peratonlabs.closure.codegen.rpc.serialization.RPCField;
import com.peratonlabs.closure.codegen.rpc.serialization.RPCMethod;
import com.peratonlabs.closure.codegen.rpc.serialization.RPCObject;
import com.peratonlabs.closure.codegen.rpc.serialization.RPCObjectTag;

public class ClosureRemoteHalMaster implements ClosureRemote
{
    private Object rpc(Object obj, boolean oneway, GapsTag outTag) {
        System.out.println("RPC " + outTag.toString());
         
        byte[] payload = Serializer.serialize(obj);
        
        String inUri = IPC.getInUri();
        String outUri = IPC.getOutUri();
        HalZmq halzmq = new HalZmq(outUri, inUri, outTag);
            
        halzmq.xdc_asyn_send(payload, outTag);
        
        Object rtn = null;
        if (!oneway) {
            SDH sdh = halzmq.xdc_blocking_recv();
            
            rtn = Serializer.deserialize(sdh.getData());
            System.out.println("returned value: " + rtn);
        }
        halzmq.shutdown();
        
        System.out.println("-------------------------------");
        return rtn;
    }
    
    public void instantiate(int oid, String fqcn, Class<?>[] argTypes, Object[] args) {
        RPCObject obj = new RPCConstructor(oid, fqcn, argTypes, args, 0);
        GapsTag tag = RPCObjectTag.lookup(obj.getUid());
        
        rpc(obj, true, tag);
    }
    
    public void finalize(int localId, String fqcn) {
    }

    public Object invokeMethod(int oid, String fqcn, String methodName, Class<?>[] argTypes, Object[] args) {
        RPCMethod obj = new RPCMethod(oid, fqcn, methodName, argTypes, args);
        GapsTag tag = RPCObjectTag.lookup(obj.getUid());
        
        return rpc(obj, false, tag);
    }

    public Object readField(int oid, String fqcn, String fieldName) {
        RPCField obj = new RPCField(oid, fqcn, fieldName, null,  RPCObject.FLAG_READ);
        GapsTag tag = RPCObjectTag.lookup(obj.getUid());
        
        return rpc(obj, false, tag);
    }
    
    public void writeField(int oid, String fqcn, String fieldName, Object value) {
        RPCField obj = new RPCField(oid, fqcn, fieldName, value,  RPCObject.FLAG_WRITE);
        GapsTag tag = RPCObjectTag.lookup(obj.getUid());
        
        rpc(obj, true, tag);
    }
    
    public Object readStaticField(String fqcn, String fieldName) {
        RPCField obj = new RPCField(-1, fqcn, fieldName, null, RPCObject.FLAG_READ | RPCObject.FLAG_STATIC);
        GapsTag tag = RPCObjectTag.lookup(obj.getUid());
        
        return rpc(obj, false, tag);
    }
    
    public void writeStaticField(String fqcn, String fieldName, Object value) {
        RPCField obj = new RPCField(-1, fqcn, fieldName, value,  RPCObject.FLAG_WRITE | RPCObject.FLAG_STATIC);
        GapsTag tag = RPCObjectTag.lookup(obj.getUid());

        rpc(obj, true, tag);
    }
}