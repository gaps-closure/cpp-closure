; ModuleID = '/home/bflin/gaps/cpp-closure/build/capo/tests/cpp/struct_pointers.cpp'
source_filename = "/home/bflin/gaps/cpp-closure/build/capo/tests/cpp/struct_pointers.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.foo = type { i32 }

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !10 {
  %1 = alloca %struct.foo, align 4
  %2 = alloca i32*, align 8
  call void @llvm.dbg.declare(metadata %struct.foo* %1, metadata !16, metadata !DIExpression()), !dbg !20
  %3 = getelementptr inbounds %struct.foo, %struct.foo* %1, i32 0, i32 0, !dbg !21
  store i32 42, i32* %3, align 4, !dbg !22
  call void @llvm.dbg.declare(metadata i32** %2, metadata !23, metadata !DIExpression()), !dbg !25
  %4 = getelementptr inbounds %struct.foo, %struct.foo* %1, i32 0, i32 0, !dbg !26
  store i32* %4, i32** %2, align 8, !dbg !25
  ret i32 0, !dbg !27
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { mustprogress noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "Ubuntu clang version 14.0.6", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "/home/bflin/gaps/cpp-closure/build/capo/tests/cpp/struct_pointers.cpp", directory: "/home/bflin/gaps/cpp-closure/build/capo/tests", checksumkind: CSK_MD5, checksum: "7900f63b42891d9a8b8ec9453e0901a1")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 1}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"Ubuntu clang version 14.0.6"}
!10 = distinct !DISubprogram(name: "main", scope: !11, file: !11, line: 5, type: !12, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!11 = !DIFile(filename: "cpp/struct_pointers.cpp", directory: "/home/bflin/gaps/cpp-closure/build/capo/tests", checksumkind: CSK_MD5, checksum: "7900f63b42891d9a8b8ec9453e0901a1")
!12 = !DISubroutineType(types: !13)
!13 = !{!14}
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{}
!16 = !DILocalVariable(name: "x", scope: !10, file: !11, line: 6, type: !17)
!17 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "foo", file: !11, line: 1, size: 32, flags: DIFlagTypePassByValue, elements: !18, identifier: "_ZTS3foo")
!18 = !{!19}
!19 = !DIDerivedType(tag: DW_TAG_member, name: "bar", scope: !17, file: !11, line: 2, baseType: !14, size: 32)
!20 = !DILocation(line: 6, column: 16, scope: !10)
!21 = !DILocation(line: 7, column: 7, scope: !10)
!22 = !DILocation(line: 7, column: 11, scope: !10)
!23 = !DILocalVariable(name: "y", scope: !10, file: !11, line: 8, type: !24)
!24 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!25 = !DILocation(line: 8, column: 11, scope: !10)
!26 = !DILocation(line: 8, column: 18, scope: !10)
!27 = !DILocation(line: 9, column: 1, scope: !10)
