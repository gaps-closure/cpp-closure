; ModuleID = '/home/bflin/gaps/cpp-closure/build/capo/tests/cpp/class_basic.cpp'
source_filename = "/home/bflin/gaps/cpp-closure/build/capo/tests/cpp/class_basic.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%class.Foo = type { i32 }

$_ZN3FooC2Ev = comdat any

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !26 {
  %1 = alloca i32, align 4
  %2 = alloca %class.Foo, align 4
  store i32 0, i32* %1, align 4
  call void @llvm.dbg.declare(metadata %class.Foo* %2, metadata !30, metadata !DIExpression()), !dbg !31
  call void @_ZN3FooC2Ev(%class.Foo* noundef nonnull align 4 dereferenceable(4) %2), !dbg !31
  ret i32 0, !dbg !32
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN3FooC2Ev(%class.Foo* noundef nonnull align 4 dereferenceable(4) %0) unnamed_addr #2 comdat align 2 !dbg !33 {
  %2 = alloca %class.Foo*, align 8
  store %class.Foo* %0, %class.Foo** %2, align 8
  call void @llvm.dbg.declare(metadata %class.Foo** %2, metadata !34, metadata !DIExpression()), !dbg !36
  %3 = load %class.Foo*, %class.Foo** %2, align 8
  %4 = getelementptr inbounds %class.Foo, %class.Foo* %3, i32 0, i32 0, !dbg !37
  store i32 0, i32* %4, align 4, !dbg !39
  ret void, !dbg !40
}

attributes #0 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!18, !19, !20, !21, !22, !23, !24}
!llvm.ident = !{!25}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "Ubuntu clang version 14.0.6", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "/home/bflin/gaps/cpp-closure/build/capo/tests/cpp/class_basic.cpp", directory: "/home/bflin/gaps/cpp-closure/build/capo/tests", checksumkind: CSK_MD5, checksum: "15a894bced049a0f51222ae376e7e325")
!2 = !{!3}
!3 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Foo", file: !4, line: 3, size: 32, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !5, identifier: "_ZTS3Foo")
!4 = !DIFile(filename: "cpp/class_basic.cpp", directory: "/home/bflin/gaps/cpp-closure/build/capo/tests", checksumkind: CSK_MD5, checksum: "15a894bced049a0f51222ae376e7e325")
!5 = !{!6, !8, !12, !15}
!6 = !DIDerivedType(tag: DW_TAG_member, name: "bar", scope: !3, file: !4, line: 6, baseType: !7, size: 32)
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !DISubprogram(name: "Foo", scope: !3, file: !4, line: 9, type: !9, scopeLine: 9, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!12 = !DISubprogram(name: "get_bar", linkageName: "_ZN3Foo7get_barEv", scope: !3, file: !4, line: 13, type: !13, scopeLine: 13, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!13 = !DISubroutineType(types: !14)
!14 = !{!7, !11}
!15 = !DISubprogram(name: "set_bar", linkageName: "_ZN3Foo7set_barEi", scope: !3, file: !4, line: 15, type: !16, scopeLine: 15, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!16 = !DISubroutineType(types: !17)
!17 = !{null, !11, !7}
!18 = !{i32 7, !"Dwarf Version", i32 5}
!19 = !{i32 2, !"Debug Info Version", i32 3}
!20 = !{i32 1, !"wchar_size", i32 4}
!21 = !{i32 7, !"PIC Level", i32 2}
!22 = !{i32 7, !"PIE Level", i32 2}
!23 = !{i32 7, !"uwtable", i32 1}
!24 = !{i32 7, !"frame-pointer", i32 2}
!25 = !{!"Ubuntu clang version 14.0.6"}
!26 = distinct !DISubprogram(name: "main", scope: !4, file: !4, line: 18, type: !27, scopeLine: 18, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !29)
!27 = !DISubroutineType(types: !28)
!28 = !{!7}
!29 = !{}
!30 = !DILocalVariable(name: "f", scope: !26, file: !4, line: 20, type: !3)
!31 = !DILocation(line: 20, column: 9, scope: !26)
!32 = !DILocation(line: 21, column: 5, scope: !26)
!33 = distinct !DISubprogram(name: "Foo", linkageName: "_ZN3FooC2Ev", scope: !3, file: !4, line: 9, type: !9, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !8, retainedNodes: !29)
!34 = !DILocalVariable(name: "this", arg: 1, scope: !33, type: !35, flags: DIFlagArtificial | DIFlagObjectPointer)
!35 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3, size: 64)
!36 = !DILocation(line: 0, scope: !33)
!37 = !DILocation(line: 10, column: 9, scope: !38)
!38 = distinct !DILexicalBlock(scope: !33, file: !4, line: 9, column: 11)
!39 = !DILocation(line: 10, column: 13, scope: !38)
!40 = !DILocation(line: 11, column: 5, scope: !33)
