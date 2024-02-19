# Program graph specification for C++

## Notes from 11/7 Meeting
1. Organize functions, methods, constructors, destructors together? (so that parameters and returns will apply to both)
2. Unclear whether we need virtual or not.
3. Need to have abstract methods subtype?
   - These will not have a body in the LLVM IR 
   - Whether to to exclude this from the PDG is TBD, we're going to keep for now. 
4. Need to include static methods 
5. Direct and indirect (virtual) method invocation 
6. Might make sense to subtype Struct.Field/Method by whether it is static or not.  
7. Do we need class inheritance hierarchy?


## Graph Representation

This graph representation more closely aligns with the clang AST, and requires less mapping from 
SVF. The only information brought from SVF are the points-to edges. This information can be mapped
from the LLVM/SVF through use of the debug info.


### Nodes

| Node Name        | Node Description                                                                            | Notes    |
| ---------------- | ------------------------------------------------------------------------------------------- | -------- |
| Decl.Var         | All variable declarations. It will be marked with global/static property                    | From AST |
| Decl.Function    | A static global variable inside a function                                                  | From AST |
| Decl.Record      | A class, struct or union definition.                                                        | From AST |
| Decl.Field       | A record field.                                                                             | From AST |
| Decl.Method      | A record method. May have virtual/abstract property                                         | From AST |
| Decl.Param       | Formal parameter of a function/method. Contains information about parameter index           | From AST |
| Decl.Constructor | A constructor for a class/struct                                                            | From AST |
| Decl.Destructor  | A destructor for a class/struct                                                             | From AST |
| Stmt.Decl        | A statement which also has a declaration                                                    | From AST |
| Stmt.Call        | An clang statement that represents a call to a function, method, constructor or destructor. | From AST |
| Stmt.Compound    | An clang statement that only has other statements as children.                              | From AST |
| Stmt.Ref         | An clang statement which holds a reference to a decl                                        | From AST |
| Stmt.Field       | An clang statement which is a field accessor                                                | From AST |
| Stmt.This        | An clang statement which refers to 'this'                                                   | From AST |
| Stmt.Return      | A return within a given function                                                            | From AST |
| Stmt.Other       | An clang statement that does not transfer control to another function.                      | From AST |

### Edges

| Edge Name                     | Edge Description                                                                        | Source Type          | Destination Type | Notes          |
| ----------------------------- | --------------------------------------------------------------------------------------- | -------------------- | ---------------- | -------------- |
| Struct.Field                  | Connects a record to its field                                                          | Decl.Record          | Decl.Field       | From AST       |
| Struct.Method                 | Connects a record to its method                                                         | Decl.Record          | Decl.Method      | From AST       |
| Struct.Constructor            | Connects a record to its constructor                                                    | Decl.Record          | Decl.Constructor | From AST       |
| Struct.Destructor             | Connects a record to its destructor                                                     | Decl.Record          | Decl.Destructor  | From AST       |
| Struct.Inherit                | Connects two records by inheritance relation. Type of inheritance given by a property   | Decl.Record          | Decl.Record      | From AST       |
| Struct.Param                  | Connects a function-like object to its parameters                                       | Decl                 | Decl.Param       | From AST       |
| Struct.Child                  | When a statement has a child not described by the above. Can be pruned out of the graph | Stmt                 | Stmt             | From AST       |
| Control.Entry                 | Connects a function-like object to it's body                                            | Decl                 | Stmt             | From AST       |
| Control.FunctionInvocation    | A direct call invocation to a C-style function                                          | Stmt.Call            | Decl.Function    | From AST       |
| Control.MethodInvocation      | A method call                                                                           | Stmt.Call            | Decl.Method      | From AST       |
| Control.ConstructorInvocation | A constructor call                                                                      | Stmt.Call            | Decl.Constructor | From AST       |
| Control.DestructorInvocation  | A call to a destructor                                                                  | Stmt.Call            | Decl.Destructor  | From AST       |
| Data.DefUse                   | A def-use relation between statements                                                   | Stmt.Decl            | Stmt.Ref         | From AST + SVF |
| Data.ArgPass                  | An argument pass to a call instruction. Destination has a paramater index               | Stmt.Call            | Stmt             | From AST       |
| Data.Return                   | Connects a call to its resulting value at the call site                                 | Stmt.Call            | Stmt             | From AST       |
| Data.Object                   | Connects a method call/field to the object being called upon (e.g. f as in f.foo())     | Stmt.Call/Stmt.Field | Stmt             | From AST       |
| Data.FieldAccess              | A field access                                                                          | Stmt                 | Decl.Field       | From AST       |
| Data.InstanceOf               | Connects a 'this' instance to a class definition                                        | Stmt.This            | Decl.Record      | From AST       |
| Data.Decl                     | Connects a decl statment to a decl                                                      | Stmt.Decl            | Decl             | From AST       |
| Data.PointsTo                 | A points-to relation                                                                    | Decl                 | Decl             | From AST + SVF |

## Example snippet

```
class Foo()
    #cle ORANGE
    Bar a;
    method() {
        int c; // Nodes: Decl.Var for 'c'
        c = d; // Nodes: Stmt.Other for the assignment. 
               //        Children are Stmt.Ref to 'c', Stmt.Ref to 'd'. DefUse edge from ref to Decl.Var for 'd'.
        this.a.b = c;
    }
```

### Nodes

1. Stmt.Other node. Represents the entire assignment statement
2. Stmt.Field node. For the LHS, 'this.a.b'
3. Stmt.Field node. For 'this.a'
4. Stmt.This node. For 'this'
5. Decl.Field node for the class field 'Bar a' (gets ORANGE from annotation)
6. Decl.Field node for the class field 'b'
7. Stmt.Ref node for 'c'
8. Decl.Method node for method()
9. Decl.Record node for Foo

### Edges

  - Child edge between (1) and (2)
  - Child edge between (1) and (7)
  - Data.FieldAccess edge between (2) and (6)
  - Data.Object edge between (2) and (3)
  - Data.Object edge between (3) and (4)
  - Data.InstanceOf edge between (4) and (9)
  - Data.FieldAccess edge between (3) and (5)
  - Data.DefUse edge between (7) and the Decl.Var for `c`
  - Struct.Field edge from (9) to (5)
  - Struct.Method edge from (9) to (8)

## Headers for `nodes.csv` and `edges.csv`

| Node ID | Node Type | Debug Name | Annotation | Parent Decl ID | Parent Class ID | Parent Function ID | Param Index | Source Filename | Source Begin Offset | Source End Offset |
| ------- | --------- | ---------- | ---------- | -------------- | --------------- | ------------------ | ----------- | --------------- | ------------------- | ----------------- |

| Edge ID | Edge Type | Source Node ID | Edge Node ID |
| ------- | --------- | -------------- | ------------ |