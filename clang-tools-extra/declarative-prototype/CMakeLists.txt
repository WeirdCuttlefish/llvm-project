add_clang_executable(declarative-prototype DeclarativePrototype.cpp Andersen.h CallGraph.h)

target_link_libraries(declarative-prototype
  clangTooling
  clangBasic
)