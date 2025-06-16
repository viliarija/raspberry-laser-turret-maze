#include "game.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <random>
#include <ctime>
#include <map>

Game::Game(int width, int height, int size)
    : mazeWidth(width), mazeHeight(height), 
      cellSize(size), windowWidth(width*size), 
      windowHeight(height*size), rng(std::time(nullptr)) 
{   
    // Initialize maze to 0
    maze = new char[mazeWidth * mazeHeight];
    memset(maze, 0, mazeWidth * mazeHeight * sizeof(char));

    // player(width * size / 2, height * size / 2),
    player.cellX = 0;
    player.cellY = 0;

    player.x = size * (player.cellX + 0.5);
    player.y = size * (player.cellY + 0.5);

    printf("%f - %f\n", player.x, player.y);
}

Game::~Game() 
{
    if (maze) {
        delete[] maze;
    }
}

// Find 2d object in a 1d array
size_t Game::index(int x, int y) const 
{
    return x + mazeWidth * y;
}

void Game::initialize() 
{
    loadMazeFromFile(MAZE_BIN);
}

void Game::loadMazeFromFile(const char* filename) {
    // Open the binary file in read mode
    std::ifstream inFile(filename, std::ios::binary);

    // Check if the file is open
    if (!inFile) {
        std::cerr << "Failed to open file for loading maze." << std::endl;
        return;
    }

    // Read maze dimensions (width and height)
    inFile.read(reinterpret_cast<char*>(&mazeWidth), sizeof(mazeWidth));
    inFile.read(reinterpret_cast<char*>(&mazeHeight), sizeof(mazeHeight));

    // Read maze data into the allocated memory
    inFile.read(maze, mazeWidth * mazeHeight);

    // Close the file
    inFile.close();

    std::cout << "Maze loaded from: " << filename << std::endl;
}

void Game::movePlayer(float dx, float dy) 
{
    if (calibration > 0) {
        if (calibration <= 16) {
            int dir = calibration % 4;
            float newX = player.x;
            float newY = player.y;

            switch(dir){
                case 0:
                    newX += 5;
                    break;
                case 1:
                    newY += 5;
                    break;
                case 2:
                    newX -= 5;
                    break;
                case 3:
                    newY -= 5;
                    break;
            }

            // Bounds
            if (newX < 0) {newX = 0; calibration += 1;}
            if (newY < 0) {newY = 0; calibration += 1;}
            if (newX > windowWidth) {newX = windowWidth; calibration += 1;}
            if (newY > windowHeight) {newY = windowHeight; calibration += 1;}

            player.x = newX;
            player.y = newY;
        } else {
            calibration = 0;
            reset();
        }
        return;
    }
    
    float newX = player.x + dx;
    float newY = player.y + dy;
    
    // Bounds
    if (newX <= WALL_THICKNESS) newX = WALL_THICKNESS;
    if (newY <= WALL_THICKNESS) newY = WALL_THICKNESS;
    if (newX >= (windowWidth - WALL_THICKNESS))  newX = windowWidth - WALL_THICKNESS;
    if (newY >= (windowHeight - WALL_THICKNESS)) newY = windowHeight - WALL_THICKNESS;

    // Cells
    char cell = maze[index(player.cellX, player.cellY)];

    if ((int)(newX / cellSize) < player.cellX) {
        if (!(cell & DIR_LEFT)) {
            newX = player.cellX * cellSize;
        } else { 
            player.cellX -= 1;
        }
    } else if ((int)(newX / cellSize) > player.cellX) {
        if (!(cell & DIR_RIGHT)) {
            newX = player.cellX * cellSize + (cellSize - 1);
        } else {
            player.cellX += 1;
        }
    }
    
    if ((int)(newY / cellSize) < player.cellY) {
        if (!(cell & DIR_TOP)) {
            newY = player.cellY * cellSize;
        } else { 
            player.cellY -= 1;
        }
    } else if ((int)(newY / cellSize) > player.cellY) {
        if (!(cell & DIR_BOTTOM)) {
            newY = player.cellY * cellSize + (cellSize - 1);
        } else { 
            player.cellY += 1;
        }
    }

    player.x = newX;
    player.y = newY;
}

void Game::getCoords(float coords[2])
{
    coords[0] = player.x;
    coords[1] = player.y;
}

void Game::reset()
{
    player.cellX = 0;
    player.cellY = 0;

    player.x = cellSize * (player.cellX + 0.5);
    player.y = cellSize * (player.cellY + 0.5);
}

void Game::calib()
{
    player.x = 0;
    player.y = 0;

    calibration = 1;
}
