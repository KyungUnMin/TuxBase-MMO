#include <Verify/SignalHandler.h>
#include <Boost/BoostNetEngineServer.h>
#include <EngineCommon/PacketDispatcher.h>

int main()
{
    SignalHandler::InitSignal();

    PacketDispatcher packetDispatcher;
    BoostNetEngineServer netEngine(packetDispatcher, 13000, 1024, 5);
    netEngine.Start();

    while (true)
    {
        if (0 == packetDispatcher.ProcessPackets(netEngine))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    netEngine.Stop();

    std::cout << "Hello\n";

    return 0;
}
