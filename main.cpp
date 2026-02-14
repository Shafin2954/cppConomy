#include "world.h"
#include "style.h"
#include "cli.h"
#include <csignal>

// Global pointer to cli instance for signal handler
cli *gCliInstance = nullptr;

void signalHandler(int signum)
{
    if (gCliInstance)
    {
        gCliInstance->running = false;
    }
    std::cout << "\n"
              << styledTerminal::Styled("Goodbye!\n", styledTerminal::Theme::Success);
    exit(0);
}

int main()
{
    styledTerminal::Init(); // Initialize terminal for color support (Windows)
    world world;
    cli cli_instance(world);
    gCliInstance = &cli_instance;

    // Register signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);

    cli_instance.run();
    return 0;
}