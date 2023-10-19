/*******************************************************************************
 * Copyright (c) 2021 Peraton Labs, Inc  - All Rights Reserved.
 * Proprietary and confidential. Unauthorized copy or use of this file, via
 * any medium or mechanism is strictly prohibited. 
 *    
 * @author tchen
 *
 *******************************************************************************/
package com.peratonlabs.closure.remote;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.lang.reflect.Constructor;
import java.util.HashMap;

public class ClosureRemoteRMI implements ClosureRemote
{
    public static final String myEnclave = "##LEVEL##";
            
    // Integer is object ID
    private HashMap<String, HashMap<Integer, Object>> instanceMap = new HashMap<String, HashMap<Integer, Object>> ();

    public static void init() {
        try {
            ClosureRemoteRMI obj = new ClosureRemoteRMI();

            ClosureRemote stub = (ClosureRemote) UnicastRemoteObject.exportObject(obj, 0);

            Registry registry = LocateRegistry.getRegistry();
            registry.rebind("ClosureRemote", stub);

            System.err.println("HAL Ready");
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public void instantiate(int localId, String fqcn, Class<?>[] argTypes, Object[] args) {
        HashMap<Integer, Object> remoteMap = instanceMap.get(fqcn);
        if (remoteMap != null) {
            Object remoteId = remoteMap.get(localId);
            if (remoteId != null) {
                System.err.println("object already exits: " + fqcn + " " + localId);
            }
        }
        else {
            remoteMap = new HashMap<Integer, Object>();
            instanceMap.put(fqcn, remoteMap);
            System.out.println("object created " + fqcn + ", id:" + localId);
        }
        try {
            Class<?> clazz = Class.forName(fqcn);
            Constructor<?> cons = clazz.getConstructor(argTypes);
            Object obj = cons.newInstance(args);

            remoteMap.put(localId, obj);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public void finalize(int localId, String fqcn) {
        HashMap<Integer, Object> remoteMap = instanceMap.get(fqcn);
        if (remoteMap == null) {
            System.err.println("map does not exit: " + fqcn + " " + localId);
            return;
        }
        remoteMap.remove(new Integer(localId));
    }

    public Object invokeMethod(int oid, String fqcn, String methodName, Class<?>[] argTypes, Object[] args) {
        boolean isStatic = (oid == -1);
        
        Object obj = null;
        if (!isStatic) {
            HashMap<Integer, Object> remoteMap = instanceMap.get(fqcn);
            if (remoteMap == null) {
                System.err.println("object map not found: " + fqcn + " " + oid);
                return null;
            }
            obj = remoteMap.get(oid);
            if (obj == null) {
                System.err.println("object not found: oid=" + oid);
                return null;
            }
        }
        
        try {
            Class<?> clazz = Class.forName(fqcn);
            if (!isStatic && !clazz.isInstance(obj)) {
                System.err.println("object is not an instance of " + clazz);
                return null;
            }
            
            Object val;
            Method method;
            if (isStatic)
                method = clazz.getMethod(methodName, argTypes);
            else
                method = obj.getClass().getMethod(methodName, argTypes);
            if (method == null) {
                System.err.println("no such method: " + methodName);
                return null; 
            }
            if (isStatic)
                val = method.invoke(null, args);
            else
                val = method.invoke(obj, args);
            
            System.out.println((isStatic ? "static " : "") + "method invoked: " + methodName + " value = " + val);

            return val;
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public Object readField(int oid, String fqcn, String fieldName) {
        HashMap<Integer, Object> remoteMap = instanceMap.get(fqcn);
        if (remoteMap == null) {
            System.err.println("object map not found: " + fqcn + " " + oid);
            return null;
        }
        Object obj = remoteMap.get(oid);
        if (obj == null) {
            System.err.println("object not found: oid=" + oid);
            return null;
        }
        
        try {
            Class<?> clazz = Class.forName(fqcn);
            if (!clazz.isInstance(obj)) {
                System.err.println("object is not an instance of " + clazz);
                return null;
            }
            
            Field field = obj.getClass().getField(fieldName);
            if (field == null) {
                System.err.println("no such field: " + fieldName);
                return null; 
            }
            Object val = field.get(obj);
            
            System.out.println("read field " + fieldName + " value = " + val);
            return val;
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
    
    public void writeField(int oid, String fqcn, String fieldName, Object value) {
        HashMap<Integer, Object> remoteMap = instanceMap.get(fqcn);
        if (remoteMap == null) {
            System.err.println("object map not found: " + fqcn + " " + oid);
            return;
        }
        Object obj = remoteMap.get(oid);
        if (obj == null) {
            System.err.println("object not found: oid=" + oid);
            return;
        }
        
        try {
            Class<?> clazz = Class.forName(fqcn);
            if (!clazz.isInstance(obj)) {
                System.err.println("object is not an instance of " + clazz);
                return;
            }
            
            Field field = obj.getClass().getField(fieldName);
            if (field == null) {
                System.err.println("no such field: " + fieldName);
                return; 
            }
            field.set(obj, value);
            
            System.out.println("write field " + fieldName + " to " + value);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public Object readStaticField(String fqcn, String fieldName) {
        try {
            Class<?> clazz = Class.forName(fqcn);
            
            Field field = clazz.getField(fieldName);
            if (field == null) {
                System.err.println("no such field: " + fieldName);
                return null; 
            }
            Object val = field.get(null);
            
            System.out.println("read static field " + fieldName + " value = " + val);
            return val;
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
    
    public void writeStaticField(String fqcn, String fieldName, Object value) {
        try {
            Class<?> clazz = Class.forName(fqcn);
            
            Field field = clazz.getField(fieldName);
            if (field == null) {
                System.err.println("no such field: " + fieldName);
                return; 
            }
            field.set(null, value);
            
            System.out.println("set static field " + fieldName + " to " + value);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
}