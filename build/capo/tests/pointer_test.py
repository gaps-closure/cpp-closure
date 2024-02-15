import os
import sys
import glob
import subprocess
from pathlib import Path

CLANG_14_EXECUTABLE = os.environ['CLANG_14_EXECUTABLE']
OPT_14_EXECUTABLE = os.environ['OPT_14_EXECUTABLE']

POINTS_TO_EDGES_SCRIPT = os.path.abspath('../points_to_edges.py')
DUMP_PTG_EXECUTABLE = os.path.abspath('../svf/Release-build/bin/dump-ptg')

CLANG_PLUGIN_SO = os.path.abspath(glob.glob('../clang-plugin/build/CLE.*').pop())
EXTRACT_DECLARES_PLUGIN_SO = os.path.abspath(glob.glob('../extract_declares/build/ExtractDeclares.*').pop())

CPP_TEST_SRC_DIR = os.path.abspath('cpp')

TMP_DIR = 'intermediate'
EDGES_CSV = 'edges.csv'
NODES_CSV = 'nodes.csv'
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

    status = subprocess.run(command, stdout=subprocess.PIPE)
    assert status.returncode == 0

    cpp_file_wo_ext = Path(cpp_file_name).stem
    os.makedirs(TMP_DIR, exist_ok=True)
    os.rename(EDGES_CSV, f'{TMP_DIR}/{cpp_file_wo_ext}-{EDGES_CSV}')
    os.rename(NODES_CSV, f'{TMP_DIR}/{cpp_file_wo_ext}-{NODES_CSV}')

    return {
        EDGES_CSV: os.path.abspath(f'{TMP_DIR}/{cpp_file_wo_ext}-{EDGES_CSV}'),
        NODES_CSV: os.path.abspath(f'{TMP_DIR}/{cpp_file_wo_ext}-{NODES_CSV}')
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


def gen_intermediate_files(cpp_file_name):
    file_paths = {}
    file_paths[LL] = compile_to_bitcode(cpp_file_name)
    file_paths.update(run_clang_plugin(cpp_file_name))
    file_paths.update(run_dump_ptg(file_paths[LL]))
    file_paths[DECLARES_CSV] = run_extract_declares_plugin(file_paths[LL])

    for file_path in file_paths.values():
        assert os.path.isfile(file_path)

    return file_paths


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

    expected_output = b'9,Data.PointsTo,5,3\n10,Data.PointsTo,7,3\n'
    actual_output = run_points_to_edges(file_paths[NODES_CSV], 
                                        file_paths[EDGES_CSV], 
                                        file_paths[SVF_NODES_CSV], 
                                        file_paths[SVF_EDGES_CSV], 
                                        file_paths[DECLARES_CSV])
    assert actual_output == expected_output


def test_ref_basic():
    file_paths = gen_intermediate_files('ref_basic.cpp')

    expected_output = b'7,Data.PointsTo,5,3\n'
    actual_output = run_points_to_edges(file_paths[NODES_CSV], 
                                        file_paths[EDGES_CSV], 
                                        file_paths[SVF_NODES_CSV], 
                                        file_paths[SVF_EDGES_CSV], 
                                        file_paths[DECLARES_CSV])
    assert actual_output == expected_output


def test_struct_pointers():
    file_paths = gen_intermediate_files('struct_pointers.cpp')

    expected_output = b'11,Data.PointsTo,11,1\n'
    actual_output = run_points_to_edges(file_paths[NODES_CSV], 
                                        file_paths[EDGES_CSV], 
                                        file_paths[SVF_NODES_CSV], 
                                        file_paths[SVF_EDGES_CSV], 
                                        file_paths[DECLARES_CSV])
    assert actual_output == expected_output
