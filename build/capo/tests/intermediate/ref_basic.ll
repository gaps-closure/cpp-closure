; ModuleID = '/home/bflin/gaps/cpp-closure/build/capo/tests/cpp/ref_basic.cpp'
source_filename = "/home/bflin/gaps/cpp-closure/build/capo/tests/cpp/ref_basic.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !10 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32*, align 8
  store i32 0, i32* %1, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !16, metadata !DIExpression()), !dbg !17
  store i32 0, i32* %2, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata i32** %3, metadata !18, metadata !DIExpression()), !dbg !20
  store i32* %2, i32** %3, align 8, !dbg !20
  ret i32 0, !dbg !21
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { mustprogress noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "Ubuntu clang version 14.0.6", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "/home/bflin/gaps/cpp-closure/build/capo/tests/cpp/ref_basic.cpp", directory: "/home/bflin/gaps/cpp-closure/build/capo/tests", checksumkind: CSK_MD5, checksum: "a98aac5c54d70788a9b3abde9a88b263")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 1}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"Ubuntu clang version 14.0.6"}
!10 = distinct !DISubprogram(name: "main", scope: !11, file: !11, line: 1, type: !12, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!11 = !DIFile(filename: "cpp/ref_basic.cpp", directory: "/home/bflin/gaps/cpp-closure/build/capo/tests", checksumkind: CSK_MD5, checksum: "a98aac5c54d70788a9b3abde9a88b263")
!12 = !DISubroutineType(types: !13)
!13 = !{!14}
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{}
!16 = !DILocalVariable(name: "y", scope: !10, file: !11, line: 2, type: !14)
!17 = !DILocation(line: 2, column: 9, scope: !10)
!18 = !DILocalVariable(name: "x", scope: !10, file: !11, line: 3, type: !19)
!19 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !14, size: 64)
!20 = !DILocation(line: 3, column: 10, scope: !10)
!21 = !DILocation(line: 5, column: 5, scope: !10)
