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

public class ClosureRemoteHalSlave
{
    private ClosureRemoteRMI rmi = new ClosureRemoteRMI();

    public static void init() {
        ClosureRemoteHalSlave slave = new ClosureRemoteHalSlave();
        slave.listen();
    }
    
    private void listen() {
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
            System.out.println("-------------------------------");
        }
    }
    
    public void instantiate(RPCConstructor o) {
        rmi.instantiate(o.getOid(), o.getFqcn(), o.getArgTypes(), o.getArgs());
    }
    
    public void finalize(int localId, String fqcn) {
    }

    public Object invokeMethod(RPCMethod o) {
        return rmi.invokeMethod(o.getOid(), o.getFqcn(), o.getMethodName(), o.getArgTypes(), o.getArgs());
    }

    public Object readField(RPCField o) {
        return rmi.readField(o.getOid(), o.getFqcn(), o.getFieldName());
    }
    
    public void writeField(RPCField o) {
        rmi.writeField(o.getOid(), o.getFqcn(), o.getFieldName(), o.getValue());
    }
    
    public Object readStaticField(RPCField o) {
        return rmi.readStaticField(o.getFqcn(), o.getFieldName());
    }
    
    public void writeStaticField(RPCField o) {
        rmi.writeStaticField(o.getFqcn(), o.getFieldName(), o.getValue());
    }
}