#include <Verify/SignalHandler.h>
#include <Boost/BoostNetEngineServer.h>
#include <EngineCommon/PacketDispatcher.h>

int main()
{
    SignalHandler::InitSignal();

    BoostNetEngineServer netEngine(13000, 1024, 5);
    PacketDispatcher packetDispatcher(netEngine);
    netEngine.Start();

    while (true)
    {
        if (0 == packetDispatcher.ProcessPackets())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    netEngine.Stop();

    std::cout << "Hello\n";

    return 0;
}
