set(CLOSURE_TOOLS
    closure-divider
)

set(closure-divider_SOURCES
 "${CMAKE_CURRENT_SOURCE_DIR}/ClosureDividerMain.cpp"
 "${CMAKE_CURRENT_SOURCE_DIR}/ClosureDividerVisitor.cpp"
 "${CMAKE_CURRENT_SOURCE_DIR}/ClosureDividerMatcher.cpp"
 "${CMAKE_CURRENT_SOURCE_DIR}/Topology.cpp"
 "${CMAKE_CURRENT_SOURCE_DIR}/Annotation.cpp"
)

# CONFIGURE THE TOOLS
# ===================
foreach( tool ${CLOSURE_TOOLS} )
    # Create a library corresponding to 'plugin'
    add_executable(
      ${tool}
      ${${tool}_SOURCES}
      )

    # Configure include directories for 'tool'
    target_include_directories(
      ${tool}
      PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    )

    # Link in the required libraries 
    target_link_libraries(
      ${tool}
      "clangTooling"
    )
endforeach()