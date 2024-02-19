%%%%%%%%%%%%%%%
% Graph nodes %
%%%%%%%%%%%%%%%

set of int: Decl_Var;         % A variable declaration. It will be marked with global/static property
set of int: Decl_Function;    % A static global variable inside a function
set of int: Decl_Record;      % A class, struct or union definition
set of int: Decl_Field;       % A record field
set of int: Decl_Method;      % A record method. May have virtual/abstract property
set of int: Decl_Param;       % A formal parameter of a function/method. Contains information about parameter index
set of int: Decl_Constructor; % A constructor for a class/struct
set of int: Decl_Destructor;  % A destructor for a class/struct

set of int: Stmt_Decl;        % A statement which also has a declaration
set of int: Stmt_Call;        % A clang statement that represents a call to a function, method, constructor or destructor
set of int: Stmt_Compound;    % A clang statement that only has other statements as children
set of int: Stmt_Ref;         % A clang statement which holds a reference to a decl
set of int: Stmt_Field;       % A clang statement which is a field accessor
set of int: Stmt_This;        % A clang statement which refers to 'this'
set of int: Stmt_Return;      % A return within a given function
set of int: Stmt_Other;       % A clang statement that does not transfer control to another function

set of int: NodeIdx; % Spans all nodes

% Aggregate to unify constraints on functions, constructors, and methods
set of int: FunOrMethodOrCon = Decl_Function union Decl_Constructor union Decl_Method union Decl_Destructor;

%%%%%%%%%%%%%%%
% Graph edges %
%%%%%%%%%%%%%%%

set of int: Record_Field;                  % Connects a record to its field                           | Decl.Record -> Decl.Field
set of int: Record_Method;                 % Connects a record to its method                          | Decl.Record -> Decl.Method
set of int: Record_Constructor;            % Connects a record to its constructor                     | Decl.Record -> Decl.Constructor
set of int: Record_Destructor;             % Connects a record to its destructor                      | Decl.Record -> Decl.Destructor 
set of int: Record_Inherit;                % Connects two records by inheritance relation             | Decl.Record -> Decl.Record

set of int: Control_Return;                % A function/method/constructor/destructor return          | Stmt.Return -> Stmt.Call
set of int: Control_Entry;                 % Connects a function-like object to it's body             | Decl -> Stmt

set of int: Control_FunctionInvocation;    % A direct call invocation to a C-style function           | Stmt.Call -> Decl.Function
set of int: Control_MethodInvocation;      % A method call                                            | Stmt.Call -> Decl.Method
set of int: Control_ConstructorInvocation; % A constructor call                                       | Stmt.Call -> Decl.Constructor
set of int: Control_DestructorInvocation;  % A call to a destructor                                   | Stmt.Call -> Decl.Destructor

set of int: Data_PointsTo;                 % A points-to relation                                     | Decl -> Decl
set of int: Data_DefUse;                   % A def-use relation between statements                    | Stmt.Decl -> Stmt.Ref
set of int: Data_ArgPass;                  % An argument pass to a call instruction                   | Stmt.Call -> Stmt
set of int: Data_Return;                   % Connects data being returned to the call site            | Stmt -> Stmt.Call
set of int: Data_Object;                   % Connects a method call/field to the object being invoked | Stmt.Call/Stmt.Field -> Stmt
set of int: Data_FieldAccess;              % A field access                                           | Stmt -> Decl.Field
set of int: Data_InstanceOf;               % Connects a 'this' instance to a class definition         | Stmt.This -> Decl.Record
set of int: Data_Decl;                     % Connects a decl statment to a decl                       | Stmt.Decl -> Decl
set of int: Data_Child;                    % When a statement has a child not described by the above  | Stmt -> Stmt

set of int: EdgeIdx;  % Spans all data and control edges

set of int: DataEdge =
    Data_PointsTo union Data_DefUse union Data_ArgPass union
    Data_Return union Data_Object union Data_FieldAccess union
    Data_InstanceOf union Data_Decl union Data_Child;

% Aggregates of enclave-safe and invocation edges
set of int: Control_EnclaveSafe =
    Record_Field union Record_Method union Record_Constructor union
    Record_Destructor union Record_Inherit union Control_Entry;

set of int: Control_Invocation = 
    Control_FunctionInvocation union Control_MethodInvocation union
    Control_ConstructorInvocation union Control_DestructorInvocation;

set of int: Data_EnclaveSafe =
    Data_Child union Data_FieldAccess union Data_InstanceOf union
    Data_Object union Data_DefUse union Data_Decl union Data_ArgPass;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Containment mappings for nodes, edge endpoints, parameter indices, and node properties %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

array[NodeIdx]  of bool: isGlobal;
array[NodeIdx]  of int:  hasClass; % 0 for node not in a class, otherwise maps to the Decl_Record containing the node

array[NodeIdx]  of bool: userAnnotatedNode;

array[NodeIdx]  of int:  hasFunction; % 0 for globals, otherwise maps to the Decl_Function/Method/Constructor containing the node
array[EdgeIdx]  of int:  hasSource;
array[EdgeIdx]  of int:  hasDest;
array[NodeIdx]  of int:  hasParamIdx;

% Note: if n is a Decl_Record (i.e. a class declaration), then hasClass[n] == 0 if the class is not contained in another class,
% but if n is a Decl_Function then hasFunction[n] == n (i.e. a function contains itself but a class does not)

%%%%%%%%%%%%%%%%%%%
% CLE input model %
%%%%%%%%%%%%%%%%%%%

enum Level;
enum Enclave;
array[Enclave] of Level: hasEnclaveLevel;

enum cleLabel;
enum cdf;
enum GuardOperation = {nullGuardOperation, allow, deny, redact};
enum Direction      = {nullDirection, bidirectional, egress, ingress};

int: MaxFnParams; % Upper bound on number of parameters for any function in the program
set of int: paramIdx = 1..MaxFnParams;

array[cleLabel]                of Level:          hasLabelLevel;
array[cleLabel]                of bool:           isFunctionAnnotation;
array[cdf]                     of cleLabel:       fromCleLabel;
array[cdf]                     of Level:          hasRemotelevel;
array[cdf]                     of GuardOperation: hasGuardOperation;
array[cdf]                     of Direction:      hasDirection;
array[cdf]                     of bool:           isOneway;
array[cleLabel, Level]         of cdf:            cdfForRemoteLevel;

array[cdf, cleLabel]           of bool:           hasRettaints;
array[cdf, cleLabel]           of bool:           hasCodtaints;
array[cdf, paramIdx, cleLabel] of bool:           hasArgtaints;
array[cdf, cleLabel]           of bool:           hasARCtaints;

%%%%%%%%%%%%%%%%%%%%%%
% Decision variables %
%%%%%%%%%%%%%%%%%%%%%%

array[NodeIdx] of var Enclave:  nodeEnclave;
array[NodeIdx] of var cleLabel: taint;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Utility functions and predicates %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

predicate isFunction(NodeIdx: n)    = (n in FunOrMethodOrCon);
predicate allowOrRedact(var cdf: c) = (hasGuardOperation[c] == allow \/ hasGuardOperation[c] == redact);

predicate sourceAnnotFun(EdgeIdx: e) =
    (if hasFunction[hasSource[e]] != 0 then userAnnotatedNode[hasFunction[hasSource[e]]] else false endif);

predicate destAnnotFun(EdgeIdx: e) =
    (if hasFunction[hasDest[e]] != 0 then userAnnotatedNode[hasFunction[hasDest[e]]] else false endif);

predicate isInArctaint(var cleLabel: fan, var cleLabel: tnt, var Level: lvl) =
    (if isFunctionAnnotation[fan] then hasARCtaints[cdfForRemoteLevel[fan, lvl], tnt] else false endif);

predicate xdedge(EdgeIdx: e) = nodeEnclave[hasSource[e]] != nodeEnclave[hasDest[e]];

function var cleLabel: esTaint(EdgeIdx: e) = taint[hasSource[e]];
function var cleLabel: edTaint(EdgeIdx: e) = taint[hasDest[e]];

function var cleLabel: esFunTaint(EdgeIdx: e) = if sourceAnnotFun(e) then taint[hasFunction[hasSource[e]]] else nullCleLabel endif;
function var cleLabel: edFunTaint(EdgeIdx: e) = if destAnnotFun(e)   then taint[hasFunction[hasDest[e]]]   else nullCleLabel endif;

function var cdf: esFunCdf(EdgeIdx: e) = if sourceAnnotFun(e) then cdfForRemoteLevel[esFunTaint(e), hasLabelLevel[edTaint(e)]] else nullCdf endif;
function var cdf: edFunCdf(EdgeIdx: e) = if destAnnotFun(e)   then cdfForRemoteLevel[edFunTaint(e), hasLabelLevel[esTaint(e)]] else nullCdf endif;

function var cleLabel: ftaint(NodeIdx: n) = if hasFunction[n] != 0 then taint[hasFunction[n]] else nullCleLabel endif;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Structural constraints on node enclave/level %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Nodes must have a non-null enclave which is shared within a function and matches their level
constraint :: "NodeHasEnclave"            forall (n in NodeIdx) (nodeEnclave[n] != nullEnclave);
constraint :: "NodeEnclaveIsFunEnclave"   forall (n in NodeIdx) ((hasFunction[n] != 0) -> (nodeEnclave[n] == nodeEnclave[hasFunction[n]]));
constraint :: "NodeEnclaveIsClassEnclave" forall (n in NodeIdx) ((hasClass[n]    != 0) -> (nodeEnclave[n] == nodeEnclave[hasClass[n]]));
constraint :: "NodeLevelAtEnclaveLevel"   forall (n in NodeIdx) (hasLabelLevel[taint[n]] == hasEnclaveLevel[nodeEnclave[n]]);

%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Classifying annotations %
%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Annotations on functions, methods, and constructors are function annotations and can only be made by the user.
% No other node should have a function annotation.
constraint :: "FnAnnotationForFnOnly"
    forall (n in NodeIdx)          (isFunctionAnnotation[taint[n]] -> isFunction(n));
constraint :: "FnAnnotationByUserOnly"
    forall (n in FunOrMethodOrCon) (isFunctionAnnotation[taint[n]] -> userAnnotatedNode[n]);
constraint :: "annotationOnFunctionIsFunAnnotation"
    forall (n in FunOrMethodOrCon) (userAnnotatedNode[n] -> isFunctionAnnotation[taint[n]]);

% Annotations on classes and their fields are node annotations, not function annotations.
constraint :: "annotationOnClassIsNodeAnnotation"
    forall (n in Decl_Record) (userAnnotatedNode[n] -> not (isFunctionAnnotation[taint[n]]));
constraint :: "annotationOnFieldIsNodeAnnotation"
    forall (n in Decl_Field)  (userAnnotatedNode[n] -> not (isFunctionAnnotation[taint[n]]));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Structural taint relationships between nodes %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% All nodes in an un-annotated function must have the taint of the function
constraint :: "UnannotatedFunContentTaintMatch"
    forall (n in NodeIdx where hasFunction[n] != 0) ((not userAnnotatedNode[hasFunction[n]]) -> (taint[n] == ftaint(n)));

% All nodes in an un-annotated class must have the taint of the class
% (may be ALL if multiple taints construct the class, but not if the class accesses tainted data)
constraint :: "noAnnotatedDataForUnannotatedClass"
    forall (n in NodeIdx where hasClass[n] != 0)
        ((not userAnnotatedNode[hasClass[n]]) -> (taint[n] == taint[hasClass[n]]));

% All nodes in an un-annotated class must be un-annotated
constraint :: "noAnnotatedDataForUnannotatedClass"
    forall (n in NodeIdx where hasClass[n] != 0)
        ((not userAnnotatedNode[hasClass[n]]) -> (not userAnnotatedNode[n]));

% Un-annotated constructors get the class taint
constraint :: "unannotatedConstructorGetsClassTaint"
    forall (n in Decl_Constructor)
        ((hasClass[n] != 0 /\ not userAnnotatedNode[n]) -> (taint[n] == taint[hasClass[n]]));

% Un-annotated destructors get the class taint
constraint :: "unannotatedDestructorGetsClassTaint"
    forall (n in Decl_Destructor)
        ((hasClass[n] != 0 /\ not userAnnotatedNode[n]) -> (taint[n] == taint[hasClass[n]]));

% Un-annotated methods get the class taint
constraint :: "unannotatedMethodGetsClassTaint"
    forall (n in Decl_Method)
        ((hasClass[n] != 0 /\ not userAnnotatedNode[n]) -> (taint[n] == taint[hasClass[n]]));

% Classes connected by an inheritance relationship share the same taint
constraint :: "inheritTaint"
    forall (e in Record_Inherit) (esTaint(e) == edTaint(e));

% All nodes in an annotated function must have a taint in the ARCtaints
constraint :: "AnnotatedFunContentCoercible"
    forall (n in NodeIdx where (hasFunction[n] != 0) /\ (not isFunction(n)))
        (userAnnotatedNode[hasFunction[n]] -> isInArctaint(ftaint(n), taint[n], hasLabelLevel[taint[n]]));

% Annotated constructors (which are only valid in annotated classes) must have the class taint
% as their sole rettaint in every CDF
% TODO: doesn't validate that it is the only rettaint
constraint :: "annotatedConstructorReturnsClassTaint"
    forall (n in Decl_Constructor)
        (userAnnotatedNode[n] ->
            (forall (lvl in Level)
                ((cdfForRemoteLevel[taint[n], lvl] != nullCdf) -> hasRettaints[cdfForRemoteLevel[taint[n], lvl], taint[hasClass[n]]])));

% Any function whose address is taken in the program cannot have a function annotation
constraint :: "FunctionPtrSinglyTainted"
    forall (e in Data_PointsTo) (isFunction(hasDest[e]) -> not userAnnotatedNode[hasDest[e]]);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Control never leaves enclave except via valid XDC %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

constraint :: "NonCallRetControlEnclaveSafe"
    forall (e in Control_EnclaveSafe) (xdedge(e) == false);
constraint :: "XDCallBlest" 
    forall (e in Control_Invocation) (xdedge(e) -> userAnnotatedNode[hasDest[e]]);
constraint :: "XDCallAllowed"
    forall (e in Control_Invocation) (xdedge(e) -> allowOrRedact(edFunCdf(e)));
constraint :: "XDReturnAllowed"
    forall (e in Control_Return) (xdedge(e) -> allowOrRedact(esFunCdf(e)));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Data never leaves enclave except via parameters or return for valid XDC %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

constraint :: "EnclaveSafeDataEdges"
    forall (e in Data_EnclaveSafe) (xdedge(e) == false);

% Cross-domain returns must be allowed by the CDF, but the taints need not match
% (can't enforce taint matching as the level must change).
constraint :: "XDReturnDataAllowed"
    forall (e in Data_Return)
        (xdedge(e) -> allowOrRedact(esFunCdf(e)));

% Cross-domain Points-To edges must be allowed by the CDF, but the taints need not match.
% Note: The reason cross domain points-to edges are not disallowed completely
% is because passing arrays necessitates cross-domain points-to edges.
constraint :: "XDPointsToAllowed"
    forall (e in Data_PointsTo)
        (xdedge(e) -> (allowOrRedact(cdfForRemoteLevel[edTaint(e), hasLabelLevel[esTaint(e)]]) /\ not isFunction(hasDest[e])));

%%%%%%%%%%%%%%%%%%%%%
% Taint propagation %
%%%%%%%%%%%%%%%%%%%%%

% Data edges within un-annotated functions are fully constrained by UnannotatedFunContentTaintMatch,
% because the source and destination are in the same function.

% Data edges within annotated functions are partially constrained by AnnotatedFunContentCoercible,
% so we have that the source and destination must be in the function's ARCtaints.
% Sometimes, however, pointer dependencies are captured by a chain
% of points-to edges which may be intra-function edges. Therefore
% we restrict intra-function points-to edges to have the
% same taint, even in annotated functions.
predicate intraFunEdge(DataEdge: e) =
    (hasFunction[hasSource[e]] != 0 /\ hasFunction[hasDest[e]] != 0 /\
     hasFunction[hasSource[e]] == hasFunction[hasDest[e]]);
constraint :: "intraFunPointsToTaintsMatch"
    forall (e in Data_PointsTo) (intraFunEdge(e) -> esTaint(e) == edTaint(e));

% For ANY data edge between two function-external nodes, the taints must match
predicate globalGlobalEdge(DataEdge: e) = (isGlobal[hasSource[e]] /\ isGlobal[hasDest[e]]);
constraint :: "externExternDataEdgeTaintsMatch"
    forall (e in DataEdge) (globalGlobalEdge(e) -> (esTaint(e) == edTaint(e)));

% For ANY data edge between data within a function and a function-external node, the taints must match
predicate srcFunExternEdge(DataEdge: e)  = (hasFunction[hasSource[e]] != 0 /\ hasFunction[hasDest[e]]   == 0);
predicate destFunExternEdge(DataEdge: e) = (hasFunction[hasDest[e]]   != 0 /\ hasFunction[hasSource[e]] == 0);
predicate funExternEdge(DataEdge: e)     = (srcFunExternEdge(e) \/ destFunExternEdge(e));
constraint :: "externDataEdgeTaintsMatch"
    forall (e in DataEdge) (funExternEdge(e) -> esTaint(e) == edTaint(e));

% This leaves taint propagation for inter-function data edges.

% For non-XD return edges from a callee function to a callsite:
%   -If the callee (source) function is un-annotated, the taints must match
%   -If the callee (source) function is annotated, the taint on the dest node
%    must be in the callee's rettaints
constraint :: "retEdgeFromUnannotatedTaintsMatch"
    forall (e in Data_Return where not sourceAnnotFun(e))
        (esTaint(e) == edTaint(e));
constraint :: "returnNodeInRettaints"
    forall (e in Data_Return where sourceAnnotFun(e) /\ not xdedge(e))
        (hasRettaints[esFunCdf(e), edTaint(e)]);

% ArgPass edges: The callsite node (Stmt_Call) has ArgPass edges to each of its arguments, and call invocation edges
% to each potential callee. The ParamIdx is on the argpass edge destination and the Decl_Param (unused).
% For non-XD argument passing edges:
%   -If the destination function is un-annotated, the taint of the argument must match the taint of the destination function
%   -If the destination function is annotated, the taint of the argument must be in the argtaints of the function at that parameter index
constraint :: "argumentToUnannotatedTaintsMatch"
    forall (e in Control_Invocation where not destAnnotFun(e))
        (forall (arg_e in Data_ArgPass where hasSource[arg_e] == hasSource[e])
            (edTaint(arg_e) == edTaint(e)));
constraint :: "argumentInArgtaints"
    forall (e in Control_Invocation where destAnnotFun(e) /\ not xdedge(e))
        (forall (arg_e in Data_ArgPass where hasSource[arg_e] == hasSource[e])
            (hasArgtaints[edFunCdf(e), hasParamIdx[hasDest[arg_e]], taint[hasDest[arg_e]]]));

% Inter-function points-to edges should always have the same taint
predicate interFunEdge(DataEdge: e) =
    (hasFunction[hasSource[e]] != 0 /\ hasFunction[hasDest[e]] != 0 /\
     hasFunction[hasSource[e]] != hasFunction[hasDest[e]]);
constraint :: "interFunPointsToTaintsMatch"
    forall (e in Data_PointsTo)
        ((not xdedge(e) /\ interFunEdge(e)) -> esTaint(e) == edTaint(e));

% TODO: Properly constrain Data_FieldAccess edges. FieldAccess is special because it's a transfer of data from in
% to outside of a class which is not mediated by a function. Interacts with the ALL label.
% TODO: ALL label. Use a function.

