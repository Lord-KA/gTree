# gTree
This is a generalized tree data structure with support of unlimited number of nodes' children

## Building
```bash
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make
```

## Using in your project
Add code below to your CMakeLists.txt and include "gtree.h"
```
FetchContent_Declare(
  gtree
  GIT_REPOSITORY https://github.com/Lord-KA/gTree.git
  GIT_TAG        release-1.X
)
if(NOT gtree_POPULATED)
  FetchContent_Populate(gtree)
  include_directories(${gtree_SOURCE_DIR})
endif()
```
You have to pre-define `GTREE_TYPE` with macro or `typedef` before including the header
To use gTree (re)storing you need to define three functions (read more in gtree.h)

## DONE
1. Basic abstract tree
2. Utility ObjPool data structure
3. GraphViz dumps
4. CMake with fetching
5. (Re)storing of a tree to/from a file in human-readalbe format
6. Modern-Art capybara ASCII

## TODO
1. Unit tests
2. Test coverage check
3. Github CI
