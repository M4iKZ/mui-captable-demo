
#include <Logger.hpp>

#include "CapTableApp.hpp"

int main()
{
    Pulse::setLogger();

    mUI::Demo::CapTable::CapTableApp app;
 
    if (!app.initialize()) return -1;
    app.run();
 
    Pulse::shutdownLogger();

    return 0;
}