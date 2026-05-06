#include <Verify/SignalHandler.h>
#include <Boost/BoostNetEngineServer.h>
#include "Example.pb.h"

int main()
{
    SignalHandler::InitSignal();

    // 패킷 처리를 어떻게 정의할지 구조 고민 필요.
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
