mkdir -p build
g++ -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -I./glad -I./stb game.cpp -o build/river -I/usr/include/SDL2 -D_REENTRANT -lSDL2
