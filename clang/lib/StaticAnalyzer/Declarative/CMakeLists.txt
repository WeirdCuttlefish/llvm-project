set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
)

add_clang_library(clangStaticAnalyzerDeclarative
  CollectDeclRefExprVisitor.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers

  DEPENDS
  omp_gen
  )


