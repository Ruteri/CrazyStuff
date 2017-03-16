cmake_minimum_required (VERSION 2.6)
project (CrazyLambdaVariadicTemplateVisitor)
add_executable(CrazyLambdaVariadicTemplateVisitor templatetest.cpp)
target_compile_options(CrazyLambdaVariadicTemplateVisitor PRIVATE --std=c++1y)

project (CrazyPolymorphicVectorTemplate)
add_executable(CrazyPolymorphicVectorTemplate polymorphic_vector.cpp)

target_compile_options(CrazyPolymorphicVectorTemplate PRIVATE --std=c++1y)
