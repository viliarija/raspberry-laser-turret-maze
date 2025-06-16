#ifndef GAME_H
#define GAME_H

#include <random>
#include <vector>

#define WALL_THICKNESS 2
#define MAZE_BIN "export/maze.bin"

struct Player {
    float x;
    float y;

    int cellX;
    int cellY;
};


class Game {
public:
    Game(int width, int height, int size);
    ~Game();

    void initialize();
    void render();
    void movePlayer(float dx, float dy);
    void getCoords(float coords[2]);
    void calib();
    void reset();

private:
    size_t index(int x, int y) const;
    void loadMazeFromFile(const char* filename);
    
    int mazeWidth;
    int mazeHeight;
    int cellSize;
    int windowWidth;
    int windowHeight;

    int calibration;
    
    char* maze = nullptr;
    
    Player player;

    std::mt19937 rng;
    
    enum Direction {
        DIR_TOP    = 1 << 0,
        DIR_RIGHT  = 1 << 1,
        DIR_BOTTOM = 1 << 2,
        DIR_LEFT   = 1 << 3
    };
    
    int dx[4] = { 0, 1, 0, -1 };
    int dy[4] = { -1, 0, 1, 0 };
    int opposite[4] = { DIR_BOTTOM, DIR_LEFT, DIR_TOP, DIR_RIGHT };
};

#endif
