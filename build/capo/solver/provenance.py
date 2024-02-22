cle_template = "; Fact: "
node_template =       "; NODE\n;     File:     {}\n;     Function: {}\n;     Line:     {}\n;     Label:    {}\n; Rule:\n;     "
edge_template = ("; EDGE FROM\n;     File:     {}\n;     Function: {}\n;     Line:     {}\n;     Label:    {}\n" + 
                        "; TO\n;     File:     {}\n;     Function: {}\n;     Line:     {}\n;     Label:    {}\n; Rule:\n;     ")

rules = {
    "taints":                                "This line was annotated with the given label.",
    "hasLabelLevel":                         "Label {} is at level {}.",
    "isFunctionAnnotation":                  "Label {} is {}a function annotation.",
    "hasGuardOperation":                     "CDF {} has guard operation {}.",
    "hasEnclaveLevel":                       "Enclave {} is at level {}.",
    "cdfForRemoteLevel":                     "Label {} has CDF {} at remote level {}.",
    "hasRettaints":                          "CDF {} {} {} as a rettaint.",
    "hasARCtaints":                          "CDF {} {} {} as an ARCtaint.",
    "hasArgtaints":                          "CDF {} {} {} as an argtaint of argument {}.",
    "isPinned":                              "Class with node ID={} {} to receive the ALL label.",
    "NodeHasEnclave":                        "Node must have a non-null enclave.",
    "NodeEnclaveIsFunEnclave":               "Node must have the same enclave as its containing function.",
    "NodeEnclaveIsClassEnclave":             "Node must have the same enclave as its containing class.",
    "NodeLevelAtEnclaveLevel":               "The level of the node's taint must match the level of the node's enclave.",
    "FnAnnotationForFnOnly":                 "Function taints can only be applied to functions.",
    "FnAnnotationByUserOnly":                "Function annotations can only be made by the developer.",
    "annotationOnFunctionIsFunAnnotation":   "If the developer annotates this function, it must be with a function annotation.",
    "annotationOnClassIsNodeAnnotation":     "If the developer annotates this class, it must be with a node annotation.",
    "annotationOnFieldIsNodeAnnotation":     "If the developer annotates this field, it must be with a node annotation.",
    "UnannotatedFunContentTaintMatch":       "This node is in an un-annotated function and must share its taint with the function.",
    "UnannotatedClassTaintsMatch":           "This node is in an un-annotated class and must share its taint with the class.",
    "noAnnotatedDataForUnannotatedClass":    "This node is in an un-annotated class and may not be annotated.",
    "unannotatedConstructorGetsClassTaint":  "This constructor is un-annotated and must receive the class taint.",
    "unannotatedDestructorGetsClassTaint":   "This destructor is un-annotated and must receive the class taint.",
    "unannotatedMethodGetsClassTaint":       "This method is un-annotated and must receive the class taint.",
    "AnnotatedFunContentCoercible":          "This node's taint must be in the ARCtaints of its function.",
    "annotatedConstructorReturnsClassTaint": "This constructor is annotated, so its rettaint must be the class taint.",
    "definitionForALL":                      "This node may only take the ALL label if it is in an unpinned class",
    "inheritTaint":                          "These classes are connected by an inheritance relationship and must share the same taint.",
    "FunctionPtrSinglyTainted":              "This function has its address taken and may not have a function annotation.",
    "NonCallRetControlEnclaveSafe":          "This is an enclave-safe control/structural edge; it must not be cross-domain.",
    "XDCallBlest":                           "If this call is cross-domain, the callee must be annotated.",
    "XDCallAllowed":                         "If this call is cross-domain, it must be allowed by the callee CDF.",
    "XDReturnAllowed":                       "If this return is cross-domain, it must be allowed by the caller CDF.",
    "EnclaveSafeDataEdges":                  "This is an enclave-safe data edge; it must not be cross-domain.",
    "XDReturnDataAllowed":                   "If this data return is cross-domain, it must be allowed by the caller CDF.",
    "XDPointsToAllowed":                     "If this points-to edge is cross-domain, it must be allowed by the callee CDF.",
    "intraFunPointsToTaintsMatch":           "These nodes have a points-to dependency and must share the same taint.",
    "externExternDataEdgeTaintsMatch":       "These two globals have a data dependency and must share the same taint.",
    "externDataEdgeTaintsMatch":             "This is a data dependency from a global to a node in a function. The taints must match.",
    "retEdgeFromUnannotatedTaintsMatch":     "This return is from an un-annotated function; the taint of the return data must match the taint at the callsite.",
    "returnNodeInRettaints":                 "This return is from an annotated function; the taint of the callsite must be in the callee's rettaints.",
    "argumentToUnannotatedTaintsMatch":      "This call is to an un-annotated function; the taints of each argument must match the callee's taint.",
    "argumentInArgtaints":                   "This call is to an annotated function; the taints of each argument must match the callee's argtaints.",
    "interFunPointsToTaintsMatch":           "These nodes have a points-to dependency and must share the same taint."
}

def getLine(node_row):
    return node_row[2]

def node_provenance(node_id, nodes):
    row = nodes[node_id - 1]
    fun_id = int(row[6])
    fun = "None"
    if fun_id != 0: fun = getLine(nodes[fun_id - 1])
    return row[8], fun, getLine(row), row[3]

def edge_provenance(edge_id, nodes, edges):
    row = edges[edge_id - 1]
    n1, n2 = int(row[2]), int(row[3])
    f1, fun1, line1, label1 = node_provenance(n1, nodes)
    f2, fun2, line2, label2 = node_provenance(n2, nodes)
    return f1, fun1, line1, label1, f2, fun2, line2, label2

def explain_single(fml, t, n, args, nodes, edges):
    assertion = "\n(assert {})".format(fml.sexpr())
    r = rules[n]
    if   t == 'cle':
        return (cle_template + r).format(*args) + assertion
    elif t == 'node':
        return (node_template + r).format(*node_provenance(args, nodes)) + assertion
    elif t == 'edge':
        return (edge_template + r).format(*edge_provenance(args, nodes, edges)) + assertion

def explain_all(cs, nodes, edges):
    s = "; EXPLANATION FOR UNSATISFIABILITY\n"
    for (fml, (t, n, args)) in cs: s += "\n{}\n".format(explain_single(fml, t, n, args, nodes, edges))
    s += "\n(check-sat)"
    return s
