linux:
	gcc demo/main.c $(wildcard ext/soil/src/*.c) $(wildcard ext/soil/src/SOIL/*.c) \
	$(wildcard ext/Chipmunk2D/src/constraints/*.c) $(wildcard ext/Chipmunk2D/src/*.c) \
	-Iext/Chipmunk2D/include \
	-o ./build/linux_example \
	-lGL -lglfw3 -lm -DNDEBUG