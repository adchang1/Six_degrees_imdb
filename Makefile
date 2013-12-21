#
# Based on Makefiles previously written by Julie Zelenski.
#

CXX = g++

# The CFLAGS variable sets compile flags for gcc:
#  -g                         compile with debug information
#  -Wall                      give all diagnostic warnings
#  -pedantic                  require compliance with ANSI standard
#  -O0                        do not optimize generated code
#  -std=c++0x                 go with the c++0x extensions for thread support, unordered maps, etc
CXXFLAGS = -g -Wall -pedantic -O0 -std=c++0x

# The LDFLAGS variable sets flags for linker
LDFLAGS =

HEADERS = imdb-utils.h

IMDB_CLASSES = imdb.cc
IMDB_HEADERS = $(HEADERS) $(IMDB_CLASSES:.cc=.h)

IMDB_TEST_HEADERS = $(IMDB_HEADERS)
IMDB_TEST_SOURCES = $(IMDB_CLASSES) imdb-unit-test.cc
IMDB_TEST_OBJECTS = $(IMDB_TEST_SOURCES:.cc=.o)
IMDB_TEST = imdb-unit-test

SIX_DEGREES_CLASSES = $(IMDB_CLASSES) path.cc
SIX_DEGREES_HEADERS = $(HEADERS) $(SIX_DEGREES_CLASSES:.cc=.h)
SIX_DEGREES_SOURCES = $(SIX_DEGREES_CLASSES) six-degrees.cc
SIX_DEGREES_OBJECTS = $(SIX_DEGREES_SOURCES:.cc=.o)
SIX_DEGREES = six-degrees

TARGETS = $(IMDB_TEST) $(SIX_DEGREES)
default : $(TARGETS)

$(IMDB_TEST) : $(IMDB_TEST_OBJECTS)
	$(CXX) -o $(IMDB_TEST) $(IMDB_TEST_OBJECTS) $(LDFLAGS)

$(SIX_DEGREES) : $(SIX_DEGREES_OBJECTS)
	$(CXX) -o $(SIX_DEGREES) $(SIX_DEGREES_OBJECTS) $(LDFLAGS)

# In make's default rules, a .o automatically depends on its .c file
# (so editing the .c will cause recompilation into its .o file).
# The line below creates additional dependencies, most notably that it
# will cause the .c to reocmpiled if any included .h file changes.

ALL_HEADERS = $(IMDB_TEST_HEADERS) $(SIX_DEGREES_HEADERS)
ALL_SOURCES = $(IMDB_TEST_SOURCES) $(SIX_DEGREES_SOURCES)

Makefile.dependencies:: $(ALL_SOURCES) $(ALL_HEADERS)
	$(CXX) $(CXXFLAGS) -MM $(ALL_SOURCES) > Makefile.dependencies

-include Makefile.dependencies

# Phony means not a "real" target, it doesn't build anything
# The phony target "clean" is used to remove all compiled object files.
# The phony target "spartan" is used to remove all compiled object files and ~files. 
.PHONY: clean spartan

clean : 
	/bin/rm -f *.o a.out $(IMDB_TEST) $(SIX_DEGREES) core Makefile.dependencies

spartan: clean
	rm -fr *~
