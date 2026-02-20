#include "world.h"
#include "style.h"
#include "cli.h"

int main()
{
    styledTerminal::Init(); // Initialize terminal for color support (Windows)
    world world;
    cli cli_interface(world);
    cli_interface.run();
    return 0;
}