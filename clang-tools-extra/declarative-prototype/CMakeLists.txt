add_clang_executable(declarative-prototype DeclarativePrototype.cpp Andersen.h)

target_link_libraries(declarative-prototype
  clangTooling
  clangBasic
)