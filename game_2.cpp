#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <conio.h> 
#include <windows.h> 
#include <cstdlib> 
#include <ctime> // For random piece generation

using namespace std;


const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const int BLOCK_SIZE = 2; 

// Tetris piece - all possible shapes
const vector<vector<vector<bool>>> TETROMINOES = {
    // I
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // O
    {
        {1, 1},
        {1, 1}
    },
    // T
    {
        {0, 1, 0},
        {1, 1, 1},
        {0, 0, 0}
    },
    // S
    {
        {0, 1, 1},
        {1, 1, 0},
        {0, 0, 0}
    },
    // Z
    {
        {1, 1, 0},
        {0, 1, 1},
        {0, 0, 0}
    },
    // J
    {
        {1, 0, 0},
        {1, 1, 1},
        {0, 0, 0}
    },
    // L
    {
        {0, 0, 1},
        {1, 1, 1},
        {0, 0, 0}
    }
};


const vector<int> COLORS = {
    1,  // Blue (I)
    6,  // Yellow (O)
    5,  // Purple (T)
    2,  // Green (S)
    4,  // Red (Z)
    3,  // Cyan (J)
    14  // Bright Yellow (L)
};

class Tetris {
private:
    vector<vector<int>> board; // Game board (0 = empty, 1-7 = occupied by piece)
    vector<vector<bool>> currentPiece; 
    int currentX, currentY; 
    int currentColor; // Color of current piece
    int pieceType; // Type of current piece (0-6)
    int score;
    int level;
    int linesCleared;
    bool gameOver;
    int fallSpeed; 
    chrono::time_point<chrono::steady_clock> lastFall;

    // clear the console
    void clearScreen() {
        system("cls");
    }

    //  set console cursor position
    void setCursorPosition(int x, int y) {
        COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    // function to set console color
    void setColor(int color) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    }

    // Reset color to default-white
    void resetColor() {
        setColor(7); 
    }

    // Initialize the game
    void initBoard() {
        board.resize(BOARD_HEIGHT, vector<int>(BOARD_WIDTH, 0));
    }

    // Generate  random piece
    void newPiece() {
        pieceType = rand() % 7;
        currentPiece = TETROMINOES[pieceType];
        currentColor = COLORS[pieceType];
        currentX = BOARD_WIDTH / 2 - currentPiece[0].size() / 2;
        currentY = 0;

        //  game over condition (new piece can not be placed)
        if (checkCollision()) {
            gameOver = true;
        }
    }

    // Check if current piece collides with board boundaries or other pieces
    bool checkCollision() {
        for (int y = 0; y < currentPiece.size(); y++) {
            for (int x = 0; x < currentPiece[y].size(); x++) {
                if (currentPiece[y][x]) {
                    int boardX = currentX + x;
                    int boardY = currentY + y;

                    if (boardX < 0 || boardX >= BOARD_WIDTH || boardY >= BOARD_HEIGHT) {
                        return true;
                    }

                    if (boardY >= 0 && board[boardY][boardX]) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    // Rotate  piece 90 degrees clockwise
    void rotatePiece() {
        vector<vector<bool>> rotated(currentPiece[0].size(), vector<bool>(currentPiece.size()));
        for (int y = 0; y < currentPiece.size(); y++) {
            for (int x = 0; x < currentPiece[y].size(); x++) {
                rotated[x][currentPiece.size() - 1 - y] = currentPiece[y][x];
            }
        }

        vector<vector<bool>> oldPiece = currentPiece;
        currentPiece = rotated;

        // If rotation causes collision-convert to old shape
        if (checkCollision()) {
            currentPiece = oldPiece;
        }
    }

    // Merge the current piece into the board
    void mergePiece() {
        for (int y = 0; y < currentPiece.size(); y++) {
            for (int x = 0; x < currentPiece[y].size(); x++) {
                if (currentPiece[y][x]) {
                    int boardY = currentY + y;
                    int boardX = currentX + x;
                    if (boardY >= 0 && boardY < BOARD_HEIGHT && boardX >= 0 && boardX < BOARD_WIDTH) {
                        board[boardY][boardX] = currentColor;
                    }
                }
            }
        }
    }

    // Check for completed lines and remove it
    void clearLines() {
        int linesToClear = 0;
        for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
            bool lineComplete = true;
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board[y][x] == 0) {
                    lineComplete = false;
                    break;
                }
            }

            if (lineComplete) {
                // Remove line
                board.erase(board.begin() + y);
                board.insert(board.begin(), vector<int>(BOARD_WIDTH, 0));
                linesToClear++;
                y++; // row shifted down
            }
        }

        // Update score 
        if (linesToClear > 0) {
            linesCleared += linesToClear;
            switch (linesToClear) {
                case 1: score += 100 * level; break;
                case 2: score += 300 * level; break;
                case 3: score += 500 * level; break;
                case 4: score += 800 * level; break;
            }

            // Increase level every 10 lines
            level = 1 + linesCleared / 10;

            // Increase speed 
            fallSpeed = max(100, 1000 - (level * 100));
        }
    }

    
    void draw() {
        clearScreen();

        
        setColor(15); 
        cout << "+";
        for (int x = 0; x < BOARD_WIDTH * BLOCK_SIZE; x++) {
            cout << "-";
        }
        cout << "+\n";

        // Draw board 
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            cout << "|";
            for (int x = 0; x < BOARD_WIDTH; x++) {
                // Check if this position is part of the current piece
                bool isCurrentPiece = false;
                for (int py = 0; py < currentPiece.size(); py++) {
                    for (int px = 0; px < currentPiece[py].size(); px++) {
                        if (currentPiece[py][px] && 
                            currentX + px == x && 
                            currentY + py == y) {
                            isCurrentPiece = true;
                            break;
                        }
                    }
                    if (isCurrentPiece) break;
                }

                if (isCurrentPiece) {
                    setColor(currentColor);
                    for (int i = 0; i < BLOCK_SIZE; i++) {
                        cout << "#";
                    }
                } else if (board[y][x] != 0) {
                    setColor(board[y][x]);
                    for (int i = 0; i < BLOCK_SIZE; i++) {
                        cout << "#";
                    }
                } else {
                    resetColor();
                    for (int i = 0; i < BLOCK_SIZE; i++) {
                        cout << " ";
                    }
                }
            }
            resetColor();
            cout << "|\n";
        }

        // Draw bottom border
        setColor(15);
        cout << "+";
        for (int x = 0; x < BOARD_WIDTH * BLOCK_SIZE; x++) {
            cout << "-";
        }
        cout << "+\n";
        resetColor();

        // Draw score and level
        cout << "Score: " << score << "  Level: " << level << "  Lines: " << linesCleared << "\n";
        cout << "Controls: Arrow keys to move, Up to rotate, Space to drop\n";
    }

public:
    Tetris() {
        srand(time(0)); // random number generator
        initBoard();
        score = 0;
        level = 1;
        linesCleared = 0;
        gameOver = false;
        fallSpeed = 1000; // Start with 1 second between falls
        newPiece();
        lastFall = chrono::steady_clock::now();
    }

    // Handle user input
    void handleInput() {
        if (_kbhit()) {
            int key = _getch();

            // Handle arrow keys 
            if (key == 224 || key == 0) {
                key = _getch();
            }

            switch (key) {
                case 72: // Up arrow - rotate
                    rotatePiece();
                    break;
                case 75: // Left arrow - move left
                    currentX--;
                    if (checkCollision()) currentX++;
                    break;
                case 77: // Right arrow - move right
                    currentX++;
                    if (checkCollision()) currentX--;
                    break;
                case 80: // Down arrow - soft drop
                    currentY++;
                    if (checkCollision()) currentY--;
                    break;
                case 32: // Space - hard drop
                    while (!checkCollision()) {
                        currentY++;
                    }
                    currentY--;
                    mergePiece();
                    clearLines();
                    newPiece();
                    lastFall = chrono::steady_clock::now();
                    break;
                case 27: // ESC - pause or quit
                    gameOver = true;
                    break;
            }
        }
    }

    // Update state
    void update() {
        auto now = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastFall).count();

        if (elapsed >= fallSpeed) {
            currentY++;
            if (checkCollision()) {
                currentY--;
                mergePiece();
                clearLines();
                newPiece();
            }
            lastFall = now;
        }
    }

    // Main game 
    void run() {
        while (!gameOver) {
            draw();
            handleInput();
            update();
          
        }

        clearScreen();
        cout << "Game Over!\n";
        cout << "Final Score: " << score << "\n";
        cout << "Lines Cleared: " << linesCleared << "\n";
    }
};

int main() {
    
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r);
    MoveWindow(console, r.left, r.top, 800, 600, TRUE);

    Tetris game;
    game.run();
    return 0;
}
