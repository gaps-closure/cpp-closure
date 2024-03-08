import os
import sys
import glob
import subprocess
import re
from pathlib import Path
from typing import Optional

CLANG_14_EXECUTABLE = os.environ['CLANG_14_EXECUTABLE'] 
OPT_14_EXECUTABLE = os.environ['OPT_14_EXECUTABLE']

POINTS_TO_EDGES_SCRIPT = os.path.abspath('../points_to_edges.py')
DUMP_PTG_EXECUTABLE = os.path.abspath('../svf/Release-build/bin/dump-ptg')
SOLVER_SCRIPT = os.path.abspath('../solver/main.py')
MODEL_MZN = os.path.abspath('../solver/model.mzn')

CLANG_PLUGIN_SO = os.path.abspath(glob.glob('../clang-plugin/build/CLE.*').pop())
EXTRACT_DECLARES_PLUGIN_SO = os.path.abspath(glob.glob('../extract_declares/build/ExtractDeclares.*').pop())

CPP_TEST_SRC_DIR = os.path.abspath('cpp')

TMP_DIR = 'intermediate'
EDGES_CSV = 'edges.csv'
NODES_CSV = 'nodes.csv'
INSTANCE_MZN = 'instance.mzn'
COLLATED_JSON = 'collated.json'
SVF_EDGES_CSV = 'svf_edges.csv'
SVF_NODES_CSV = 'svf_nodes.csv'
DECLARES_CSV = 'declares.csv'
LL = 'll'
SVF_BC = 'svf.bc'


def compile_to_bitcode(cpp_file_name):
    command = (
        CLANG_14_EXECUTABLE,
        '-g',
        '-O0',
        '-S',
        '-Wno-unknown-attributes',
        '-emit-llvm',
        f'{CPP_TEST_SRC_DIR}/{cpp_file_name}'
    )
    status = subprocess.run(command, stdout=subprocess.PIPE)
    assert status.returncode == 0

    cpp_file_wo_ext = Path(cpp_file_name).stem
    os.makedirs(TMP_DIR, exist_ok=True)
    os.rename(f'{cpp_file_wo_ext}.ll', f'{TMP_DIR}/{cpp_file_wo_ext}.ll')

    return os.path.abspath(f'{TMP_DIR}/{cpp_file_wo_ext}.ll')


def run_clang_plugin(cpp_file_name):
    command = (
        CLANG_14_EXECUTABLE,
        '-g',
        '-c',
        '-Xclang', '-load',
        '-Xclang', CLANG_PLUGIN_SO,
        '-Xclang', '-plugin',
        '-Xclang', 'cle',
        f'{CPP_TEST_SRC_DIR}/{cpp_file_name}'
    )


    status = subprocess.run(command, stdout=subprocess.PIPE, env=os.environ)
    assert status.returncode == 0

    cpp_file_wo_ext = Path(cpp_file_name).stem
    os.makedirs(TMP_DIR, exist_ok=True)
    os.rename(EDGES_CSV, f'{TMP_DIR}/{cpp_file_wo_ext}-{EDGES_CSV}')
    os.rename(NODES_CSV, f'{TMP_DIR}/{cpp_file_wo_ext}-{NODES_CSV}')
    os.rename(COLLATED_JSON, f'{TMP_DIR}/{cpp_file_wo_ext}-{COLLATED_JSON}')

    return {
        EDGES_CSV: os.path.abspath(f'{TMP_DIR}/{cpp_file_wo_ext}-{EDGES_CSV}'),
        NODES_CSV: os.path.abspath(f'{TMP_DIR}/{cpp_file_wo_ext}-{NODES_CSV}'),
        COLLATED_JSON: os.path.abspath(f'{TMP_DIR}/{cpp_file_wo_ext}-{COLLATED_JSON}')
    }


def run_extract_declares_plugin(ll_file_path):
    command = (
        OPT_14_EXECUTABLE,
        '-disable-output',
        f'-load-pass-plugin={EXTRACT_DECLARES_PLUGIN_SO}',
        '-passes=declare-csv',
        ll_file_path
    )

    status = subprocess.run(command, stdout=subprocess.PIPE)
    assert status.returncode == 0

    ll_file_wo_ext = Path(ll_file_path).stem
    os.makedirs(TMP_DIR, exist_ok=True)
    os.rename(DECLARES_CSV, f'{TMP_DIR}/{ll_file_wo_ext}-{DECLARES_CSV}')

    return os.path.abspath(f'{TMP_DIR}/{ll_file_wo_ext}-{DECLARES_CSV}')

def run_dump_ptg(bitcode_file):
    ll_file_wo_ext = Path(bitcode_file).stem
    command = (
        DUMP_PTG_EXECUTABLE,
        '-ander',
        bitcode_file,
        SVF_NODES_CSV,
        SVF_EDGES_CSV
    )

    status = subprocess.run(command, stdout=subprocess.PIPE)
    assert status.returncode == 0

    os.rename(SVF_EDGES_CSV, f'{TMP_DIR}/{ll_file_wo_ext}-{SVF_EDGES_CSV}')
    os.rename(SVF_NODES_CSV, f'{TMP_DIR}/{ll_file_wo_ext}-{SVF_NODES_CSV}')

    return {
        SVF_BC: os.path.abspath(f'{TMP_DIR}/{ll_file_wo_ext}.svf.bc'),
        SVF_NODES_CSV: os.path.abspath(f'{TMP_DIR}/{ll_file_wo_ext}-{SVF_NODES_CSV}'),
        SVF_EDGES_CSV: os.path.abspath(f'{TMP_DIR}/{ll_file_wo_ext}-{SVF_EDGES_CSV}'),
    }


def run_points_to_edges(node_csv, edge_csv, svf_node_csv, svf_edge_csv, declares_csv):
    command = (
        sys.executable,
        POINTS_TO_EDGES_SCRIPT,
        '--pnode-csv', node_csv,
        '--pedge-csv', edge_csv,
        '--svf-node-csv', svf_node_csv,
        '--svf-edge-csv', svf_edge_csv,
        '--declares-csv', declares_csv
    )
    status = subprocess.run(command, stdout=subprocess.PIPE)
    assert status.returncode == 0

    return status.stdout 



def modify_edges(edge_csv, points_to_edges):
    with open(edge_csv, 'ab') as efile:
        efile.write(points_to_edges)


def gen_intermediate_files(cpp_file_name):
    file_paths = {}
    file_paths[LL] = compile_to_bitcode(cpp_file_name)
    file_paths.update(run_clang_plugin(cpp_file_name))
    file_paths.update(run_dump_ptg(file_paths[LL]))
    file_paths[DECLARES_CSV] = run_extract_declares_plugin(file_paths[LL])

    for file_path in file_paths.values():
        assert os.path.isfile(file_path)

    return file_paths

def run_solver(nodes_csv, edges_csv, collated_json, test_file_name, solver_type = "mzn") -> subprocess.CompletedProcess[bytes]:
    command = (
        sys.executable,
        SOLVER_SCRIPT,
        '--max-fn-params', '10',
        '--cle-json', collated_json,
        '--temp-dir', TMP_DIR,
        solver_type,
        nodes_csv, edges_csv
    )
    status = subprocess.run(command, stdout=subprocess.PIPE, env=dict(os.environ, **{"MODEL_MZN": MODEL_MZN}))

    assert status.returncode == 0 

    if solver_type == "mzn":
        instance_mzn = f'{TMP_DIR}/{test_file_name}-{INSTANCE_MZN}'
        os.rename(Path(TMP_DIR) / INSTANCE_MZN, instance_mzn)

    return status


def run_end_to_end(cpp_file_name) -> subprocess.CompletedProcess[bytes]:
    file_paths = gen_intermediate_files(cpp_file_name)
    point_to_edges = run_points_to_edges(file_paths[NODES_CSV], 
                                        file_paths[EDGES_CSV], 
                                        file_paths[SVF_NODES_CSV], 
                                        file_paths[SVF_EDGES_CSV], 
                                        file_paths[DECLARES_CSV])
    modify_edges(file_paths[EDGES_CSV], point_to_edges)  
    return run_solver(file_paths[NODES_CSV], file_paths[EDGES_CSV], file_paths[COLLATED_JSON], Path(cpp_file_name).stem)

def parse_solver_output(output: str) -> Optional[list[str]]:
    
    res = re.search(r'taint = \[((\w+)(,\s+)?)*\]', output)
    if res:
        return [ s.removeprefix("taint = [").removesuffix("]") for s in res.group(0).split(", ") ]
    else:
        return None 

def test_configuration():
    assert os.path.exists(CLANG_14_EXECUTABLE)
    assert os.path.exists(OPT_14_EXECUTABLE)
    assert os.path.exists(POINTS_TO_EDGES_SCRIPT)
    assert os.path.exists(DUMP_PTG_EXECUTABLE)

    assert os.path.exists(CLANG_PLUGIN_SO)
    assert os.path.exists(EXTRACT_DECLARES_PLUGIN_SO)

    assert os.path.isdir(CPP_TEST_SRC_DIR)

    status = subprocess.run((CLANG_14_EXECUTABLE, '--version'), stdout=subprocess.PIPE)
    assert status.returncode == 0
    assert b'clang version 14' in status.stdout

    status = subprocess.run((OPT_14_EXECUTABLE, '--version'), stdout=subprocess.PIPE)
    assert status.returncode == 0
    assert b'LLVM version 14' in status.stdout

    status = subprocess.run((DUMP_PTG_EXECUTABLE), stderr=subprocess.PIPE)
    assert b'Usage' in status.stderr

    status = subprocess.run((sys.executable, POINTS_TO_EDGES_SCRIPT, '--help'), stdout=subprocess.PIPE)
    assert status.returncode == 0


def test_alias_basic():
    file_paths = gen_intermediate_files('alias_basic.cpp')

    expected_output = b'9,Data.PointsTo,2,1\n10,Data.PointsTo,3,1\n'
    actual_output = run_points_to_edges(file_paths[NODES_CSV], 
                                        file_paths[EDGES_CSV], 
                                        file_paths[SVF_NODES_CSV], 
                                        file_paths[SVF_EDGES_CSV], 
                                        file_paths[DECLARES_CSV])
    assert actual_output == expected_output




def test_ref_basic():
    file_paths = gen_intermediate_files('ref_basic.cpp')

    expected_output = b'7,Data.PointsTo,2,1\n'
    actual_output = run_points_to_edges(file_paths[NODES_CSV], 
                                        file_paths[EDGES_CSV], 
                                        file_paths[SVF_NODES_CSV], 
                                        file_paths[SVF_EDGES_CSV], 
                                        file_paths[DECLARES_CSV])
    assert actual_output == expected_output


def test_e2e_alias_basic():
    output = run_end_to_end('alias_basic.cpp')
    taints = parse_solver_output(output.stdout.decode())
    assert taints
    assert all((taint == "GREEN" for taint in taints))

def test_e2e_class_basic():
    output = run_end_to_end('class_basic.cpp')
    print(output.stdout.decode())

def test_e2e_xd_only():
    output = run_end_to_end('xd_only.cpp')
    taints = parse_solver_output(output.stdout.decode())
    assert taints 

def test_e2e_xd_with_class():
    output = run_end_to_end('xd_with_class.cpp')
    taints = parse_solver_output(output.stdout.decode())
    assert taints
    assert "ALL" in taints

def test_e2e_annotated_class():
    output = run_end_to_end('annotated_class.cpp')
    taints = parse_solver_output(output.stdout.decode())
    assert taints

def test_e2e_simple_fail():
    output = run_end_to_end('simple_fail.cpp')
    assert output.stdout.decode().find("UNSATISFIABLE") != -1

def test_e2e_coerce():
    output = run_end_to_end('coerce.cpp')
    print(output.stdout.decode())
    taints = parse_solver_output(output.stdout.decode())
    assert taints
    assert all((taint in ["ORANGE", "ORANGE_SHAREABLE", "GET_FOO"] for taint in taints))

def test_struct_pointers():
    file_paths = gen_intermediate_files('struct_pointers.cpp')

    expected_output = b'12,Data.PointsTo,2,1\n'
    actual_output = run_points_to_edges(file_paths[NODES_CSV], 
                                        file_paths[EDGES_CSV], 
                                        file_paths[SVF_NODES_CSV], 
                                        file_paths[SVF_EDGES_CSV], 
                                        file_paths[DECLARES_CSV])
    assert actual_output == expected_output
