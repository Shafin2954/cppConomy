#include "world.h"
#include "style.h"
#include "cli.h"

int main()
{
    styledTerminal::Init(); // Initialize terminal for color support (Windows)
    world world;
    cli cli_instance(world);
    cli_instance.run();
    return 0;
}