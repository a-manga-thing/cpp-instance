//#include <ipfs/client.h>
#include <drogon/drogon.h>
#include "Globals.h"
#include "Util.h"

Globals globals;

int main (int argc, char** argv) {
    
    drogon::app().loadConfigFile("./config.json");
    
    globals.instance.url =
        drogon::app().getCustomConfig()["instance"].asString();
    
    drogon::app().run();
    
    return 0;
    
}
