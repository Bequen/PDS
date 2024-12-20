OUTPUT=distr
FILES=$(wildcard ./src/*.cpp) $(wildcard ./src/gui/*.cpp ./src/network/*.cpp ./src/chord/*.cpp ./src/raft/*.cpp ./src/mutex/*.cpp)

IMGUI_FILES=$(wildcard ./external/imgui/*.cpp) \
			./external/imgui/backends/imgui_impl_glfw.cpp \
			./external/imgui/backends/imgui_impl_opengl3.cpp

CPP_FILES=$(FILES) $(IMGUI_FILES)
OBJ_FILES=$(addprefix ./obj/, $(CPP_FILES:.cpp=.o))

INCLUDE=-I./external/imgui/ \
		-I./external/cpp-channel/include/ \
		-I./src/

raft: test
	./${OUTPUT} raft

chord: test
	./${OUTPUT} chord

mutex: test
	./${OUTPUT} mutex

.PHONY: test raft chord mutex
test: ${OUTPUT}

info:
	@echo "Object files"
	@echo ${OBJ_FILES}

	@echo "Source files"
	@echo ${CPP_FILES}

external/cpp-channel/include/channel.hpp:
	git submodule update --init --recursive

${OUTPUT}: ${OBJ_FILES}
	@${CXX} ${CXXFLAGS} -lglfw -lGL -g3 -Wall -o ${OUTPUT} ${OBJ_FILES}

obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	@${CXX} ${CXXFLAGS} ${INCLUDE} -g3 -Wall -o $@ -c $<
