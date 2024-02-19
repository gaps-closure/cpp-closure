import argparse
from pathlib import Path
import csv
import sys

from cle import CLE
from graph import ProgramGraph
from mzn_solver import run_mzn
from z3_solver import run_z3

def parsed_args():
    parser = argparse.ArgumentParser("main")
    parser.add_argument('model', help="constraint model to use (supports 'mzn' or 'z3')", choices=['mzn', 'z3'])
    parser.add_argument('graph_nodes', help="path to program graph nodes as a .csv file", type=Path)
    parser.add_argument('graph_edges', help="path to program graph edges as a .csv file", type=Path)
    parser.add_argument('--max-fn-params', required=True, help="maximum number of function parameters", type=int)
    parser.add_argument('--cle-json',      required=True, type=Path, help='collated CLE JSON')
    parser.add_argument('--temp-dir',      required=True, help="relative path to store outputs", type=Path)
    args = parser.parse_args()
    args.graph_nodes = args.graph_nodes.resolve()
    args.graph_edges = args.graph_edges.resolve()
    return args

def main():

    args = parsed_args()

    print("building graph model and CLE model...\n")
    csv.field_size_limit(sys.maxsize)
    with open(args.graph_nodes) as f:
        nodes_csv = list(csv.reader(f, quotechar="'", skipinitialspace=True))
    with open(args.graph_edges) as f:
        edges_csv = list(csv.reader(f, quotechar="'", skipinitialspace=True))

    pgraph = ProgramGraph(nodes_csv, edges_csv, args.max_fn_params)
    cle = CLE(args.cle_json, args.max_fn_params)

    if args.model == 'mzn':
        print("== USING MINIZINC ==\n")
        run_mzn(pgraph, cle, args.temp_dir)
    else:
        print("===== USING Z3 =====\n")
        run_z3(pgraph, cle, 'int', True, False, args.temp_dir)

if __name__ == "__main__":
    main()

'''
inside cpp-closure/build/capo/solver, run:

rm -rf tmp && mkdir tmp && \
python3 main.py mzn fake-test/nodes.csv fake-test/edges.csv \
        --max_fn_params=10 \
        --cle-json=fake-test/collated.json \
        --function-args=fake-test/fargs.txt \
        --one-way=fake-test/oneway.txt \
        --temp_dir tmp
'''