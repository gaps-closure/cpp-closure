/*******************************************************************************
 * Copyright (c) 2021 Peraton Labs, Inc  - All Rights Reserved.
 * Proprietary and confidential. Unauthorized copy or use of this file, via
 * any medium or mechanism is strictly prohibited. 
 *    
 * @author tchen
 *
 *******************************************************************************/
package com.peratonlabs.closure.remote;

import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.HashMap;

import org.aspectj.lang.reflect.ConstructorSignature;
import org.aspectj.lang.reflect.MethodSignature;

public class ClosureShadow
{
    public static final String myEnclave = "Orange";

    private static int nextId = 0;
    private static HashMap<Class<?>, HashMap<Object, Integer>> instanceMap = new HashMap<Class<?>, HashMap<Object, Integer>> ();
    
    private static final int NO_REMOTE = -1;
    private static final int NO_SUCH_OBJECT = -2;
    
    public enum RemoteType {
        RMI,
        HAL
    }
    
    private static RemoteType remoteType = RemoteType.HAL;
    
    private static HashMap<Object, Integer> getMap(Object obj) {
        Class<?> clazz = obj.getClass();
        
        return instanceMap.get(clazz);
    }
    
    private static int getId(HashMap<Object, Integer> instanceMap, Object obj) {
        Integer remoteId = instanceMap.get(obj);
        if (remoteId == null) {
            return NO_SUCH_OBJECT;
        }
        return remoteId;
    }
    
    public static ClosureRemote getRemote() {
        if (remoteType == RemoteType.HAL) {
            return new ClosureRemoteHalMaster();
        }
        else if (remoteType == RemoteType.RMI) {
            try {
                Registry registry = LocateRegistry.getRegistry(null);
                return (ClosureRemote) registry.lookup("ClosureRemote");
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }
        return null;
    }
    
    public static int instantiate(Object obj, ConstructorSignature signature, Object[] args) {
        System.out.println(remoteType + ": construct " + signature.getDeclaringTypeName());

        HashMap<Object, Integer> remoteMap = getMap(obj);
        if (remoteMap == null) {
            remoteMap = new HashMap<Object, Integer>();
            instanceMap.put(obj.getClass(), remoteMap);
        }
        
        int remoteId = getId(remoteMap, obj);
        if (remoteId != NO_SUCH_OBJECT) {
            System.err.println("object already exists: " + obj);
            return remoteId;
        }

        try {
            remoteId = ++nextId;
            remoteMap.put(obj, remoteId);
            
            ClosureRemote rmi = getRemote();
            if (rmi == null) {
                return NO_REMOTE;
            }
            
            // MethodSignature is not serializable
            String fqcn = signature.getDeclaringTypeName();
            Class<?>[] argTypes = signature.getParameterTypes(); 
            
            rmi.instantiate(remoteId, fqcn, argTypes, args);
        }
        catch (Exception e) {
            System.err.println("Client exception: " + e.toString());
            e.printStackTrace();
        }
        return remoteId;
    }
    
    public static void finalize(Object obj) {
        System.out.println(remoteType + ": finalize " + obj);

        HashMap<Object, Integer> remoteMap = getMap(obj);
        if (remoteMap == null) {
            System.err.println("finalize: map does not exist: " + obj);
            return;
        }
        
        int remoteId = getId(remoteMap, obj);
        if (remoteId == NO_SUCH_OBJECT) {
            System.err.println("object does not exist: " + obj);
            return;
        }

        try {
            ClosureRemote rmi = getRemote();
            if (rmi == null) {
                return;
            }
            rmi.finalize(remoteId, obj.getClass().getName());
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public static Object invokeMethod(Object obj, MethodSignature signature, Object[] args) {
        boolean isStatic = (obj == null);
        System.out.println(remoteType + ": invoke " + (isStatic ? "static " : "") + "method " + signature.getName());

        int remoteId = -1;
        if (!isStatic) {
            HashMap<Object, Integer> remoteMap = getMap(obj);
            if (remoteMap == null) {
                System.err.println("invokeMethod: map does not exist: " + obj);
                return null;
            }

            remoteId = getId(remoteMap, obj);
            if (remoteId == NO_SUCH_OBJECT) {
                System.err.println("object does not exist: " + obj);
                return null;
            }
        }
        try {
            ClosureRemote rmi = getRemote();
            if (rmi == null) {
                return null;
            }
            // MethodSignature is not serializable
            String fqcn = signature.getDeclaringTypeName();
            String methodName = signature.getName();
            Class<?>[] argTypes = signature.getParameterTypes(); 
            
//            if (!XdccCheck.allowedMethod(myEnclave, fqcn, methodName, argTypes)) {
//                throw new ClosurePermissionException("Not allowed to invoke " + methodName + " from " + myEnclave);
//            }
            
            return rmi.invokeMethod(remoteId, fqcn, methodName, argTypes, args);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
    
    public static Object readField(Object obj, String fieldName) {
        System.out.println(remoteType + ": read field " + fieldName);

        HashMap<Object, Integer> remoteMap = getMap(obj);
        if (remoteMap == null) {
            System.err.println("readField: map does not exist: " + obj);
            return null;
        }
        
        int remoteId = getId(remoteMap, obj);
        if (remoteId == NO_SUCH_OBJECT) {
            System.err.println("object does not exist: " + obj);
            return null;
        }
        
        try {
            ClosureRemote rmi = getRemote();
            if (rmi == null) {
                return null;
            }
            String fqcn = obj.getClass().getName();
            return rmi.readField(remoteId, fqcn, fieldName);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
    
    public static void writeField(Object obj, String fieldName, Object value) {
        System.out.println(remoteType + ": write field " + fieldName + " to " + value);

        HashMap<Object, Integer> remoteMap = getMap(obj);
        if (remoteMap == null) {
            System.err.println("writeField: map does not exist: " + obj);
            return;
        }
        
        int remoteId = getId(remoteMap, obj);
        if (remoteId == NO_SUCH_OBJECT) {
            System.err.println("object does not exist: " + obj);
            return;
        }
        
        try {
            ClosureRemote rmi = getRemote();
            if (rmi == null) {
                return;
            }
            String fqcn = obj.getClass().getName();
            rmi.writeField(remoteId, fqcn, fieldName, value);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public static Object readStaticField(String className, String fieldName) {
        System.out.println(remoteType + ": read static field " + fieldName);
        
        try {
            ClosureRemote rmi = getRemote();
            if (rmi == null) {
                return null;
            }
            return rmi.readStaticField(className, fieldName);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
    
    public static void writeStaticField(String fqcn, String fieldName, Object value) {
        System.out.println(remoteType + ": write static field " + fieldName + " to " + value);

        try {
            ClosureRemote rmi = getRemote();
            if (rmi == null) {
                return;
            }
            rmi.writeStaticField(fqcn, fieldName, value);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static RemoteType getRemoteType() {
        return remoteType;
    }
}
