# Program graph specification for C++

## Nodes

| Node Name                | Node Description                                                                            | Notes                           |
| ------------------------ | ------------------------------------------------------------------------------------------- | ------------------------------- |
| Var.Global               | True globals. Can be scoped to a namespace but not static inside class                      | From IR                         |
| Var.FunctionStaticGlobal | A static global variable inside a function                                                  | From IR + DebugInfo             |
| Var.ModuleStaticGlobal   | A global variable scoped to a single translation unit                                       | From IR + DebugInfo             |
| Var.RecordStaticGlobal   | A static variable within a class                                                            | From IR + DebugInfo or from AST |
| Instruction.Call         | An LLVM instruction that represents a call to a function, method, constructor or destructor | From IR                         |
| Instruction.Other        | An LLVM instruction that does not transfer control to another function.                     | From IR                         |
| Function                 | A function definition                                                                       | From IR                         |
| Param.Actual             | Actual parameter of a function or method call. Contains information about parameter index   | From IR + DebugInfo             |
| Param.Formal             | Formal parameter of a function/method. Contains information about parameter index           | From IR + DebugInfo             |
| Return                   | A return for a given function                                                               | From IR + DebugInfo             |
| Annotation               | A CLE annotation                                                                            | From IR                         |
| Record                   | A class, struct or union definition. Associated with an LLVM type definition                | From AST + IR + DebugInfo       |
| Field                    | A record field, with no directly associated LLVM                                            | From AST + IR + DebugInfo       |
| Method.Normal            | A non-virtual class/struct method.                                                          | From AST + IR + DebugInfo       |
| Method.Virtual           | A virtual class/struct method.                                                              | From AST + IR + DebugInfo       |
| Constructor              | A constructor for a class/struct                                                            | From AST + IR + DebugInfo       |
| Destructor               | A destructor for a class/struct                                                             | From AST + IR + DebugInfo       |


## Edges

| Edge Name                        | Edge Description                                                       | Source Type                            | Destination Type          | Notes                     |
| -------------------------------- | ---------------------------------------------------------------------- | -------------------------------------- | ------------------------- | ------------------------- |
| ControlDep.FunctionInvocation    | A direct call invocation to a C-style function                         | Instruction.Call                       | Function                  | From IR                   |
| ControlDep.MethodInvocation      | A method call                                                          | Instruction.Call                       | Method                    | From IR + AST + DebugInfo |
| ControlDep.ConstructorInvocation | A constructor call                                                     | Instruction.Call                       | Constructor               | From IR + AST + DebugInfo |
| ControlDep.DestructorInvocation  | A call to a destructor                                                 | Instruction.Call                       | Destructor                | From IR + AST + DebugInfo |
| ControlDep.Return                | A function/method/constructor/destructor return                        | Return                                 | Instruction.Call          | From IR                   |
| ControlDep.Entry                 | An instruction within a function/method/constructor/destructor         | Function/Method/Constructor/Destructor | Instruction               | From IR                   |
| DataDep.DefUse                   | A direct or indirect def-use. May be interprocedural                   | Instruction/Parameter/Var              | Instruction/Parameter/Var | From IR + SVF             |
| DataDep.PointsTo                 | A points-to relation                                                   | Instruction/Parameter/Var              | Instruction/Parameter/Var | From IR + SVF             |
| DataDep.Arg                      | Connects a call to its actual parameters                               | Instruction.Call                       | Param.Actual              | From IR                   |
| DataDep.ArgPass                  | An argument pass to a call instruction                                 | Instruction/Parameter/Var              | Param.Actual              | From IR                   |
| DataDep.Parameter                | Connects Actual -> Formal                                              | Param.Actual                           | Param.Formal              | From IR                   |
| DataDep.FieldAccess              | A field access                                                         | Instruction                            | Field                     | From IR + AST + DebugInfo |
| Annotation                       | Connects an annotation to where its applied                            | Annotation                             | All Nodes?                | From IR                   |
| Record.Field                     | Connects a record to its field                                         | Record                                 | Field                     | From AST                  |
| Record.Method                    | Connects a record to its method                                        | Record                                 | Method                    | From AST                  |
| Record.Method                    | Connects a record to its method                                        | Record                                 | Method                    | From AST                  |
| Record.Constructor               | Connects a record to its constructor                                   | Record                                 | Constructor               | From AST                  |
| Record.Destructor                | Connects a record to its destructor                                    | Record                                 | Destructor                | From AST                  |
| Record.Inherit.Normal            | Connects two records by inheritance relation                           | Record                                 | Record                    | From AST                  |
| Record.Inherit.Virtual           | Connects two records by a virtual inheritance relation                 | Record                                 | Record                    | From AST                  |
| Record.Virtual                   | Connects a method which virtually overrides another one in a sub class | Method.Virtual                         | Method.Virtual            | From AST                  |

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


# Alternative Graph Representation

This graph representation more closely aligns with the clang AST, and requires less mapping from 
SVF. The only information brought from SVF are the points-to edges. This information can be mapped
from the LLVM/SVF through use of the debug info.


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
| Stmt.Compound    | An clang statement that only has other statements as childern.                              | From AST |
| Stmt.Ref         | An clang statement which holds a reference to a decl                                        | From AST |
| Stmt.Other       | An clang statement that does not transfer control to another function.                      | From AST |
| Stmt.Return      | A return within a given function                                                            | From AST |
| Annotation       | A CLE annotation                                                                            | From AST |


## Edges

| Edge Name                     | Edge Description                                                                                   | Source Type | Destination Type | Notes          |
| ----------------------------- | -------------------------------------------------------------------------------------------------- | ----------- | ---------------- | -------------- |
| Record.Field                  | Connects a record to its field                                                                     | Decl.Record | Decl.Field       | From AST       |
| Record.Method                 | Connects a record to its method                                                                    | Decl.Record | Decl.Method      | From AST       |
| Record.Constructor            | Connects a record to its constructor                                                               | Decl.Record | Decl.Constructor | From AST       |
| Record.Destructor             | Connects a record to its destructor                                                                | Decl.Record | Decl.Destructor  | From AST       |
| Record.Inherit                | Connects two records by inheritance relation. Type of inheritance given by a property              | Decl.Record | Decl.Record      | From AST       |
| Control.Return                | A function/method/constructor/destructor return                                                    | Stmt.Return | Stmt.Call        | From AST       |
| Control.Entry                 | Connects a function-like object to it's body                                                       | Decl        | Stmt             | From AST       |
| Control.FunctionInvocation    | A direct call invocation to a C-style function                                                     | Stmt.Call   | Decl.Function    | From AST       |
| Control.MethodInvocation      | A method call                                                                                      | Stmt.Call   | Decl.Method      | From AST       |
| Control.ConstructorInvocation | A constructor call                                                                                 | Stmt.Call   | Decl.Constructor | From AST       |
| Control.DestructorInvocation  | A call to a destructor                                                                             | Stmt.Call   | Decl.Destructor  | From AST       |
| Data.PointsTo                 | A points-to relation                                                                               | Decl        | Decl             | From AST + SVF |
| Data.DefUse                   | A def-use relation between statements                                                              | Stmt.Decl   | Stmt.Ref         | From AST       |
| Data.ArgPass                  | An argument pass to a call instruction                                                             | Stmt.Call   | Stmt             | From AST       |
| Data.Object                   | Connects a method call to the object being called upon (e.g. f as in f.foo())                      | Stmt.Call   | Stmt             | From AST       |
| Data.Param                    | Connects a function-like object to its parameters                                                  | Decl        | Decl.Param       | From AST       |
| Data.FieldAccess              | A field access                                                                                     | Stmt        | Decl.Field       | From AST       |
| Data.Decl                     | Connects a decl statment to a decl                                                                 | Stmt.Decl   | Decl             | From AST       |
| Child                         | When a statement has a child not described by the above. Mostly used for compound/other statements | Stmt        | Stmt             | From AST       |