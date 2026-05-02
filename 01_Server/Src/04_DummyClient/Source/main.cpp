#include <Verify/SignalHandler.h>
#include <Boost/BoostNetEngineClient.h>

int main()
{
    SignalHandler::InitSignal();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    BoostNetEngineClient netEngine("127.0.0.1", 12345, 1024, 5);
    netEngine.Start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    netEngine.Stop();

    return 0;
}
