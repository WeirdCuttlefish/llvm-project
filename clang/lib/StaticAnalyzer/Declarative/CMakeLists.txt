set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
)

add_clang_library(clangStaticAnalyzerDeclarative
  CollectDeclRefExprVisitor.cpp
  DependencyGraph.cpp
  DeclarativeFunctionVisitor.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers

  DEPENDS
  omp_gen
  )


