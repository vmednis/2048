#include <iostream>
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

int main()
{
    Terminal::Instance()->Setup();
    Terminal::Instance()->ClearScreen();
    
    char cur = '\0';
    while(cur != 'q')
    {
        std::cin >> cur;
    }

    Terminal::Instance()->Restore();

    return 0;
}
