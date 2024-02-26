from time import time
from math import log2, ceil

from provenance import explain_all
from z3 import * 

class ConflictAnalyzer:

    def __init__(self, pdg, cle, solver_type, minimize):

        # Options
        self.use_bv = (solver_type == 'bv')
        self.minimize = minimize

        # Solver state
        self.s = Solver()
        self.encoded = False
        self.status = 'UNSOLVED'
        self.constraints = []
        self.assumptions = []
        self.explanations = {}

        # Program graph
        self.pdg = pdg

        # Conversion from CLE into z3 API representation
        self.Label, self.LabelCons = EnumSort('Label', cle.cle_labels)
        self.Level, self.LevelCons = EnumSort('Level', cle.levels)
        self.Enclave, self.EnclaveCons = EnumSort('Enclave', cle.enclaves)
        self.GuardOperation, self.GuardOperationCons = EnumSort('GuardOperation', cle.gds)
        self.nLabels = len(self.LabelCons)
        self.nLevels = len(self.LevelCons)
        self.nEnclaves = len(self.EnclaveCons)
        self.label2enum = dict(zip(cle.cle_labels, self.LabelCons))
        self.level2enum = dict(zip(cle.levels, self.LevelCons))
        self.enc2enum = dict(zip(cle.enclaves, self.EnclaveCons))
        self.gd2enum = dict(zip(cle.gds, self.GuardOperationCons))
        self.allow = self.gd2enum['allow']
        self.redact = self.gd2enum['redact']
        self.nullCleLabel = self.label2enum['nullCleLabel']
        self.nullLevel = self.level2enum['nullLevel']
        self.nullEnclave = self.enc2enum['nullEnclave']
        self.allLabel = self.label2enum['ALL']
        self.allLevel = self.level2enum['all']
        self.allEnclave = self.enc2enum['all_E']
        self.hasLabelLevel = [ self.level2enum[e['cle-json']['level']] for e in cle.cle ]
        self.hasEnclaveLevel = list(self.LevelCons)
        self.isFunctionAnnotation = [ cle.isFn(e) for e in cle.cle ]       
        self.Cdf, self.CdfCons = EnumSort('Cdf', cle.all_cdfs)
        self.cdf2enum = dict(zip(cle.all_cdfs, self.CdfCons))
        self.nCdfs = len(self.CdfCons)
        self.nullCdf = self.cdf2enum['nullCdf']
        self.hasGuardOperation = [self.gd2enum[gd] for gd in cle.has_guard_op]
        self.cdfForRemoteLevel = [[self.cdf2enum[cdf] for cdf in cdfs] for cdfs in cle.cdf_for_remote_level]
        self.hasRettaints = [[s == 'true' for s in ls] for ls in cle.has_rettaints]
        self.hasCodtaints = [[s == 'true' for s in ls] for ls in cle.has_codtaints]
        self.hasARCtaints = [[s == 'true' for s in ls] for ls in cle.has_arctaints]
        self.hasArgtaints = [[[s == 'true' for s in ls] for ls in lss] for lss in cle.has_argtaints]

        # Always encode immediately
        self.encode()

    # add a set of constraints while excluding them from unsat cores (currently unused)
    def assume(self, assumptions):
        self.s.add(assumptions)
        self.assumptions.extend(assumptions)

    # add a set of constraints
    def add(self, constraints):
        self.constraints.extend(constraints)

    # Solve and return sat or unsat
    def solve(self):
        t = time()
        print("solving (using theory: {})...".format("bv" if self.use_bv else "int"))
        if self.status != unsat:
            for (c, ex) in self.constraints:
                self.explanations[c] = ex
            self.status = self.s.check(list(list(zip(*self.constraints))[0]))
        print("\nsolving time (s): {}".format(int(time() - t)))
        return self.status

    # Print out the model for a sat instance
    def evidence(self, handle):
        if self.status != sat:
            return "No evidence for '{}' status".format(str(self.status))
        print("Dumping model...")
        handle.write(str(self.s.model()))

    # Explain unsatisfiability of an instance
    def explain(self, temp_dir, handle):
        t = time()
        if self.status != unsat:
            return "Cannot explain '{}' status".format(str(self.status))
        print("determining unsat core explanation (core minimization: {})...".format("on" if self.minimize else "off"))
        core = self.s.unsat_core()
        core_s = Solver()
        if self.minimize:
            core_s.set(':core.minimize', True)
            core_s.add(self.assumptions)
            assert core_s.check([fml for fml in core]) == unsat
            core = core_s.unsat_core()
            core_s = Solver()
        core_s.add(self.assumptions + [fml for fml in core])
        with open(temp_dir / 'core.smt2', 'w') as f: f.write(core_s.sexpr())
        print("unsat core time (s): {}".format(int(time() - t)))
        handle.write(explain_all([(fml, self.explanations[fml]) for fml in core], self.pdg.nodes_csv, self.pdg.edges_csv))

    def encode(self):

        if self.encoded: return
        self.encoded = True
        pdg = self.pdg

        t_start = time()
        def elapsed():
            print("\ntime (s): {}".format(int(time() - t_start)))

        # Sorts (sort of IDs may be ints or bitvectors)
        b = BoolSort()
        id = IntSort()
        def mkId(i):
            assert i >= 0
            return i
        def mkVar(s):
            return Int(s)
        if self.use_bv:
            bvl = ceil(log2(pdg.e['EdgeIdx'][-1]))
            id = BitVecSort(bvl)
            mkId = lambda i: BitVecVal(i, bvl)
            mkVar = lambda s: BitVec(s, bvl)
            
        # Decision variables
        nodeEnclave = Function('nodeEnclave', id, self.Enclave)
        taint = Function('taint', id, self.Label)
        self.add([(taint(mkId(i)) == self.label2enum[pdg.nodeTaints[i]], ('node', 'taints', i)) for i in pdg.nodeTaints])

        # Aggregates
        nUnion = lambda ns: sum([pdg.n[n] for n in ns], [])
        eUnion = lambda es: sum([pdg.e[e] for e in es], [])
        pdg.FunctionLike = nUnion(['Decl_Function', 'Decl_Constructor', 'Decl_Method', 'Decl_Destructor'])
        pdg.DataEdge = eUnion(['Data_PointsTo', 'Data_DefUse', 'Data_ArgPass', 'Data_Return', 'Data_Object', 'Data_FieldAccess', 'Data_InstanceOf', 'Data_Decl'])
        pdg.Control_EnclaveSafe = eUnion(['Struct_Field', 'Struct_Method', 'Struct_Constructor', 'Struct_Destructor', 'Struct_Inherit', 'Control_Entry', 'Struct_Child', 'Struct_Param'])
        pdg.Control_Invocation = eUnion(['Control_FunctionInvocation', 'Control_MethodInvocation', 'Control_ConstructorInvocation', 'Control_DestructorInvocation'])
        pdg.Data_EnclaveSafe = eUnion(['Data_FieldAccess', 'Data_InstanceOf', 'Data_Object', 'Data_DefUse', 'Data_Decl', 'Data_ArgPass'])

        # Helpers / Properties (operate on python integers, NOT z3 IDs!)
        hasClass          = lambda n: pdg.hasClass[n - 1] # 0 for node not in a class, otherwise maps to the Decl_Record containing the node
        isGlobal          = lambda n: pdg.isGlobal[n - 1]
        hasFunction       = lambda n: pdg.hasFunction[n - 1]
        hasSource         = lambda e: pdg.hasSource[e - 1]
        hasDest           = lambda e: pdg.hasDest[e - 1]
        hasParamIdx       = lambda n: pdg.hasParamIdx[n - 1]
        userAnnotatedNode = lambda n: pdg.userAnnotatedNode[n - 1]
        isFun             = lambda n: n in pdg.FunctionLike
        isClass           = lambda n: n in pdg.n['Decl_Record']
        srcFun            = lambda e: hasFunction(hasSource(e))
        dstFun            = lambda e: hasFunction(hasDest(e))
        sourceAnnotFun    = lambda e: userAnnotatedNode(srcFun(e)) if srcFun(e) != 0 else False
        destAnnotFun      = lambda e: userAnnotatedNode(dstFun(e)) if dstFun(e) != 0 else False
        interFunEdge      = lambda e: srcFun(e) != 0 and dstFun(e) != 0 and srcFun(e) != dstFun(e)
        intraFunEdge      = lambda e: srcFun(e) != 0 and dstFun(e) != 0 and srcFun(e) == dstFun(e)
        globalGlobalEdge  = lambda e: isGlobal(hasSource(e)) and isGlobal(hasDest(e))
        srcFunExternEdge  = lambda e: srcFun(e) != 0 and dstFun(e) == 0
        destFunExternEdge = lambda e: dstFun(e) != 0 and srcFun(e) == 0
        funExternEdge     = lambda e: srcFunExternEdge(e) or destFunExternEdge(e)

        # Convert CLE arrays to uninterpreted functions and populate their values
        hasLabelLevel = Function('hasLabelLevel', self.Label, self.Level)
        for lbl in range(len(self.hasLabelLevel)):
            self.add([(hasLabelLevel(self.LabelCons[lbl]) == self.hasLabelLevel[lbl], ('cle', 'hasLabelLevel', (str(self.LabelCons[lbl]), str(self.hasLabelLevel[lbl]))))])

        isFunctionAnnotation = Function('isFunctionAnnotation', self.Label, b)
        for lbl in range(len(self.isFunctionAnnotation)):
            v = self.isFunctionAnnotation[lbl]
            self.add([(isFunctionAnnotation(self.LabelCons[lbl]) == v, ('cle', 'isFunctionAnnotation', (str(self.LabelCons[lbl]), "" if v else "not ")))])

        hasGuardOperation = Function('hasGuardOperation', self.Cdf, self.GuardOperation)
        for cdf in range(len(self.hasGuardOperation)):
            self.add([(hasGuardOperation(self.CdfCons[cdf]) == self.hasGuardOperation[cdf], ('cle', 'hasGuardOperation', (str(self.CdfCons[cdf]), str(self.hasGuardOperation[cdf]))))])

        hasEnclaveLevel = Function('hasEnclaveLevel', self.Enclave, self.Level)
        for enc in range(len(self.hasEnclaveLevel)):
            self.add([(hasEnclaveLevel(self.EnclaveCons[enc]) == self.hasEnclaveLevel[enc], ('cle', 'hasEnclaveLevel', (str(self.EnclaveCons[enc]), str(self.hasEnclaveLevel[enc]))))])
        
        cdfForRemoteLevel = Function('cdfForRemoteLevel', self.Label, self.Level, self.Cdf)
        for lbl in range(len(self.cdfForRemoteLevel)):
            for lvl in range(len(self.cdfForRemoteLevel[lbl])):
                cdf = self.cdfForRemoteLevel[lbl][lvl]
                self.add([(cdfForRemoteLevel(self.LabelCons[lbl], self.LevelCons[lvl]) == cdf, ('cle', 'cdfForRemoteLevel', (str(self.LabelCons[lbl]), str(cdf), str(self.LevelCons[lvl]))))])

        hasRettaints = Function('hasRettaints', self.Cdf, self.Label, b)
        for cdf in range(len(self.hasRettaints)):
            for lbl in range(len(self.hasRettaints[cdf])):
                v = self.hasRettaints[cdf][lbl]
                self.add([(hasRettaints(self.CdfCons[cdf], self.LabelCons[lbl]) == v, ('cle', 'hasRettaints', (str(self.CdfCons[cdf]), "has" if v else "does not have", str(self.LabelCons[lbl]))))])

        hasARCtaints = Function('hasARCtaints', self.Cdf, self.Label, b)
        for cdf in range(len(self.hasARCtaints)):
            for lbl in range(len(self.hasARCtaints[cdf])):
                v = self.hasARCtaints[cdf][lbl]
                self.add([(hasARCtaints(self.CdfCons[cdf], self.LabelCons[lbl]) == v, ('cle', 'hasARCtaints', (str(self.CdfCons[cdf]), "has" if v else "does not have", str(self.LabelCons[lbl]))))])

        hasArgtaints = Function('hasArgtaints', self.Cdf, id, self.Label, b)
        for cdf in range(len(self.hasArgtaints)):
            for i in range(len(self.hasArgtaints[cdf])):
                for lbl in range(len(self.hasArgtaints[cdf][i])):
                    v = self.hasArgtaints[cdf][i][lbl]
                    self.add([(hasArgtaints(self.CdfCons[cdf], mkId(i), self.LabelCons[lbl]) == v, ('cle', 'hasArgtaints', (str(self.CdfCons[cdf]), "has" if v else "does not have", str(self.LabelCons[lbl]), str(i))))])

        isPinned = Function('isPinned', id, b)
        for r in pdg.n['Decl_Record']:
            v = False
            if userAnnotatedNode(r):
                v = True
            for n in pdg.n['NodeIdx']:
                if hasClass(n) == r and userAnnotatedNode(n):
                    v = True
            for e in pdg.e['EdgeIdx']:
                if hasClass(hasSource(e)) == r and hasClass(hasDest(e)) != r:
                    v = True
            self.add([(isPinned(mkId(r)) == v, ('cle', 'isPinned', (str(r), "is not eligible" if v else "is eligible")))])

        # Shorthands over UIFs (these return z3 objects)
        allowOrRedact = lambda c: Or(hasGuardOperation(c) == self.allow, hasGuardOperation(c) == self.redact)
        isInArctaint  = lambda fan, tnt, lvl: If(isFunctionAnnotation(fan), hasARCtaints(cdfForRemoteLevel(fan, lvl), tnt), False)
        xdedge        = lambda e: And(nodeEnclave(mkId(hasSource(e))) != nodeEnclave(mkId(hasDest(e))), nodeEnclave(mkId(hasSource(e))) != self.allEnclave, nodeEnclave(mkId(hasDest(e))) != self.allEnclave)
        taintsAgree   = lambda t1, t2: Or(t1 == t2, t1 == self.allLabel, t2 == self.allLabel)
        esTaint       = lambda e: taint(mkId(hasSource(e)))
        edTaint       = lambda e: taint(mkId(hasDest(e)))
        esFunTaint    = lambda e: taint(mkId(srcFun(e))) if sourceAnnotFun(e) else self.nullCleLabel
        edFunTaint    = lambda e: taint(mkId(dstFun(e))) if destAnnotFun(e)   else self.nullCleLabel
        esFunCdf      = lambda e: cdfForRemoteLevel(esFunTaint(e), hasLabelLevel(edTaint(e))) if sourceAnnotFun(e) else self.nullCdf
        edFunCdf      = lambda e: cdfForRemoteLevel(edFunTaint(e), hasLabelLevel(esTaint(e))) if destAnnotFun(e)   else self.nullCdf
        ftaint        = lambda n: taint(mkId(hasFunction(n))) if hasFunction(n) != 0 else self.nullCleLabel

        # Constraints (names match model.mzn)

        ## NODE CONSTRAINTS

        self.add([
            (
                nodeEnclave(mkId(n)) != self.nullEnclave,
                ('node', 'NodeHasEnclave', n)
            )
            for n in pdg.n['NodeIdx']
        ])

        self.add([
            (
                nodeEnclave(mkId(n)) == nodeEnclave(mkId(hasFunction(n))), 
                ('node', 'NodeEnclaveIsFunEnclave', n)
            )
            for n in pdg.n['NodeIdx'] if hasFunction(n) != 0
        ])

        self.add([
            (
                nodeEnclave(mkId(n)) == nodeEnclave(mkId(hasClass(n))),
                ('node', 'NodeEnclaveIsClassEnclave', n)
            )
            for n in pdg.n['NodeIdx'] if hasClass(n) != 0
        ])

        self.add([
            (
                hasLabelLevel(taint(mkId(n))) == hasEnclaveLevel(nodeEnclave(mkId(n))),
                ('node', 'NodeLevelAtEnclaveLevel', n)
            )
            for n in pdg.n['NodeIdx']
        ])

        self.add([
            (
                Implies(isFunctionAnnotation(taint(mkId(n))), isFun(n)), 
                ('node', 'FnAnnotationForFnOnly', n)
            )
            for n in pdg.n['NodeIdx']
        ])
        
        self.add([
            (
                Implies(isFunctionAnnotation(taint(mkId(n))), userAnnotatedNode(n)), 
                ('node', 'FnAnnotationByUserOnly', n)
            ) 
            for n in pdg.FunctionLike
        ])
        
        self.add([
            (
                Implies(userAnnotatedNode(n), isFunctionAnnotation(taint(mkId(n)))), 
                ('node', 'annotationOnFunctionIsFunAnnotation', n)
            ) 
            for n in pdg.FunctionLike
        ])
        
        self.add([
            (
                Implies(userAnnotatedNode(n), isFunctionAnnotation(taint(mkId(n))) == False), 
                ('node', 'annotationOnClassIsNodeAnnotation', n)
            ) 
            for n in pdg.n['Decl_Record']
        ])
        
        self.add([
            (
                Implies(userAnnotatedNode(n), isFunctionAnnotation(taint(mkId(n))) == False), 
                ('node', 'annotationOnFieldIsNodeAnnotation', n)
            ) 
            for n in pdg.n['Decl_Field']
        ])
        
        self.add([
            (
                Implies(userAnnotatedNode(hasFunction(n)) == False, taint(mkId(n)) == ftaint(n)), 
                ('node', 'UnannotatedFunContentTaintMatch', n)
            ) 
            for n in pdg.n['NodeIdx'] if hasFunction(n) != 0
        ])
        
        self.add([
            (
                Implies(userAnnotatedNode(hasClass(n)) == False, taint(mkId(n)) == taint(mkId(hasClass(n)))), 
                ('node', 'UnannotatedClassTaintsMatch', n)
            ) 
            for n in pdg.n['NodeIdx'] if hasClass(n) != 0
        ])
        
        self.add([
            (
                Implies(userAnnotatedNode(hasClass(n)) == False, userAnnotatedNode(n) == False), 
                ('node', 'noAnnotatedDataForUnannotatedClass', n)
            ) 
            for n in pdg.n['NodeIdx'] if hasClass(n) != 0
        ])
        
        self.add([
            (
                taint(mkId(n)) == taint(mkId(hasClass(n))), 
                ('node', 'unannotatedConstructorGetsClassTaint', n)
            ) 
            for n in pdg.n['Decl_Constructor'] if not userAnnotatedNode(n)
        ])
        
        self.add([
            (
                taint(mkId(n)) == taint(mkId(hasClass(n))), 
                ('node', 'unannotatedDestructorGetsClassTaint', n)
            ) 
            for n in pdg.n['Decl_Destructor'] if not userAnnotatedNode(n)
        ])
        
        self.add([
            (
                taint(mkId(n)) == taint(mkId(hasClass(n))), 
                ('node', 'unannotatedMethodGetsClassTaint', n)
            ) 
            for n in pdg.n['Decl_Method'] if not userAnnotatedNode(n)
        ])
        
        self.add([
            (
                Implies(
                    userAnnotatedNode(hasFunction(n)), 
                    isInArctaint(ftaint(n), taint(mkId(n)), hasLabelLevel(taint(mkId(n))))
                ),
                ('node', 'AnnotatedFunContentCoercible', n)
            ) 
            for n in pdg.n['NodeIdx'] if hasFunction(n) != 0 and not isFun(n)
        ])
        
        self.add([
            (
                True, # TODO
                ('node', 'annotatedConstructorReturnsClassTaint', n)
            ) 
            for n in pdg.n['Decl_Constructor']
        ])
        
        self.add([
            (
                Or(
                    And(hasClass(n) != 0, isPinned(hasClass(n)) == False), 
                    And(isClass(n), isPinned(n) == False)
                ) == (taint(mkId(n)) == self.allLabel),
                ('node', 'definitionForALL', n)
            ) 
            for n in pdg.n['NodeIdx']
        ])
        
        ## EDGE CONSTRAINTS

        self.add([
            (
                esTaint(e) == edTaint(e), 
                ('edge', 'inheritTaint', e)
            ) 
            for e in pdg.e['Struct_Inherit']
        ])
        
        self.add([
            (
                userAnnotatedNode(hasDest(e)) == False, 
                ('edge', 'FunctionPtrSinglyTainted', e)
            ) 
            for e in pdg.e['Data_PointsTo'] if isFun(hasDest(e))
        ])
        
        self.add([
            (
                xdedge(e) == False, 
                ('edge', 'NonCallRetControlEnclaveSafe', e)
            ) 
            for e in pdg.Control_EnclaveSafe
        ])
        
        self.add([
            (
                Implies(xdedge(e), userAnnotatedNode(hasDest(e))), 
                ('edge', 'XDCallBlest', e)
            ) 
            for e in pdg.Control_Invocation
        ])
        
        self.add([
            (
                Implies(xdedge(e), allowOrRedact(edFunCdf(e))), 
                ('edge', 'XDCallAllowed', e)
            ) 
            for e in pdg.Control_Invocation
        ])
        
        self.add([
            (
                Implies(xdedge(e), allowOrRedact(esFunCdf(e))), 
                ('edge', 'XDReturnAllowed', e)
            ) 
            for e in pdg.e['Control_Return']
        ])
        
        self.add([
            (
                xdedge(e) == False, 
                ('edge', 'EnclaveSafeDataEdges', e)
            ) 
            for e in pdg.Data_EnclaveSafe
        ])
        
        self.add([
            (
                Implies(xdedge(e), allowOrRedact(esFunCdf(e))), 
                ('edge', 'XDReturnDataAllowed', e)
            ) 
            for e in pdg.e['Data_Return']
        ])
        
        self.add([
            (
                Implies(
                    xdedge(e),
                    And(
                        allowOrRedact(cdfForRemoteLevel(edTaint(e), hasLabelLevel(esTaint(e)))), 
                        isFun(hasDest(e)) == False
                    )
                ), 
                ('edge', 'XDPointsToAllowed', e)
            ) 
            for e in pdg.e['Data_PointsTo']
        ])
        
        self.add([
            (
                esTaint(e) == edTaint(e), 
                ('edge', 'intraFunPointsToTaintsMatch', e)
            ) 
            for e in pdg.e['Data_PointsTo'] if intraFunEdge(e)
        ])
        
        self.add([
            (
                taintsAgree(esTaint(e), edTaint(e)),
                ('edge', 'externExternDataEdgeTaintsMatch', e)
            ) 
            for e in pdg.DataEdge if globalGlobalEdge(e)
        ])
        
        self.add([
            (
                taintsAgree(esTaint(e), edTaint(e)), 
                ('edge', 'externDataEdgeTaintsMatch', e)
            ) 
            for e in pdg.DataEdge if funExternEdge(e)
        ])
        
        self.add([
            (
                taintsAgree(esTaint(e), edTaint(e)), 
                ('edge', 'retEdgeFromUnannotatedTaintsMatch', e)
            ) 
            for e in pdg.e['Data_Return'] if not sourceAnnotFun(e)
        ])
        
        self.add([
            (
                Implies(xdedge(e) == False, hasRettaints(esFunCdf(e), edTaint(e))), 
                ('edge', 'returnNodeInRettaints', e)
            ) 
            for e in pdg.e['Data_Return'] if sourceAnnotFun(e)
        ])
        
        self.add([
            (
                True, # TODO
                ('edge', 'argumentToUnannotatedTaintsMatch', e)
            ) 
            for e in pdg.Control_Invocation if not destAnnotFun(e)
        ])
        
        self.add([
            (
                True, # TODO
                ('edge', 'argumentInArgtaints', e)
            ) 
            for e in pdg.Control_Invocation if destAnnotFun(e)
        ])
        
        self.add([
            (
                Implies(xdedge(e) == False, taintsAgree(esTaint(e), edTaint(e))), 
                ('edge', 'interFunPointsToTaintsMatch', e)
            ) 
            for e in pdg.e['Data_PointsTo'] if interFunEdge(e)
        ])
        
        elapsed()

def run_z3(pgraph, cle, solver, minimize_core, log_constraints, temp_dir):

    # Encode constraints
    print("encoding...")
    ca = ConflictAnalyzer(pgraph, cle, solver, minimize_core)

    # Write constraints (can take a long time)
    if log_constraints:
        print("building sexpr...")
        exp = ca.s.sexpr()
        print("dumping sexpr to 'instance.smt2'...")
        with open(temp_dir / 'instance.smt2', 'w') as out: out.write(exp)

    # Solve
    res = ca.solve()

    # Write results
    print(str(res))
    if res == sat:
        with open(temp_dir / 'results.txt', 'w') as out: 
            ca.evidence(out)
        print("see 'results.txt' for solution")
    else:
        with open(temp_dir / 'explain_unsat.smt2', 'w') as out:          
            ca.explain(temp_dir, out)
        print("see 'core.smt2' and 'explain_unsat.smt2' for unsat core and explanation")