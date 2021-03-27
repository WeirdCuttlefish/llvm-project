add_subdirectory(lib)

add_clang_executable(declarative-prototype 
  DeclarativePrototype.cpp
  Andersen.h CallGraph.h
  CustomMatchers.h
  DeclarativeCheckingVisitor.h
  DeclarativeCheckingVisitorUtils.h
  Utils.h)

target_link_libraries(declarative-prototype
  clangTooling
  clangBasic
  Declarative
)

target_include_directories(declarative-prototype
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
)
