from collections import defaultdict
from dataclasses import dataclass
from typing import Dict, Generator, Iterable, List, Optional, Set, Tuple, Union
from intervaltree import IntervalTree # type: ignore[import-untyped]
from pathlib import Path
import argparse
import csv

SourceTree = Dict[str, IntervalTree]  

@dataclass
class GlobalID: 
    global_name: str
    def __hash__(self):
        return hash((self.global_name, -1, -1))

@dataclass
class InstID(GlobalID):
    inst_idx: int
    def __hash__(self):
        return hash((self.global_name, self.inst_idx, -1))

@dataclass
class ParamID(GlobalID):
    param_idx: int
    def __hash__(self):
        return hash((self.global_name, -1, self.param_idx))


LLID = Union[GlobalID, InstID, ParamID]

def llid_from_tuple(tuple: Tuple[str, Optional[int], Optional[int]]) -> LLID:
    global_name, inst_idx, param_idx = tuple
    if param_idx:
        return ParamID(global_name, param_idx)
    elif inst_idx:
        return InstID(global_name, inst_idx)
    else:
        return GlobalID(global_name)

def llid_from_str_tuple(tuple: Tuple[str, str, str]) -> LLID:
    return llid_from_tuple((tuple[0], int(tuple[1]) if tuple[1] else None, int(tuple[2]) if tuple[2] else None))


@dataclass 
class SourceRef:
    file: str
    offset: int

@dataclass
class PNode: 
    id: int 
    node_type: str 
    parent_decl: Optional[int]
    debug: Optional[str]
    annotation: Optional[str]
    range: Optional[Tuple[SourceRef, SourceRef]]
    param_idx: Optional[int]

    def add_to_tree(self, tree: SourceTree) -> None:
        if self.range:
            start, end = self.range
            tree[start.file][start.offset:end.offset] = self

@dataclass
class PEdge:
    edge_id: int
    edge_type: str
    src_id: int
    dst_id: int


class PGraph:
    nodes: List[PNode]
    edges: List[PEdge]
    source_tree: SourceTree 
    def __init__(self, nodes: List[PNode], edges: List[PEdge]):
        self.nodes = nodes
        self.edges = edges
        self.source_tree = defaultdict(lambda: IntervalTree())
        for node in self.nodes:
            if node.node_type.startswith('Decl'):
                node.add_to_tree(self.source_tree)


def pnodes(filename: Path) -> Generator[PNode, None, None]:
    with open(filename) as node_file: 
        for row in csv.reader(node_file):
            id = int(row[0])
            type = row[1]
            debug = row[2] or None
            annot = row[3] or None
            parent = int(row[4]) if row[4] else None
            file = row[-3]
            start_off = int(row[-2])
            end_off = int(row[-1])
            yield PNode(id, type, parent, debug, annot, (SourceRef(file, start_off), SourceRef(file, end_off + 1)), None)

def pedges(filename: Path) -> Generator[PEdge, None, None]:
    with open(filename) as edge_file: 
        for row in csv.reader(edge_file):
            id = int(row[0])
            type = row[1]
            src = int(row[-2])
            dst = int(row[-1])
            yield PEdge(id, type, src, dst) 


DeclRef = Tuple[LLID, SourceRef]
DeclMap = Dict[LLID, int]

def decl_refs(decl_map_filename: Path) -> Generator[DeclRef, None, None]:
    with open(decl_map_filename) as decl_map_file:
        for row in csv.reader(decl_map_file):
            tup: Tuple[str, str, str] = tuple(row[0:3]) # type: ignore[assignment]
            llid = llid_from_str_tuple(tup)
            filename = row[3]
            offset = int(row[4]) if row[4] else 0
            yield (llid, SourceRef(filename, offset))

def decl_map(graph: PGraph, decl_refs: Iterable[DeclRef]) -> DeclMap:
    def mk() -> Generator[Tuple[LLID, int], None, None]:
        for llid, ref in decl_refs:
            iset: set = graph.source_tree[ref.file][ref.offset]
            sorted_iset = sorted(iset, key=lambda i: (ref.offset - i.begin)**2 + (ref.offset - i.end)**2)
            if len(sorted_iset) > 0:
                node: PNode = sorted_iset[0].data
                yield (llid, node.id)
    return {llid: node_id for llid, node_id in mk()}

@dataclass
class SVFNode: 
    id: int
    type: str
    llid: LLID 

@dataclass
class SVFEdge:
    src: int
    dst: int

class PTGraph:
    nodes: Dict[int, SVFNode]
    edges: List[SVFEdge] 
    def __init__(self, nodes: Iterable[SVFNode], edges: Iterable[SVFEdge]):
        self.nodes = {node.id: node for node in nodes}
        self.edges = list(edges)


def svf_nodes(filename: Path) -> Generator[SVFNode, None, None]:
    with open(filename) as file:
        for row in csv.reader(file, quotechar="'"):
            id = int(row[0])
            type = row[1]
            tup: Tuple[str, str, str] = tuple(row[-3:]) # type: ignore[assignment]
            llid = llid_from_str_tuple(tup)
            yield SVFNode(id, type, llid)

def svf_edges(filename: Path) -> Generator[SVFEdge, None, None]:
    with open(filename) as file:
        for row in csv.reader(file):
            yield SVFEdge(int(row[0]), int(row[1]))

class Args:
    pnode_csv: Path
    pedge_csv: Path
    svf_node_csv: Path
    svf_edge_csv: Path
    declares_csv: Path


def get_args() -> Args:
    p = argparse.ArgumentParser(prog='points_to_edges')
    p.add_argument('--pnode-csv', type=Path)
    p.add_argument('--pedge-csv', type=Path)
    p.add_argument('--svf-node-csv', type=Path)
    p.add_argument('--svf-edge-csv', type=Path)
    p.add_argument('--declares-csv', type=Path)
    args = p.parse_args(namespace=Args())
    return args


def main() -> None:
    args = get_args()
    pgraph = PGraph(list(pnodes(args.pnode_csv)), list(pedges(args.pedge_csv)))
    ptgraph = PTGraph(list(svf_nodes(args.svf_node_csv)), list(svf_edges(args.svf_edge_csv)))

    # example query
    # print(sorted(pgraph.source_tree['foo.cpp'][15], key=lambda x: abs(15 - x.begin)))
    # print(ptgraph.nodes)

    refs = list(decl_refs(args.declares_csv))
    dmap = decl_map(pgraph, refs)
    edge_idx = len(pgraph.edges)
    for edge in ptgraph.edges:
        src_llid = ptgraph.nodes[edge.src].llid
        dst_llid = ptgraph.nodes[edge.dst].llid
        if src_llid in dmap and dst_llid in dmap:
            if dmap[src_llid] != dmap[dst_llid]:
                print(','.join([str(edge_idx), 'Data.PointsTo', str(dmap[src_llid]), str(dmap[dst_llid])]))
                edge_idx += 1


if __name__ == '__main__':
    main()