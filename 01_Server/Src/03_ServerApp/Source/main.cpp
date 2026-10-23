#include <Verify/SignalHandler.h>
#include <Boost/BoostNetEngineServer.h>
#include "EngineCommon/Packet.h"
#include "Cli2Msv.pb.h"
#include "types.h"

int main()
{
    SignalHandler::InitSignal();

    // PacketDispatcher packetDispatcher;
    // packetDispatcher.RegisterPacket<proto::cli_to_msv::EnterMainServer>(proto::cli_to_msv::PacketID::ENTER_MAIN_SERVER, [](const SPtr<ISession>& session, UPtr<proto::cli_to_msv::EnterMainServer>&& packetBody) -> bool
    // {
    //     return true;
    // });


    BoostNetEngineServer netEngine(13000, 1024, 5);

    while (true)
    {
        // if (0 == packetDispatcher.ProcessPackets(netEngine))
        // {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // }
    }

    // netEngine.Stop();

    std::cout << "Hello\n";

    return 0;
}
