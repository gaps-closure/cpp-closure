# Program graph specification for C++

## Notes from 11/7 Meeting
1. Organize functions, methods, constructors, destructors together? (so that parameters and returns will apply to both)
2. Unclear whether we need virtual or not.
3. Need to have abstract methods subtype?
   - These will not have a body in the LLVM IR 
   - Whether to to exclude this from the PDG is TBD, we're going to keep for now. 
4. Need to include static methods 
5. Direct and indirect (virtual) method invocation 
6. Might make sense to subtype Record.Field/Method by whether it is static or not.  
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
| Stmt.Return      | A return within a given function                                                            | From AST |
| Stmt.Other       | An clang statement that does not transfer control to another function.                      | From AST |

### Edges

| Edge Name                     | Edge Description                                                                        | Source Type | Destination Type | Notes                             |
| ----------------------------- | --------------------------------------------------------------------------------------- | ----------- | ---------------- | --------------------------------- |
| Record.Field                  | Connects a record to its field                                                          | Decl.Record | Decl.Field       | From AST                          |
| Record.Method                 | Connects a record to its method                                                         | Decl.Record | Decl.Method      | From AST                          |
| Record.Constructor            | Connects a record to its constructor                                                    | Decl.Record | Decl.Constructor | From AST                          |
| Record.Destructor             | Connects a record to its destructor                                                     | Decl.Record | Decl.Destructor  | From AST                          |
| Record.Inherit                | Connects two records by inheritance relation. Type of inheritance given by a property   | Decl.Record | Decl.Record      | From AST                          |
| Control.Return                | A function/method/constructor/destructor return                                         | Stmt.Return | Stmt.Call        | From AST                          |
| Control.Entry                 | Connects a function-like object to it's body                                            | Decl        | Stmt             | From AST                          |
| Control.FunctionInvocation    | A direct call invocation to a C-style function                                          | Stmt.Call   | Decl.Function    | From AST                          |
| Control.MethodInvocation      | A method call                                                                           | Stmt.Call   | Decl.Method      | From AST                          |
| Control.ConstructorInvocation | A constructor call                                                                      | Stmt.Call   | Decl.Constructor | From AST                          |
| Control.DestructorInvocation  | A call to a destructor                                                                  | Stmt.Call   | Decl.Destructor  | From AST                          |
| Data.PointsTo                 | A points-to relation                                                                    | Decl        | Decl             | From AST + SVF                    |
| Data.DefUse                   | A def-use relation between statements                                                   | Stmt.Decl   | Stmt.Ref         | From AST + SVF (for indirect use) |
| Data.ArgPass                  | An argument pass to a call instruction                                                  | Stmt.Call   | Stmt             | From AST                          |
| Data.Object                   | Connects a method call to the object being called upon (e.g. f as in f.foo())           | Stmt.Call   | Stmt             | From AST                          |
| Data.Param                    | Connects a function-like object to its parameters                                       | Decl        | Decl.Param       | From AST                          |
| Data.FieldAccess              | A field access                                                                          | Stmt        | Decl.Field       | From AST                          |
| Data.Decl                     | Connects a decl statment to a decl                                                      | Stmt.Decl   | Decl             | From AST                          |
| Child                         | When a statement has a child not described by the above. Can be pruned out of the graph | Stmt        | Stmt             | From AST                          |