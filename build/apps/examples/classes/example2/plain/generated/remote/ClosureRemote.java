/*******************************************************************************
 * Copyright (c) 2021 Peraton Labs, Inc  - All Rights Reserved.
 * Proprietary and confidential. Unauthorized copy or use of this file, via
 * any medium or mechanism is strictly prohibited. 
 *    
 * @author tchen
 *
 *******************************************************************************/
package com.peratonlabs.closure.remote;

import java.rmi.Remote;
import java.rmi.RemoteException;

public interface ClosureRemote extends Remote
{
    public void instantiate(int oid, String fqcn, Class<?>[] argTypes, Object[] args) throws RemoteException;
    
    public void finalize(int oid, String fqcn) throws RemoteException;
    
    public Object invokeMethod(int oid, String fqcn, String methodName, Class<?>[] argTypes, Object[] args) throws RemoteException;
    
    public Object readField(int oid, String fqcn, String fieldName) throws RemoteException;
    
    public void writeField(int oid, String fqcn, String fieldName, Object value) throws RemoteException;
    
    public Object readStaticField(String fqcn, String fieldName) throws RemoteException;
    
    public void writeStaticField(String fqcn, String fieldName, Object value) throws RemoteException;
}
