%%%%%%%%%%%%%%%
% Graph Nodes %
%%%%%%%%%%%%%%%

% TODO: Potential new node type needed for implicit constructors?
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

set of int: NodeIdx; % Spans the range of the above

%%%%%%%%%%%%%%%
% Graph edges %
%%%%%%%%%%%%%%%

set of int: Record_Field;                  % Connects a record to its field                               | Decl.Record -> Decl.Field
set of int: Record_Method;                 % Connects a record to its method                              | Decl.Record -> Decl.Method
set of int: Record_Constructor;            % Connects a record to its constructor                         | Decl.Record -> Decl.Constructor
set of int: Record_Destructor;             % Connects a record to its destructor                          | Decl.Record -> Decl.Destructor 
set of int: Record_Inherit;                % Connects two records by inheritance relation                 | Decl.Record -> Decl.Record

set of int: Control_Return;                % A function/method/constructor/destructor return              | Stmt.Return -> Stmt.Call
set of int: Control_Entry;                 % Connects a function-like object to it's body                 | Decl -> Stmt

set of int: Control_FunctionInvocation;    % A direct call invocation to a C-style function               | Stmt.Call -> Decl.Function
set of int: Control_MethodInvocation;      % A method call                                                | Stmt.Call -> Decl.Method
set of int: Control_ConstructorInvocation; % A constructor call                                           | Stmt.Call -> Decl.Constructor
set of int: Control_DestructorInvocation;  % A call to a destructor                                       | Stmt.Call -> Decl.Destructor

set of int: Data_PointsTo;                 % A points-to relation                                         | Decl -> Decl
set of int: Data_DefUse;                   % A def-use relation between statements                        | Stmt.Decl -> Stmt.Ref
set of int: Data_ArgPass;                  % An argument pass to a call instruction                       | Stmt.Call -> Stmt
set of int: Data_Object;                   % Connects a method call/field to the object being called upon | Stmt.Call/Stmt.Field -> Stmt
set of int: Data_Param;                    % Connects a function-like object to its parameters            | Decl -> Decl.Param
set of int: Data_FieldAccess;              % A field access                                               | Stmt -> Decl.Field
set of int: Data_InstanceOf;               % Connects a 'this' instance to a class definition             | Stmt.This -> Decl.Record
set of int: Data_Decl;                     % Connects a decl statment to a decl                           | Stmt.Decl -> Decl
set of int: Data_Child;                    % When a statement has a child not described by the above      | Stmt -> Stmt

set of int: DataEdge;
set of int: EdgeIdx; % Spans the range of the above

% Convenience aggregates
set of int: Control_Invocation    = Control_FunctionInvocation union Control_MethodInvocation union
                                    Control_ConstructorInvocation union Control_DestructorInvocation;
% TODO: Control_Return omitted?
set of int: Control_NonInvocation = Record_Field union Record_Method union Record_Constructor union
                                    Record_Destructor union Record_Inherit union Control_Entry;
  
set of int: Data_EnclaveSafe      = Data_Child union Data_FieldAccess union Data_InstanceOf union Data_Param union
                                    Data_Object union Data_DefUse union Data_Decl;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Containing Function for PDG Nodes, Endpoints for PDG Edges, Indices of Function Formal Parameters %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% n.b. if n is a Decl_Record (i.e. a class declaration), then hasClass[n] == 0
% if the class is not contained in another class, but if n is a Decl_Function,
% then hasFunction[n] == n
array[Decl_Var] of int:  isGlobal; % property for Decl_Var
array[NodeIdx]  of int:  hasClass;

array[NodeIdx]  of bool: userAnnotatedNode;

array[NodeIdx]  of int:  hasFunction;
array[EdgeIdx]  of int:  hasSource;
array[EdgeIdx]  of int:  hasDest;
array[Param]    of int:  hasParamIdx;

%%%%%%%%%%%%%%%%%%%
% CLE Input Model %
%%%%%%%%%%%%%%%%%%%

enum Level;
enum Enclave;
array[Enclave] of Level: hasEnclaveLevel;

enum cleLabel;
enum cdf;
enum GuardOperation = {nullGuardOperation, allow, deny, redact};
enum Direction      = {nullDirection, bidirectional, egress, ingress};

int: MaxFuncParms;  % Max number of function parameters in the program (C++ < 256)
set of int: parmIdx = 1..MaxFuncParms;

array[cleLabel]                       of Level:          hasLabelLevel;
array[cleLabel]                       of bool:           isFunctionAnnotation;

array[cdf]                            of cleLabel:       fromCleLabel;
array[cdf]                            of Level:          hasRemotelevel;
array[cdf]                            of GuardOperation: hasGuardOperation;
array[cdf]                            of Direction:      hasDirection;
array[cdf]                            of bool:           isOneway;
array[cleLabel, Level]                of cdf:            cdfForRemoteLevel;

array[cdf, cleLabel]          of bool:           hasRettaints;
array[cdf, cleLabel]          of bool:           hasCodtaints;
array[cdf, parmIdx, cleLabel] of bool:           hasArgtaints;
array[cdf, cleLabel]          of bool:           hasARCtaints;

array[NodeIdx] of var Enclave:  nodeEnclave;
array[NodeIdx] of var cleLabel: taint;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Utility functions and predicates
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

predicate isFunctionEntry(NodeIdx: n) = (n in Decl_Function);
predicate allowOrRedact(var cdf: c)   = (hasGuardOperation[c] == allow \/ hasGuardOperation[c] == redact);

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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Class constraints for C++ %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% User annotations must be of the right type. Node annotations for classes/fields,
% function annotations for constructors/methods.
constraint :: "annotationOnClassIsNodeAnnotation"
  forall (n in Decl_Record)
    (userAnnotatedNode[n] -> not (isFunctionAnnotation[taint[n]]));
constraint :: "annotationOnFieldIsNodeAnnotation"
  forall (n in Decl_Field)
    (userAnnotatedNode[n] -> not (isFunctionAnnotation[taint[n]]));
constraint :: "annotationOnFunctionIsFunAnnotation"
  forall (n in (Decl_Function union Decl_Constructor union Decl_Method))
    (userAnnotatedNode[n] -> isFunctionAnnotation[taint[n]]);

% In un-annotated classes, everything (fields, constructors, methods) must have the same label
% (may be ALL if multiple taints construct the class, but not if the class accesses tainted data)
constraint :: "noAnnotatedDataForUnannotatedClass"
  forall (n in NodeIdx)
    ((hasClass[n] != 0 /\ not userAnnotatedNode[hasClass[n]]) -> (not userAnnotatedNode[n]));

% Un-annotated constructors get the class taint.
constraint :: "unannotatedConstructorGetsClassTaint"
  forall (n in Decl_Constructor)
    ((hasClass[n] != 0 /\ not userAnnotatedNode[n]) -> (taint[n] == taint[hasClass[n]]));

% Un-annotated methods get the class taint
constraint :: "unannotatedMethodGetsClassTaint"
  forall (n in Decl_Method)
    ((hasClass[n] != 0 /\ not userAnnotatedNode[n]) -> (taint[n] == taint[hasClass[n]]));

% Annotated constructors (which are only valid in annotated classes) must have the class taint
% as their sole rettaint in every CDF
% TODO: doesn't validate that it is the only rettaint
constraint :: "annotatedConstructorReturnsClassTaint"
  forall (n in Decl_Constructor)
    (userAnnotatedNode[n] ->
      (forall (lvl in Level)
         ((cdfForRemoteLevel(taint[n], lvl) != nullCdf) -> hasRettaints[cdfForRemoteLevel(taint[n], lvl), taint[hasClass[n]]])));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Basic constraints on output decision variables
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

constraint :: "NodeHasEnclave"          forall (n in NodeIdx)       (nodeEnclave[n] != nullEnclave);
constraint :: "NodeEnclaveIsFunEnclave" forall (n in NodeIdx)       (hasFunction[n] -> (nodeEnclave[n] == nodeEnclave[hasFunction[n]]));

constraint :: "NodeLevelAtEnclaveLevel" forall (n in NodeIdx)       (hasLabelLevel[taint[n]] == hasEnclaveLevel[nodeEnclave[n]]);
constraint :: "FnAnnotationForFnOnly"   forall (n in NodeIdx)       (isFunctionAnnotation[taint[n]] -> isFunctionEntry(n));
constraint :: "FnAnnotationByUserOnly"  forall (n in Decl_Function) (isFunctionAnnotation[taint[n]] -> userAnnotatedNode[n]);

constraint :: "UnannotatedFunContentTaintMatch"
  forall (n in NodeIdx where hasFunction[n] != 0) ((not userAnnotatedNode[hasFunction[n]]) -> taint[n] == ftaint(n));

constraint :: "AnnotatedFunContentCoercible"
  forall (n in NodeIdx where (hasFunction[n] != 0) /\ (not isFunctionEntry(n)))
    (userAnnotatedNode[hasFunction[n]] -> isInArctaint(ftaint(n), taint[n], hasLabelLevel[taint[n]]));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Control never leaves enclave except via valid XDC
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

constraint :: "NonCallControlEnclaveSafe" forall (e in Control_NonInvocation) (xdedge(e) == false);
constraint :: "XDCallBlest"               forall (e in Control_Invocation) (xdedge(e) -> userAnnotatedNode[hasDest[e]]);

constraint :: "XDCallAllowed"
 forall (e in Control_Invocation) (xdedge(e) -> allowOrRedact(cdfForRemoteLevel[edTaint(e), hasLabelLevel[esTaint(e)]]));

constraint :: "XDCDataReturnAllowed"
 forall (e in Control_Return) (xdedge(e) -> allowOrRedact(cdfForRemoteLevel[esTaint(e), hasLabelLevel[edTaint(e)]]));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Data never leaves enclave except via parameters or return for valid XDC
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

constraint :: "EnclaveSafeDataEdges"
 forall (e in Data_EnclaveSafe) (xdedge(e) == false);

constraint :: "XDCParmAllowed"
 forall (e in Data_ArgPass)
   (xdedge(e) -> allowOrRedact(cdfForRemoteLevel[esTaint(e), hasLabelLevel[edTaint(e)]]));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Intra-function and function-to-global edges %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% For data edges between a function and external data not in any
% function:
%   -If the function is annotated, the taint on the 
%    external node must be in the function's ARCtaints.
%   -If the function is un-annotated, the taints must match.
predicate interFunEdge(EdgeIdx: e) =
  (hasFunction[hasSource[e]] != 0 /\ hasFunction[hasDest[e]] != 0 /\
   hasFunction[hasSource[e]] != hasFunction[hasDest[e]]);
predicate srcFunExternEdge(EdgeIdx: e) =
  (hasFunction[hasSource[e]] != 0 /\ hasFunction[hasDest[e]] == 0);
predicate destFunExternEdge(EdgeIdx: e) =
  (hasFunction[hasDest[e]] != 0 /\ hasFunction[hasSource[e]] == 0);

predicate externUnannotated(EdgeIdx: e) =
  ((srcFunExternEdge(e)  /\ not sourceAnnotFun(e)) \/
   (destFunExternEdge(e) /\ not destAnnotFun(e)));
predicate srcFunExternAnnotated(EdgeIdx: e) =
  (srcFunExternEdge(e) /\ sourceAnnotFun(e));
predicate destFunExternAnnotated(EdgeIdx: e) =
  (destFunExternEdge(e) /\ destAnnotFun(e));

constraint :: "UnannotatedExternDataEdgeTaintsMatch"
  forall (e in Data_Edge)
    (externUnannotated(e) -> esTaint(e) == edTaint(e));
constraint :: "AnnotatedExternDataEdgeInArctaints"
  forall (e in Data_Edge)
    ((srcFunExternAnnotated(e) ->
       isInArctaint(esFunTaint(e), edTaint(e), hasLabelLevel[edTaint(e)])) /\
     (destFunExternAnnotated(e) ->
       isInArctaint(edFunTaint(e), esTaint(e), hasLabelLevel[esTaint(e)])));

%%%%%%%%%%%%%%%%%%%%%%%%
% Inter-function edges %
%%%%%%%%%%%%%%%%%%%%%%%%

% TODO: Data_Return edge type doesn't exist? Where is it, and if it's not in the graph, what do we use instead?
% TODO: no distinction made between direct and indirect edges anymore?
% TODO: How have argpass edges changed from the in/out model?

% For return edges from a callee function to a callsite:
%   -If the callee (source) function is un-annotated, the taints must match
%   -If the callee (source) function is annotated, the taint on the dest node
%    must be in the callee's rettaints (or it's a cross-domain edge)
constraint :: "retEdgeFromUnannotatedTaintsMatch"
  forall (e in Data_Return)
    (not sourceAnnotFun(e)
      -> esTaint(e) == edTaint(e));
constraint :: "returnNodeInRettaints"
  forall (e in Data_Return)
    (sourceAnnotFun(e)
      -> (hasRettaints[esFunCdf(e), edTaint(e)] \/ xdedge(e)));

% For caller to callee ArgPass edges:
%   -If the destination function is un-annotated, the taints must match
%   -If the destination function is annotated, the taint on the source node
%    must be in the argtaints of the corresponding parameter index
%    (or it's a cross-domain edge)
constraint :: "argPassInEdgeToUnannotatedTaintsMatch"
  forall (e in Data_ArgPass)
    (not destAnnotFun(e)
      -> esTaint(e) == edTaint(e));
constraint :: "argPassInSourceInArgtaints"
  forall (e in Data_ArgPass)
    (destAnnotFun(e)
      -> hasArgtaints[edFunCdf(e), hasParamIdx[hasDest[e]], esTaint(e)] \/ xdedge(e));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Phase 3 indirect and pointer-based edge constraints %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% TODO: no distinction made between direct and indirect edges anymore?

% Indirect call invocations cannot be cross-domain.
% constraint :: "IndirectCallSameEnclave"
%   forall (e in ControlDep_Indirect_CallInv)
%     (xdedge(e) == false);

% ArgPass and Ret edges of indirect calls must satisfy 
% the same constraints as their direct call counter-parts.
% They are included in the unions above for inter-function edges.

% Cross-Domain Points-To Edges must have compatible taints.
% Note: The reason cross domain points-to edges are not disallowed completely
% is because passing arrays necessitates cross-domain points-to edges.
constraint :: "PointsToXD"
  forall (e in Data_PointsTo)
    (xdedge(e) -> (allowOrRedact(cdfForRemoteLevel[edTaint(e), hasLabelLevel[esTaint(e)]]) /\ not isFunctionEntry(hasDest[e]))); 

% Endpoints of a points-to edge must have the same taint unless they are cross-domain
% (including cases in which either endpoint is 
% global/module-static/function).
% Sometimes pointer dependencies are captured by a chain
% of points-to edges which may be intra-function edges. Therefore
% we restrict intra-function points-to edges to have the
% same taint, even in annotated functions.
constraint :: "PointsToTaintsMatch"
  forall (e in Data_PointsTo)
    ((not xdedge(e)) -> esTaint(e) == edTaint(e));

% If two global variables are connected by a def-use edge,
% they must have the same taint.
constraint :: "GlobalDefUseTaintsMatch"
  forall (e in Data_DefUse)
    ((isGlobal[hasSource[e]] /\ isGlobal[hasDest[e]]) -> (esTaint(e) == edTaint(e)));

% Any function whose address is taken in the program 
% cannot have a function annotation.
constraint :: "FunctionPtrSinglyTainted"
  forall (e in Data_PointsTo)
    (isFunctionEntry(hasDest[e]) -> not userAnnotatedNode[hasDest[e]]);

% MISSING:
% A structure or array and all its fields/elements 
% must have the same taint

% MISSING:
% External callback subgraph constraint