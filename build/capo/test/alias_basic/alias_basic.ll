; ModuleID = '/home/bflin/gaps/cpp-closure/build/capo/test/alias_basic/alias_basic.cpp'
source_filename = "/home/bflin/gaps/cpp-closure/build/capo/test/alias_basic/alias_basic.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !10 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32*, align 8
  %4 = alloca i32*, align 8
  store i32 0, i32* %1, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !16, metadata !DIExpression()), !dbg !17
  store i32 0, i32* %2, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata i32** %3, metadata !18, metadata !DIExpression()), !dbg !20
  store i32* %2, i32** %3, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata i32** %4, metadata !21, metadata !DIExpression()), !dbg !22
  %5 = load i32*, i32** %3, align 8, !dbg !23
  store i32* %5, i32** %4, align 8, !dbg !22
  ret i32 0, !dbg !24
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { mustprogress noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "Ubuntu clang version 14.0.6", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "/home/bflin/gaps/cpp-closure/build/capo/test/alias_basic/alias_basic.cpp", directory: "/home/bflin/gaps/cpp-closure/build/capo", checksumkind: CSK_MD5, checksum: "6b754178ece5639e886e0457f1b08e01")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 1}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"Ubuntu clang version 14.0.6"}
!10 = distinct !DISubprogram(name: "main", scope: !11, file: !11, line: 3, type: !12, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!11 = !DIFile(filename: "test/alias_basic/alias_basic.cpp", directory: "/home/bflin/gaps/cpp-closure/build/capo", checksumkind: CSK_MD5, checksum: "6b754178ece5639e886e0457f1b08e01")
!12 = !DISubroutineType(types: !13)
!13 = !{!14}
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{}
!16 = !DILocalVariable(name: "y", scope: !10, file: !11, line: 4, type: !14)
!17 = !DILocation(line: 4, column: 9, scope: !10)
!18 = !DILocalVariable(name: "x", scope: !10, file: !11, line: 5, type: !19)
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!20 = !DILocation(line: 5, column: 10, scope: !10)
!21 = !DILocalVariable(name: "z", scope: !10, file: !11, line: 6, type: !19)
!22 = !DILocation(line: 6, column: 10, scope: !10)
!23 = !DILocation(line: 6, column: 14, scope: !10)
!24 = !DILocation(line: 8, column: 5, scope: !10)
