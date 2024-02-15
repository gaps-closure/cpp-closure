class ProgramGraph:

    def __init__(self, f_nodes_csv, f_edges_csv, max_fn_params):

        # Node and edge types, ordered
        node_types = [
            "Decl.Var", "Decl.Function", "Decl.Record", "Decl.Field", "Decl.Method", 
            "Decl.Param", "Decl.Constructor", "Decl.Destructor", "Stmt.Decl", "Stmt.Call", 
            "Stmt.Compound", "Stmt.Ref", "Stmt.Field", "Stmt.This", "Stmt.Return", "Stmt.Other"
        ],
        edge_types = [
            "Record.Field", "Record.Method", "Record.Constructor", "Record.Destructor", "Record.Inherit", 
            "Control.Return", "Control.Entry", "Control.FunctionInvocation", 
            "Control.MethodInvocation", "Control.ConstructorInvocation", "Control.DestructorInvocation",
            "Data.PointsTo", "Data.DefUse", "Data.ArgPass", "Data.Return", "Data.Object", 
            "Data.FieldAccess", "Data.InstanceOf", "Data.Decl", "Data.Child"
        ]

        self.MaxFnParams = max_fn_params
        self.hasFunction = []
        self.hasSource = []
        self.hasDest = []
        self.hasParamIdx = []
        self.userAnnotatedNode = []
        self.isGlobal = []
        self.hasClass = []
        self.nodeTaints = {}

        # Generate node and edge sets, store relations
        # TODO: fix indices to match nodes.csv and edges.csv
        instance = {}
        i = 0
        def addSet(t, start, end): instance[t] = (start, end)
        for t in node_types:
            type_start = i + 1
            present = False
            while i < len(self.nodes_csv) and self.nodes_csv[i][2] == t:
                e = self.nodes_csv[i]
                present = True
                mzn_id = i + 1
                assert mzn_id == int(e[1])
                if t == "Decl.Param":
                    p_idx = int(e[12])
                    self.hasParamIdx.append(p_idx)
                if t == "Decl.Var":
                    self.isGlobal.append(False)
                self.hasFunction.append(int(e[5]))
                if e[3] != "": self.nodeTaints[mzn_id] = e[3]
                self.userAnnotatedNode.append(e[3] != "")
                self.hasClass.append(0)
                i += 1
            if present: addSet(t, type_start, i)
            else:       addSet(t, 0, -1)
        addSet("NodeIdx", 1, i)

        i = 0
        for t in edge_types:
            type_start = i + 1
            present = False
            while i < len(self.edges_csv) and self.edges_csv[i][2] == t:
                e = self.edges_csv[i]
                present = True
                mzn_id = i + 1
                assert mzn_id == int(e[1])
                self.hasSource.append(int(e[6]))
                self.hasDest.append(int(e[7]))
                i += 1
            if present: addSet(t, type_start, i)
            else:       addSet(t, 0, -1)
        addSet("EdgeIdx", 1, i)

        # Initialize domains of node/edge types
        getDomain = lambda s: list(range(instance[s][0], instance[s][1] + 1))

        self.n = {n.replace('.', '_'): getDomain(n) for n in (node_types + 'NodeIdx')}
        self.e = {e.replace('.', '_'): getDomain(e) for e in (edge_types + 'EdgeIdx')}