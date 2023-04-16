mkdir -p build
g++ -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -I./glad -I./stb game.cpp -o build/river $(sdl2-config --cflags --libs)