add_clang_executable(declarative-prototype DeclarativePrototype.cpp Andersen.h CallGraph.h DeclarativeCheckingFunctionVisitor.h)

target_link_libraries(declarative-prototype
  clangTooling
  clangBasic
)