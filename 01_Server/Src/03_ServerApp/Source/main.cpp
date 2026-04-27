#include <Boost/BoostNetEngine.h>

int main()
{
    BoostNetEngine netEngine(2, 1);
    netEngine.Start(12345);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    netEngine.Stop();

    return 0;
}
