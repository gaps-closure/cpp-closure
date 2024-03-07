bin/divider --extra-arg=-I../test/websrv/refactored/Utils --extra-arg=-I../test/websrv/refactored/Communications --extra-arg=-Wno-deprecated-declarations --extra-arg=-Wno-implicit-const-int-float-conversion ../test/websrv/topology.json --

bin/divider ../test/pp-trace/topology.json --

./pp-traceX  --callbacks="Pragma*,EndOfMainFile" ../tests/pragma.c  --

bin/divider --callbacks="Pragma*,EndOfMainFile" ../test/pp-trace/tests/pragma.c --

bin/divider ../test/pp-trace/topology.json --