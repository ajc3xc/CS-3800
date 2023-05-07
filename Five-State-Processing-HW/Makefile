CXX = g++
FLAGS = -g -W -Wall -Wextra -Wpedantic -std=c++11
LIBRARIES = -lpthread

.PHONY: default run

default: run

run:
        ${CXX} ${FLAGS} *.cpp ${LIBRARIES} -o program
        ./program
        make clean
clean:
        -@rm -rf *.o program core
