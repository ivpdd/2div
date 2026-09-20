all:
	@+echo "remaking build dir"

	@rm -rf build
	@mkdir build

	@+echo "compiling linux"
	make linux

	@+echo "compiling windows"
	make windows
linux:
	@gcc main.c $(wildcard ../ext/soil/src/*.c) $(wildcard ../ext/soil/src/SOIL/*.c) \
	$(wildcard ../ext/Chipmunk2D/src/constraints/*.c) $(wildcard ../ext/Chipmunk2D/src/*.c) \
	-I../ext/Chipmunk2D/include \
	-o ./build/demo_linux \
	-lGL -lglfw3 -lm -DNDEBUG
windows:
	@x86_64-w64-mingw32-gcc main.c $(wildcard ../ext/soil/src/*.c) $(wildcard ../ext/soil/src/SOIL/*.c) \
	$(wildcard ../ext/Chipmunk2D/src/constraints/*.c) $(wildcard ../ext/Chipmunk2D/src/*.c) \
	-I../ext/Chipmunk2D/include \
	-I/home/iv/glfw/include \
	-L/home/iv/glfw/lib-mingw-w64 \
	-o ./build/demo_windows.exe \
	-lglfw3 -lopengl32 -lgdi32 -luser32 -lm -DNDEBUG \
	-DGL_CLAMP_TO_EDGE=0x812F \
	-Wno-builtin-macro-redefined