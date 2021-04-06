add_library(Declarative 
    DependencyGraph.cpp
    DeclarativeFunctionVisitor.cpp
)

include_directories(
    "${CMAKE_CURRENT_LIST_DIR}/../include"
)
