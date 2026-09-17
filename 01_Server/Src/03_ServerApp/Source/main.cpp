#include <Verify/SignalHandler.h>
#include <Boost/BoostNetEngineServer.h>

int main()
{
    SignalHandler::InitSignal();

    BoostNetEngineServer netEngine(13000, 1024, 5);
    netEngine.Start();
    std::this_thread::sleep_for(std::chrono::seconds(100000));
    netEngine.Stop();

    std::cout << "Hello\n";

    return 0;
}
