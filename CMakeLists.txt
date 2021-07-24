cmake_minimum_required (VERSION 2.6)
project (LoonyLambdaVariadicTemplateVisitor)
add_executable(CrazyLambdaVariadicTemplateVisitor lambda_visitor.cpp)
target_compile_options(CrazyLambdaVariadicTemplateVisitor PRIVATE --std=c++1y)

project (InsaneInterfaceWrapper)
add_executable(InsaneInterfaceWrapper interface_wrapper.cpp)
target_compile_options(InsaneInterfaceWrapper PRIVATE --std=c++1y)

project (ProPolymorphicVectorTemplate)
add_executable(CrazyPolymorphicVectorTemplate polymorphic_vector.cpp)
target_compile_options(CrazyPolymorphicVectorTemplate PRIVATE --std=c++1y)

project (GreatGenericLambdaTemplateVisitor)
add_executable(GenericLambdaTemplateVisitor lambda_visitor2.cpp)
target_compile_options(GenericLambdaTemplateVisitor PRIVATE --std=c++11 -ggdb)

project (NotSoGreatGlobalInterface)
add_executable(NotSoGreatGlobalInterface global_virtual_obj.cpp)
target_compile_options(NotSoGreatGlobalInterface PRIVATE --std=c++14 -ggdb)

project (ShootYourselfInTheFootWithVirtualFunctions)
add_executable(ShootYourselfInTheFootWithVirtualFunctions bad_inheritance_cast.cpp)
target_compile_options(ShootYourselfInTheFootWithVirtualFunctions PRIVATE --std=c++14 -ggdb)

project (SuperiorMultitypeRangesV3BasedPtree)
add_executable(SuperiorMultitypeRangesV3BasedPtree rangesv3_ptree.cpp)
target_compile_options(SuperiorMultitypeRangesV3BasedPtree PRIVATE --std=c++14 -ggdb)

project (ValueConcepts)
add_executable(ValueConcepts value_concepts.cpp)
target_compile_options(ValueConcepts PRIVATE --std=c++20 -ggdb)
