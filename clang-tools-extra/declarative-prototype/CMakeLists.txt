add_clang_executable(declarative-prototype DeclarativePrototype.cpp)

target_link_libraries(declarative-prototype 
  clangTooling
  clangBasic
)