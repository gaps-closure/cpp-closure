import json
import itertools

def die(msg):
    print(msg)
    exit(0)

def validate_cle(cle, max_fn_params, fn_args, no_oneway):

    # For each entry in the list...
    for e in cle:

        # 1. MUST HAVE A 'cle-label' AND 'cle-json'
        if 'cle-label' not in e or 'cle-json' not in e:
            die("JSON entries must have a 'cle-label' and 'cle-json'")

    # 2. ALL LABELS MUST BE UNIQUE
    labels_l = [e['cle-label'] for e in cle]
    labels = set(labels_l)
    if len(labels) != len(labels_l): die("CLE labels must be unique")

    # Get data labels for check #13 later
    def isData(e):
        if 'cdf' in e['cle-json'] and type(e['cle-json']['cdf']) is list:
            cdfs = e['cle-json']['cdf']
            if len(cdfs) > 0 and 'codtaints' in e['cle-json']['cdf'][0]:
                return False
        return True
    data_labels = [e['cle-label'] for e in filter(isData, cle)]

    # For each CLE json...
    for e in cle:
        js = e['cle-json']

        # Generic error handler
        def check(b, s):
            if b: die(e['cle-label'] + ": " + s)

        # 3. MUST HAVE A 'level'
        check('level' not in js, "CLE entry must have a 'level' in its 'cle-json'")

        # 4. 'level' MUST NOT BE 'nullLevel'
        check(js['level'] == 'nullLevel', "'level' may not be 'nullLevel'")

        # 5. IF 'cdf' IS PRESENT, IT MUST BE A NON-EMPTY LIST
        check('cdf' in js and (not (type(js['cdf']) is list) or len(js['cdf']) == 0),
              "'cdf' must be non-empty list if it is present")

        # For each CDF...
        cdfs = ([c.copy() for c in js['cdf']] if 'cdf' in js else [])
        prev_taints = None
        for c in cdfs:

            # 6. MUST HAVE A 'remotelevel', 'direction', 'guarddirective'
            check('remotelevel' not in c or 'direction' not in c or 'guarddirective' not in c,
                    "'cdf' must have a 'remotelevel', 'direction', and 'guarddirective'")

            # 7. REMOTE LEVEL MAY NOT BE 'nullLevel'
            check(c['remotelevel'] == 'nullLevel', "'remotelevel' may not be 'nullLevel'")

            # 8. IF 'oneway' IS PRESENT, THE PROGRAM MUST NOT USE THE RETURN VALUE
            check('oneway' in c and e['cle-label'] in no_oneway and c['oneway'] == True,
                  "'oneway' cdf associatd with a function whose return value is used in the PDG")
            
            # 9. 'direction' MUST BE ONE OF 'ingress', 'egress', 'bidirectional'
            #    (direction is not used by the solver)
            check(c['direction'] not in ['ingress', 'egress', 'bidirectional'],
                    "'direction' must be one of 'ingress', 'egress', 'bidirectional'")

            # 10. 'guarddirective' JSON MUST HAVE 'operation', MUST BE ONE OF 'allow', 'redact', 'deny'
            gd = c['guarddirective']
            check('operation' not in gd or gd['operation'] not in ['allow', 'redact', 'deny'],
                    "'guarddirective must have 'operation' which is 'allow', 'redact', or 'deny'")
    
            # 11. ALL OR NONE OF 'argtaints', 'codtaints', 'rettaints' MUST BE PRESENT
            all_taints = 'argtaints' in c and 'codtaints' in c and 'rettaints' in c
            any_taints = 'argtaints' in c or  'codtaints' in c or  'rettaints' in c
            check(any_taints and not all_taints, "'cdf' must have all taint types or no taints")

            # If there are taints...
            if any_taints:

                # 12. TAINT FIELDS MUST BE LISTS
                flat = lambda l: [i for sl in l for i in sl]
                areLs = lambda l: all([type(sl) is list for sl in l])
                all_lsts = areLs([c['argtaints'], c['codtaints'], c['rettaints']]) and areLs(c['argtaints'])
                check(not all_lsts, "taints must be lists")
                
                # 13. EACH TAINT MUST BE AN EXISTING DATA LABEL OR "TAG_[REQUEST|RESPONSE]_{suffix}"
                taints = c['codtaints'] + c['rettaints'] + flat(c['argtaints'])
                for t in taints:
                    check(t not in data_labels and t[:12] != "TAG_REQUEST_" and t[:13] != "TAG_RESPONSE_",
                          "Each taint must be a data label or 'TAG_[REQUEST|RESPONSE]_\{suffix\}'")

                # 14. THE LENGTH OF 'argtaints' MUST NOT EXCEED THE MAXIMUM FUNCTION ARGUMENTS
                check(len(c['argtaints']) > max_fn_params,
                    "'argtaints' length may not exceed maximum function args ({})".format(max_fn_params))
                
                # 15. THE LENGTH OF 'argtaints' MUST MATCH THE NUMBER OF ACTUAL ARGUMENTS IF PROVIDED
                check(e['cle-label'] in fn_args and len(c['argtaints']) != fn_args[e['cle-label']],
                      "'argtaints' length must match actual number of function arguments")
                
                # 16. IF MULTIPLE CDFS ARE PRESENT, THEY MUST HAVE THE SAME TAINTS
                taint_tpl = (c['codtaints'], c['rettaints'], c['argtaints'])
                check(prev_taints and prev_taints != taint_tpl,
                      "cdfs for a label must all have identical taints")
                prev_taints = taint_tpl
            else:
                prev_taints = (None, None, None)
                
        # 17. NO TWO CDFS FOR THE SAME LABEL MAY SHARE A REMOTE LEVEL
        rlevels = [c['remotelevel'] for c in cdfs]
        check(len(rlevels) != len(set(rlevels)), "cdf 'remotelevels' must be unique")

class CLE:

    def __init__(self, f_cle_json, max_fn_params):

        cle = json.loads(f_cle_json.read_text())
        fn_args = "" 
        one_way = "" 

        # Convert functionArgs file contents into a dictionary mapping labels to number of args
        def fnArgsToDict(fn_args):
            args_d = {}
            for l in fn_args.splitlines():
                fs = l.split()
                fn_anno, n_args = fs[0], fs[1]
                if len(fs) == 3: fn_anno, n_args = fs[1], fs[2]
                if fn_anno in args_d and args_d[fn_anno] != int(n_args):
                    die("Functions with different numbers of arguments use the same CLE label")
                args_d[fn_anno] = int(n_args)
            return args_d
        
        def oneWayToSet(one_way):
            ow_d = {}
            for l in one_way.splitlines():
                fs = l.split()
                fn_anno, ow = fs[0], fs[1]
                if len(fs) == 3: fn_anno, ow = fs[1], fs[2]
                bf = 1 if fn_anno not in ow_d else ow_d[fn_anno]
                ow_d[fn_anno] = min(int(ow), bf)
            return { anno for anno in ow_d if ow_d[anno] == 0 }
        
        validate_cle(cle, max_fn_params, fnArgsToDict(fn_args), oneWayToSet(one_way))      

        # populate level / enclave data
        nn_levels = list({e['cle-json']['level'] for e in cle})
        self.levels    = ['nullLevel'] + nn_levels
        self.enclaves  = ['nullEnclave'] + [l + '_E' for l in nn_levels]

        # determine the set of TAG labels
        flat      = lambda l: [i for sl in l for i in sl]
        self.isFn = lambda e: 'cdf' in e['cle-json'] and 'codtaints' in e['cle-json']['cdf'][0]
        fnTnts    = lambda e: flat([c['codtaints'] + c['rettaints'] + flat(c['argtaints']) for c in e['cle-json']['cdf']])
        taints    = set(flat([fnTnts(e) for e in cle if self.isFn(e)]))
        tags      = taints - {e['cle-label'] for e in cle}

        # add synthetic null, TAG, and DFLT labels to JSON copy,
        # and sort with function annotations in front (creates a continuous range for minizinc)
        cle = list(cle)
        cle.sort(key=self.isFn, reverse=True)
        def addSyntheticLabel(n, data, idx=None):
            label = {'cle-label': n, 'cle-json': data}
            if idx != None: cle.insert(idx, label)
            else: cle.append(label)

        addSyntheticLabel('nullCleLabel', {
            'level': 'nullLevel',
            'cdf': [{
                'remotelevel': 'nullLevel',
                'direction': 'nullDirection',
                'guarddirective': {'operation': 'nullGuardOperation'}
            }]
        }, idx=0)
        for t in tags: addSyntheticLabel(t, {'level': 'nullLevel'})

        # populate label data
        self.cle = cle
        self.gds = ['nullGuardOperation', 'allow', 'deny', 'redact']
        self.cle_labels = [e['cle-label'] for e in cle]
        self.label_levels = [e['cle-json']['level'] for e in cle]
        self.is_fun_annotation = [str(self.isFn(e)).lower() for e in cle]

        # # populate cdf data
        self.all_cdfs             = []
        self.from_cle_label       = []
        self.has_remote_level     = []
        self.has_direction        = []
        self.has_guard_op         = []
        self.is_oneway            = []
        self.cdf_for_remote_level = []
        self.has_rettaints        = []
        self.has_codtaints        = []
        self.has_arctaints        = []
        self.has_argtaints        = []
        for e in cle:

            # get the cdfs
            l = e['cle-label']
            cdfs = e['cle-json']['cdf'] if 'cdf' in e['cle-json'] else []
            cdfs = list(filter(lambda c: c['remotelevel'] in self.levels, cdfs))

            # for each label, we map the cdfs to their remotelevel
            # cdf_at_level is populated below
            cdf_at_level = ['nullCdf'] * len(self.levels)
            self.cdf_for_remote_level.append(cdf_at_level)

            # iterate over cdfs to collect general and taint data
            for cdf, i in zip(cdfs, itertools.count(0)):
                cdf_id = l + '_cdf_' + str(i) if l != 'nullCleLabel' else 'nullCdf'
                    
                # general cdf data
                self.all_cdfs.append(cdf_id)
                self.from_cle_label.append(l)
                self.has_remote_level.append(cdf['remotelevel'])
                self.has_direction.append(cdf['direction'])
                self.has_guard_op.append(cdf['guarddirective']['operation'])
                self.is_oneway.append(cdf['oneway'] if 'oneway' in cdf else 'false')
                cdf_at_level[self.levels.index(cdf['remotelevel'])] = cdf_id

                # if it's a function label, collect taint data
                if self.isFn(e):

                    # codtaints, rettaints
                    cdf_arctaints = ['false'] * len(self.cle_labels)
                    def addTaints(name, ts):
                        cdf_taints = ['false'] * len(self.cle_labels)
                        for t in cdf[name]:
                            j = self.cle_labels.index(t)
                            cdf_taints[j] = 'true'
                            cdf_arctaints[j] = 'true'
                        ts.append(cdf_taints)
                    addTaints('rettaints', self.has_rettaints)
                    addTaints('codtaints', self.has_codtaints)
                    
                    # argtaints
                    cdf_argtaints = [['false'] * len(self.cle_labels) for _ in range(max_fn_params)]
                    for arg_ts, j in zip(cdf['argtaints'], itertools.count(0)):
                        for arg_t in arg_ts:
                            k = self.cle_labels.index(arg_t)
                            cdf_argtaints[j][k] = 'true'
                            cdf_arctaints[k] = 'true'
                    self.has_argtaints.append(cdf_argtaints)

                    # arctaints
                    cdf_arctaints[self.cle_labels.index(l)] = 'true'
                    self.has_arctaints.append(cdf_arctaints)

                # If not a function cdf, taints are all false
                else:
                    self.has_rettaints.append(['false'] * len(self.cle_labels))
                    self.has_codtaints.append(['false'] * len(self.cle_labels))
                    self.has_arctaints.append(['false'] * len(self.cle_labels))
                    self.has_argtaints.append([['false'] * len(self.cle_labels) for _ in range(max_fn_params)])