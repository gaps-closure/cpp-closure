## C++ conflict analyzer

The conflict analyzer take in a program graph representing a CLE-annotated C++ program, along with a JSON file defining each CLE label. The graph is given as two seperate `.csv` files, one for the nodes and one for the edges. The various node and edge types, and the `.csv` columns, are specified in `capo/graph_spec.md`.

It then uses MiniZinc or z3 to determine whether the annotated program satisfies CLE's taint coercion and cross-domain constraints and is eligible to be partitioned into two or more sub-programs running in different hardware enclaves.

The entry point is `main.py`. For a concise description of inputs, run `python3 main.py -h`.

## Dependencies

- `python`
- `minizinc`
- `z3` (`pip install z3-solver`)

## Structure

A python class for the graph model is given in `graph.py`, and a python class (and validator) for the CLE model is in `cle.py`. The conflict analyzer first reads the input `csv` and JSON files into these classes.

The conflict analyzer supports two isomorphic constraint models: one in MiniZinc (solved with Gecode) and one in smt-lib (expressed and solved using z3's python API). The MiniZinc constraints are given in `model.mzn`, and the z3 constraints are in `z3_solver.py` in the `encode()` function.

`mzn_solver.py` translates the `ProgramGraph` and `CLE` objects into a minizinc instance string, appends it `model.mzn`, and invokes `minizinc` to solve the instance. If the instance is satisfiable, the assigned enclave of each node is output.

`z3_solver.py` encodes the `ProgramGraph` and `CLE` objects as z3 integers, arrays, and uninterpreted functions, and solves the constraints via the z3 python API. If the instance is satisfiable, the assigned enclave of each node is output. If the instance is unsatisfiable, an explanation and the associated smt-lib unsat core are written to files. The translation of the unsat core into source-level descriptions is mediated by `provenance.py`.

## Making changes

### Adding a new node or edge type

- In `graph.py`, add the name of the node or edge type to the corresponding list in the correct order of appearance.
- Update `model.mzn` to include the new node or edge type. Don't forget to add it to any unions to which is ought to belong.
- If the new node or edge type belongs to any unions, update them near the top of the `encode()` function in `z3_solver.py`.

### Adding a new node or edge property

- In `graph.py`, add the property as an array below the node/edge types. The property should be indexed by node ID or edge ID, even if the property is technically only for a certain node or edge type.
- In `graph.py`, update the node/edge for-loop to append the correct property value from the `.csv` data as it iterates over each node/edge.
- In `mzn_solver.py`, update `pgraph_to_mzn()` to add the array and its values to the instance.
- In `model.mzn`, add a declaration for the property.
- In `z3_solver.py`, create a lambda for the property (see the "Helpers / Properties" comment)

### Adding or changing a constraint

- Add or update the constraint in `model.mzn`.
- Add or update the constraint in `z3_solver.py` at the bottom of the `encode()` function. Follow the syntax of the other constraint definitions (it is a bit obtuse), and make sure it is properly given a name and labeled as a node or edge constraint.
- If it is a new constraint: in `provenance.py`, in the `rules` dictionary, associate the name of the new constraint with an English description of what it constrains and why.