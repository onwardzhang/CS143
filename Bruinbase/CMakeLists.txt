cmake_minimum_required(VERSION 3.6)
project(bruinbase)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    bruinbase
    Bruinbase.h
    BTreeIndex.cc
    BTreeIndex.h
    BTreeNode.cc
    BTreeNode.h
    lex.sql.c
    main.cc
    Makefile
    movie.del
    movie.tbl
    PageFile.cc
    PageFile.h
    RecordFile.cc
    RecordFile.h
    SqlEngine.cc
    SqlEngine.h
    SqlParser.l
    SqlParser.tab.c
    SqlParser.tab.h
    SqlParser.y)

add_executable(bruinbase ${SOURCE_FILES})