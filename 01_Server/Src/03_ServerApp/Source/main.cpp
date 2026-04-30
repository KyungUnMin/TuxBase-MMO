#include <Boost/BoostNetEngineServer.h>

// TODO : 시그널 처리 고민해보기 (부스트에도 <signal.h> 같은 시그널 관련 라이브러리가 있대. ex : boost::asio::signal_set)
int main()
{
    BoostNetEngineServer netEngine(12345, 1024, 5);
    netEngine.Start();
    std::this_thread::sleep_for(std::chrono::seconds(5000));
    netEngine.Stop();

    return 0;
}
