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

