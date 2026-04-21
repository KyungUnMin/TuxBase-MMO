#include "Boost/BoostNetEngine.h"

BoostNetEngine::BoostNetEngine()
    : m_isRun(false)
    , m_accepter(m_ioContext)
{
}

BoostNetEngine::~BoostNetEngine()
{
    Stop();
}

void BoostNetEngine::Start(const UINT16 port)
{
    CreateSessions();
    m_isRun.store(true);

    Listen(port);
    Update();
}

void BoostNetEngine::CreateSessions()
{
    m_sessions.reserve(kMaxSessionCount);
    for (UINT32 i = 0; i < kMaxSessionCount; ++i)
        m_sessions.emplace_back(*this, m_ioContext);
}

void BoostNetEngine::Listen(const UINT16 port)
{
    m_accepter.open(boost::asio::ip::tcp::v4());
    m_accepter.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
    m_accepter.listen();
    m_acceptThread.Start("ListenWorkerThread", [this]()
    {
        ListenWorkerThreadFunc();
    });
}

namespace
{
    template <typename T>
    using Stack = std::vector<T>;
}

void BoostNetEngine::ListenWorkerThreadFunc()
{
    // TODO : sessionPool을 스택에다가 만들면, 클라가 접속 끊을때 세션 풀에 다시 넣어줄 수 가 없음.
    // 세션풀을 멤버변수로 바꾸거나, 좀 더 다른 방법들을 고민해봐야 함
    // 세션풀을 멤버변수로 바꾸면, 넣고 뺴고 할 때 락 잡아야 함

    ASSERT(m_sessions.size() == kMaxSessionCount, "Session count is not equal to kMaxSessionCount");
    Stack<BoostSession*> sessionPool(kMaxSessionCount, nullptr);
    for (UINT32 i = 0; i < sessionPool.size(); ++i)
    {
        BoostSession& session = m_sessions[i];
        // TODO : 유효성 체크 및 초기화 됐는지 체크
        sessionPool[i] = &session;
    }

    while (m_isRun.load())
    {
        ASSERT(!sessionPool.empty(), "Session pool is empty");
        BoostSession* session = sessionPool.back();
        sessionPool.pop_back();
        ASSERT(session, "Session is null");

        ErrorCode errorCode;
        m_accepter.accept(session->GetSocket(), errorCode);
        if (errorCode)
        {
            sessionPool.push_back(session);

            // Stop()에 의한 정상 종료
            if (errorCode == boost::asio::error::operation_aborted)
                break;

            // LOG_ERROR("Accept error: {}", errorCode.message());
            continue;
        }

        session->Start();
    }
}

void BoostNetEngine::Update()
{
    // TODO : 스레드 풀을 만들어서 ioContext.run()을 실행
    m_ioContext.run(); // 일단 오늘은 빌드 되는지만 보자
}

void BoostNetEngine::Stop()
{
    bool expected = true;                                            // 기대값
    bool desired = false;                                            // 변경값
    if (false == m_isRun.compare_exchange_strong(expected, desired)) // 변경되었다면 true 반환
        return;

    m_accepter.close();
    m_acceptThread.Join();
}