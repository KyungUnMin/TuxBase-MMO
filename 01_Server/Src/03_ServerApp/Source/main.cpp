#include <Verify/SignalHandler.h>
#include <Boost/BoostNetEngineServer.h>
#include "Example.pb.h"

int main()
{
    SignalHandler::InitSignal();

    Protocol::S2C_LoginResponse loginResponse;
    loginResponse.set_success(true);
    loginResponse.set_message("Welcome to TuxBase MMO!");

    // std::string serializedData;
    // loginResponse.SerializeToString(&serializedData);

    // BoostNetEngineServer netEngine(13000, 1024, 5);
    // netEngine.Start();
    // std::this_thread::sleep_for(std::chrono::seconds(10));
    // netEngine.Stop();

    return 0;
}
