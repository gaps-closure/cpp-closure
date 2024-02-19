import subprocess
import os

def cle_to_mzn(cle):

    # string conversion helpers
    flat = lambda l: [i for sl in l for i in sl]
    def mkMznSet(n, br_open, eles):
        br_close = ']' if br_open == '[' else '}'
        return '{} = {} {} {};'.format(n, br_open, ', '.join(eles), br_close)
    def mkMznArr(n, dims, eles):
        d = len(dims)
        eles_s = ""
        if d == 2: eles_s = ",\n  ".join([", ".join(e) for e in eles])
        else:      eles_s = ",\n  ".join([", ".join(flat(e)) for e in eles])
        return '{} = array{}d({}, [\n  {}\n]);'.format(n, d, ", ".join(dims), eles_s)

    # enclave instance
    enc_s = '\n'.join([
        mkMznSet('Level', '{', cle.levels),
        mkMznSet('Enclave', '{', cle.enclaves),
        mkMznSet('hasEnclaveLevel', '[', cle.levels)
    ])

    # cle instance
    cle_s = '\n'.join([
        mkMznSet('cleLabel', '{', cle.cle_labels),
        mkMznSet('hasLabelLevel', '[', cle.label_levels),
        mkMznSet('isFunctionAnnotation', '[', cle.is_fun_annotation),
        mkMznSet('cdf', '{', cle.all_cdfs),
        mkMznSet('fromCleLabel', '[', cle.from_cle_label),
        mkMznSet('hasRemotelevel', '[', cle.has_remote_level),
        mkMznSet('hasDirection', '[', cle.has_direction),
        mkMznSet('hasGuardOperation', '[', cle.has_guard_op),
        mkMznSet('isOneway', '[', cle.is_oneway),
        mkMznArr('cdfForRemoteLevel', ['cleLabel', 'Level'], cle.cdf_for_remote_level),
        mkMznArr('hasRettaints', ['cdf', 'cleLabel'], cle.has_rettaints),
        mkMznArr('hasCodtaints', ['cdf', 'cleLabel'], cle.has_codtaints),
        mkMznArr('hasArgtaints', ['cdf', 'paramIdx', 'cleLabel'], cle.has_argtaints),
        mkMznArr('hasARCtaints', ['cdf', 'cleLabel'], cle.has_arctaints)
    ])

    return enc_s + "\n\n" + cle_s

def pgraph_to_mzn(pgraph):

    instance = []

    # Generate relations
    def addArr(n, entries):
        instance.append("{} = [".format(n))
        instance.append(",".join([str(e).lower() for e in entries]))
        instance.append("];")

    for n in pgraph.n:
        dmn = pgraph.n[n]
        if len(dmn) == 0: instance.append("{} = 0 .. -1;".format(n))
        else:             instance.append("{} = {} .. {};".format(n, str(dmn[0]), str(dmn[-1])))
    for e in pgraph.e:
        dmn = pgraph.e[e]
        if len(dmn) == 0: instance.append("{} = 0 .. -1;".format(e))
        else:             instance.append("{} = {} .. {};".format(e, str(dmn[0]), str(dmn[-1])))

    addArr("isGlobal", pgraph.isGlobal)
    addArr("hasClass", pgraph.hasClass)
    addArr("userAnnotatedNode", pgraph.userAnnotatedNode)
    addArr("hasFunction", pgraph.hasFunction)
    addArr("hasSource", pgraph.hasSource)
    addArr("hasDest", pgraph.hasDest)
    addArr("hasParamIdx", pgraph.hasParamIdx)

    # Max function parameters
    instance.append("MaxFnParams = {};".format(pgraph.MaxFnParams))

    # Taint constraints
    for n in pgraph.nodeTaints:
        tnt = pgraph.nodeTaints[n]
        instance.append('constraint :: "TaintOnNodeIdx{}" taint[{}] = {};'.format(n, n, tnt))

    return "\n".join(instance)

def to_mzn_file(pgraph, cle):
    return cle_to_mzn(cle) + "\n\n" + pgraph_to_mzn(pgraph)

def run_mzn(pgraph, cle, temp_dir):

    model_mzn_file = os.environ["MODEL_MZN"] if "MODEL_MZN" in os.environ else "model.mzn"

    mzn_instance = to_mzn_file(pgraph, cle)
    with open(model_mzn_file, 'r') as f:
        mzn_model = f.read()
    with open(temp_dir / 'instance.mzn', 'w') as f:
        f.write(mzn_model + "\n\n" + mzn_instance)
    mzn_args = [ 'minizinc', '--no-optimize', '--solver', 'Gecode', temp_dir / 'instance.mzn' ] # TODO: Don't have gecode locally
    output = subprocess.run(mzn_args, capture_output=True, encoding='utf-8')
    if output.returncode != 0 or "Error" in output.stdout:
        print("MINIZINC FAILURE:")
    print(output.stdout)
    print(output.stderr)