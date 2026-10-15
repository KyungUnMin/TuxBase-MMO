#include <Verify/SignalHandler.h>
#include <Boost/BoostNetEngineClient.h>
#include <EngineCommon/PacketDispatcher.h>

int main()
{
    SignalHandler::InitSignal();
    PacketDispatcher packetDispatcher;
    BoostNetEngineClient netEngine(packetDispatcher, "mainserver", 13000, 1024, 5);
    netEngine.Start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    netEngine.Stop();

    return 0;
}
