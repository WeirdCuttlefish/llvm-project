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
)