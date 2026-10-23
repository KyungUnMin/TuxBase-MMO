#include <Verify/SignalHandler.h>
#include <Boost/BoostNetEngine.h>
#include "EngineCommon/Packet.h"
#include "Cli2Msv.pb.h"
#include "types.h"

/*
1. Session 연결되면 어딘가에 보관해두어야 소멸안됨
2. 서버 엔진도 힙에서 관리하는게 맞는것 같음. 다른 스레드에서 메인 스택에 접근하는건 별도 안 좋아보여
3. Session 연결되면 Start 함수 호출 필요 (인터페이스 뚫어야 함)
4. 패킷을 컨텐츠 단에서 바인딩 하는 방법에 대해 고민해보자
5. 여전히 strand 사용중일꺼임 사용 안 하는 방향으로
    아니네. 근데 송수신 처리 안 되어 있음. 세션에서 송수신 작업할 예정
*/

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
