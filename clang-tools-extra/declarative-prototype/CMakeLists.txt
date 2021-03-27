add_subdirectory(lib)

add_clang_executable(declarative-prototype 
  DeclarativePrototype.cpp
)

target_link_libraries(declarative-prototype
  clangTooling
  clangBasic
  Declarative
)

target_include_directories(declarative-prototype
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
)
