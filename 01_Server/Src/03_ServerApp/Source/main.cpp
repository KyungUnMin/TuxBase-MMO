#include <Boost/BoostNetEngineServer.h>

int main()
{
    BoostNetEngineServer netEngine(12345, 1024, 5);
    netEngine.Start();
    std::this_thread::sleep_for(std::chrono::seconds(5000));
    netEngine.Stop();

    return 0;
}
