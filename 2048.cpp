#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <termios.h>
#include <unistd.h>
#include <assert.h>

/*
 * Manages the terminal, makes it behave more game-like
 */
class Terminal
{
public:
    void Setup();
    void Restore();
    void ClearScreen();
    static Terminal* Instance();
private:
    static Terminal* ptrInstance;
    
    Terminal();

    bool isSetup;
    termios original;
};
Terminal *Terminal::ptrInstance = 0;

Terminal::Terminal()
{
    isSetup = false;
}

void Terminal::Setup()
{    
    assert(!isSetup);

    //Fetch the current configuration for restoration and use as the base
    tcgetattr(STDIN_FILENO, &original);

    termios changed = original;
    changed.c_lflag &= ~(ICANON | ECHO); //Exit canonical mode and disable echoing
    tcsetattr(STDIN_FILENO, TCSANOW, &changed);

    isSetup = true;
}

void Terminal::Restore()
{
    //Make sure that we are setup (and therefore original isn't just random memory)
    assert(isSetup);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &original);

    isSetup = false;
}

void Terminal::ClearScreen()
{
    // ^[[2J -- clears screen
    // ^[[0;0H -- moves cursor to 0l0c
    std::cout << "\033[2J\033[0;0H";
}

Terminal* Terminal::Instance()
{
    if(!ptrInstance)
    {
        ptrInstance = new Terminal();
    }
    return ptrInstance;
}

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Game
{
public:
    Game();
    ~Game();

    void DrawScreen();
    bool AddNewBlock();

    bool Move(Direction);

private:
    const static unsigned int boardWidth = 4;
    const static unsigned int boardHeight = 4;
    unsigned int** board;

    //Strings used to draw the game board
    const std::string strBlockTop = "\xe2\x94\x8c\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x90";
    const std::string strBlockMid = "\xe2\x94\x82    \xe2\x94\x82";
    const std::string strBlockBtm = "\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x98";
    //Constants used in drawing the board
    const unsigned int verticalOffset = 2;
    const unsigned int horizontalOffset = 2;
    const unsigned int verticalStep = 3;
    const unsigned int horizontalStep = 6;
};

Game::Game()
{
    //Setup the board
    board = new unsigned int* [boardWidth];
    for(unsigned int i = 0; i < boardWidth; i++)
    {
        board[i] = new unsigned int[boardHeight];
        for (unsigned int j = 0; j < boardHeight; j++)
        {
            board[i][j] = 0;
        }
    }
}

Game::~Game()
{
    //Get rid of the board
    for(unsigned int i = 0; i < boardWidth; i++)
    {
        delete [] board[i];
    }
    delete [] board;
}

void Game::DrawScreen()
{
    Terminal::Instance()->ClearScreen();

    //restore default colors
    std::cout << "\033[37;40m";

    //Draw the outer part
    for(unsigned int i = 0; i < boardHeight; i++)
    {
        //Top
        for(unsigned int j = 0; j < boardWidth; j++)
        {
            std::cout << strBlockTop;
        }
        std::cout << "\n";

        //Mid
        for(unsigned int j = 0; j < boardWidth; j++)
        {
            std::cout << strBlockMid;
        }
        std::cout << "\n";
        
        //Bottom
        for(unsigned int j = 0; j < boardWidth; j++)
        {
            std::cout << strBlockBtm;
        }
        std::cout << "\n";
    } 
    
    //Write the info
    std::cout << "Use arrow keys to move the blocks.\n";
    std::cout << "Press q to exit the game.\n";
    //Setup the cursor return 
    std::cout << "\033[s";
    
    //Fill out the blocks
    for(unsigned int i = 0; i < boardWidth; i++)
    {
        for(unsigned int j = 0; j < boardHeight; j++)
        {
            if(board[i][j] != 0)
            {
                //Calculate cursor location
                unsigned int x = i * horizontalStep + horizontalOffset;
                unsigned int y = j * verticalStep + verticalOffset;

                //Move the cursor
                std::cout << "\033[" << y << ";" << x << "H";

                //Reset intensity
                std::cout << "\033[0m";
                //Switch the color
                switch(board[i][j])
                {
                    case 2:
                        std::cout << "\033[31;40m";
                        break;
                    case 4:
                        std::cout << "\033[32;40m";
                        break;
                    case 8:
                        std::cout << "\033[33;40m";
                        break;
                    case 16:
                        std::cout << "\033[34;40m";
                        break;
                    case 32:
                        std::cout << "\033[35;40m";
                        break;
                    case 64:
                        std::cout << "\033[36;40m";
                        break;
                    case 128:
                        std::cout << "\033[37;40m";
                        break;
                    case 256:
                        std::cout << "\033[31;1;40m";
                        break;
                    case 512:
                        std::cout << "\033[32;1;40m";
                        break;
                    case 1024:
                        std::cout << "\033[33;1;40m";
                        break;
                    case 2048:
                        std::cout << "\033[34;1;40m";
                        break;
                    case 4096:
                        std::cout << "\033[35;1;40m";
                        break;
                    case 8192:
                        std::cout << "\033[36;1;40m";
                        break;
                }

                //Print the board value
                std::cout << board[i][j];
            }
        }
    }
    //Return cursor
    std::cout << "\033[u";
}

bool Game::Move(Direction direction)
{
    bool somethingMoved = false;

    //Create the array for moving data
    unsigned int boardMax = (boardWidth > boardHeight) ? boardWidth : boardHeight;
    unsigned int ** ptrsMovableData = new unsigned int * [boardMax];
    for(unsigned int i = 0; i < boardMax; i++)
    {
        ptrsMovableData[i] = 0;
    }

    //This function actually does the block shifting
    //If nothing was done it returns false
    //otherwise true
    auto stepRow = [boardMax](unsigned int ** row)->bool
    {
        bool somethingHappened = false;
        bool merged[boardMax] = {};
        for(unsigned int i = 1; i < boardMax; i++)
        {
            //No block so you can't do anything, continue
            if(*row[i] == 0) continue;

            //Find the first block behind or the end of the board
            unsigned int j;
            for(j = i - 1; j > 0; j--)
            {
                if(*row[j] != 0) break;
            }

            //If the cell behind this one is empty
            if(*row[j] == 0)
            {
                *row[j] = *row[i];
                *row[i] = 0;
                somethingHappened = true;
            }
            //If the cell behind contains the same block
            else if(*row[j] == *row[i] && !merged[j])
            {
                *row[j] += *row[i];
                *row[i] = 0;
                merged[j] = true;
                somethingHappened = true;
            }
            //We hit another block
            else if(i - 1 != j)
            {
                *row[j+1] = *row[i];
                *row[i] = 0;
                somethingHappened = true;
            }

        }

        return somethingHappened;
    };

    //Move the 'rows' in correct direction, all of them
    bool exit = false;
    unsigned int i = 0;
    while(!exit)
    {
        //Fill out the movable data with correct pointers
        switch(direction)
        {
        case DOWN:
            for(unsigned int j = 0; j < boardHeight; j++)
            {
                ptrsMovableData[j] = &board[i][boardHeight - j - 1];
            }
            if(i + 1 >= boardWidth) exit = true;
            break;
        case RIGHT:
            for(unsigned int j = 0; j < boardWidth; j++)
            {
                ptrsMovableData[j] = &board[boardWidth - j - 1][i];
            }
            if(i + 1 >= boardHeight) exit = true;
            break;
        case UP:
            for(unsigned int j = 0; j < boardWidth; j++)
            {
                ptrsMovableData[j] = &board[i][j];
            }
            if(i + 1 >= boardWidth) exit = true;
            break;
        case LEFT:
            for(unsigned int j = 0; j < boardWidth; j++)
            {
                ptrsMovableData[j] = &board[j][i];
            }
            if(i + 1 >= boardHeight) exit = true;
            break;
        }

        if(stepRow(ptrsMovableData)) somethingMoved = true;
        i++;
    }

    return somethingMoved;
}

/*
 * Returns true if succeded in adding a new block to the board
 * fialse if not
 */
bool Game::AddNewBlock()
{
    unsigned int ** freeCells = new unsigned int* [boardWidth*boardHeight];

    //Find the amount of free cells
    unsigned int freeCellsCnt = 0;
    for(unsigned int i = 0; i < boardWidth; i++)
    {
        for(unsigned int j = 0; j < boardHeight; j++)
        {
            if(board[i][j] == 0)
            {
                freeCells[freeCellsCnt] = &board[i][j];
                freeCellsCnt++;
            }
        }
    }

    if(freeCellsCnt == 0)
    {
        //Clean up
        delete [] freeCells;

        //function failed 
        return false;
    }
    
    //Special case since rand doesn't like 1
    if(freeCellsCnt == 1)
    {
        *freeCells[0] = 2;
    }
    //Chose a random free cell and set it to 2
    else
    {
        *freeCells[std::rand() % (freeCellsCnt - 1)] = 2;
    }

    //Clean up
    delete [] freeCells;

    return true;
}

int main()
{
    Terminal::Instance()->Setup();
 
    //Seed rng
    std::srand(std::time(0));

    Game game;
    game.AddNewBlock();
    game.DrawScreen();

    char cur = '\0';
    while(cur != 'q')
    {
        std::cin >> cur;
        if(cur == '\033')
        {
            //The arrow keys are an escaped sequence
            std::cin >> cur;
            if(cur == '[')
            {
                std::cin >> cur;
               
                bool moved = false;
                
                //Process the arrow keys
                if(cur == 'A' && game.Move(UP)) moved = true;
                if(cur == 'B' && game.Move(DOWN)) moved = true;
                if(cur == 'C' && game.Move(RIGHT)) moved = true;
                if(cur == 'D' && game.Move(LEFT)) moved = true;

                //If a turn has been done
                if(moved)
                {
                    game.AddNewBlock();
                    game.DrawScreen();
                }
                cur = '\0';
            }

            //Don't let a random q in escape sequence terminate the entire thing
            cur = '\0';
        }
    }

    Terminal::Instance()->Restore();

    return 0;
}
