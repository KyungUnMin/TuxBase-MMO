#include <Verify/SignalHandler.h>
#include <Boost/BoostNetEngineServer.h>
#include <EngineCommon/PacketDispatcher.h>
#include "Cli2Msv.pb.h"
#include "EngineCommon/PacketParser.h"
#include "types.h"

int main()
{
    SignalHandler::InitSignal();

    // PacketDispatcher packetDispatcher;
    // BoostNetEngineServer netEngine(packetDispatcher, 13000, 1024, 5);
    // netEngine.Start();

    PacketParser<proto::cli_to_msv::EnterMainServer> parser;

    // proto::cli_to_msv::PacketID_MAX;
    UINT16 testPacketID = static_cast<UINT16>(proto::cli_to_msv::PacketID::ENTER_MAIN_SERVER);
    proto::cli_to_msv::EnterMainServer testPacket;

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
