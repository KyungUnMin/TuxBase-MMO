#include <Boost/BoostNetEngineServer.h>

int main()
{
    BoostNetEngineServer netEngine(2);
    netEngine.Start(12345);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    netEngine.Stop();

    return 0;
}
