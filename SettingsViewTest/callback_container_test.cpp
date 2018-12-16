#include "pch.h"

#include <callback_container.h>

#include <array>
#include <condition_variable>
#include <functional>
#include <future>
#include <shared_mutex>

TEST(CallbackContainerTest, FactoryCreatesNonemptyObject)
{
    auto container = callback_container<void (*)(void)>::create_callback_container();
    ASSERT_TRUE(!!container);
}

TEST(CallbackContainerTest, RegisteredCallbackIsExecutedWhenTokenExists)
{
    bool called{false};
    auto callback = [&]() { called = true; };

    auto container = callback_container<decltype(callback)>::create_callback_container();
    auto callbackToken = container->register_callback(std::move(callback));
    (*container)();

    ASSERT_TRUE(called);
}

TEST(CallbackContainerTest, MultipleRegisteredCallbacksGetCalled)
{
    std::array<bool, 3> called{};
    auto callback0 = [&]() { called[0] = true; };
    auto callback1 = [&]() { called[1] = true; };
    auto callback2 = [&]() { called[2] = true; };

    auto container = callback_container<std::function<void(void)>>::create_callback_container();
    auto token0 = container->register_callback(std::move(callback0));
    auto token1 = container->register_callback(std::move(callback1));
    auto token2 = container->register_callback(std::move(callback2));

    (*container)();

    ASSERT_TRUE(called[0] && called[1] && called[2]);
}

TEST(CallbackContainerTest, RegisteredCallbackIsNotExecutedWhenTokenIsDestructed)
{
    bool called{false};
    auto callback = [&]() { called = true; };

    auto container = callback_container<decltype(callback)>::create_callback_container();
    (void)container->register_callback(std::move(callback));
    (*container)();

    ASSERT_FALSE(called);
}

TEST(CallbackContainerTest, CallbackTokenCanBeProperlyDestructedAfterCallbackContainerIsDestructed)
{
    bool called{false};
    auto callback = [&]() { called = true; };

    using container_t = callback_container<decltype(callback)>;
    std::unique_ptr<container_t::token_t> callbackToken;

    {
        auto container = container_t::create_callback_container();
        callbackToken = std::make_unique<container_t::token_t>(container->register_callback(std::move(callback)));
    }

    ASSERT_NO_THROW(callbackToken.reset());
}

TEST(CallbackContainerTest, CallbackTokenDoesNotPreventDestructionOfCallbackContainer)
{
    // ad-hoc, create something more general if needed
    class lifetime_tracker final
    {
    public:
        explicit lifetime_tracker(bool& destructed)
            : m_destructed{destructed}
            , m_movedFrom{false}
        {
        }

        lifetime_tracker(lifetime_tracker&& other)
            : m_destructed{other.m_destructed}
            , m_movedFrom{false}
        {
            other.m_movedFrom = true;
        }

        ~lifetime_tracker()
        {
            if (!m_movedFrom)
            {
                m_destructed = true;
            }
        }

        void operator()() {}

    private:
        bool& m_destructed;
        bool m_movedFrom;
    };

    using container_t = callback_container<lifetime_tracker>;
    container_t::token_t callbackToken;

    {
        bool destructed{false};
        lifetime_tracker callback(destructed);

        {
            auto container = container_t::create_callback_container();
            callbackToken = container->register_callback(std::move(callback));

            ASSERT_FALSE(destructed);
        }

        ASSERT_TRUE(destructed);
    }
}

TEST(CallbackContainerTest, UnregisteredCallbacksAreNotCalled)
{
    std::array<int, 4> called{};
    auto callback0 = [&]() { ++called[0]; };
    auto callback1 = [&]() { ++called[1]; };
    auto callback2 = [&]() { ++called[2]; };
    auto callback3 = [&]() { ++called[3]; };

    auto container = callback_container<std::function<void(void)>>::create_callback_container();
    auto token0 = container->register_callback(std::move(callback0));
    auto token1 = container->register_callback(std::move(callback1));
    auto token2 = container->register_callback(std::move(callback2));
    auto token3 = container->register_callback(std::move(callback3));

    (*container)();

    token1.unregister();
    token3.unregister();

    (*container)();

    ASSERT_EQ(2, called[0]);
    ASSERT_EQ(1, called[1]);
    ASSERT_EQ(2, called[2]);
    ASSERT_EQ(1, called[3]);
}

TEST(CallbackContainerTest, IfRecursiveMutexIsUsedThreadSafeMethodsCanBeCalledFromCallbacks)
{
    using container_t = callback_container<std::function<void(void)>, std::recursive_mutex>;

    auto container = container_t::create_callback_container();
    auto innerExecutedCount{0};
    auto innerCallback = [&]() { innerExecutedCount++; };
    container_t::token_t innerToken;
    auto callback = [&]() { innerToken = container->register_callback(innerCallback); };
    auto token = container->register_callback(std::move(callback));

    (*container)();
    // The inner callback was registered, but not executed
    ASSERT_EQ(0, innerExecutedCount);
    token.unregister();  // only one registration of the inner callback is required

    (*container)();

    ASSERT_EQ(1, innerExecutedCount);
}

TEST(CallbackContainerTest, IfSharedMutexIsUsedCallbacksCanBeCalledAsynchronously)
{
    static constexpr auto timeout = std::chrono::milliseconds(500);

    class condition final
    {
    public:
        void signal(const std::thread::id& thId) const
        {
            {
                std::lock_guard<std::mutex> lk(m_mtx);
                m_signalThreadId = thId;
                m_signaled = true;
            }
            m_cv.notify_one();
        }

        void wait(const std::thread::id& thId) const
        {
            std::unique_lock<std::mutex> lk(m_mtx);
            m_waitThreadId = thId;
            m_waitSuccessful = m_cv.wait_for(lk, timeout, [this] { return m_signaled; });
        }

        bool wait_successful() const
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            return m_waitSuccessful;
        }

        std::thread::id signal_thread_id() const
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            return m_signalThreadId;
        }

        std::thread::id wait_thread_id() const
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            return m_waitThreadId;
        }

    private:
        mutable std::mutex m_mtx;
        mutable std::condition_variable m_cv;
        mutable bool m_signaled{false};
        mutable bool m_waitSuccessful{false};
        mutable std::thread::id m_signalThreadId;
        mutable std::thread::id m_waitThreadId;
    };

    using container_t = callback_container<std::function<void(const condition&, const condition&)>, std::shared_mutex>;
    auto container = container_t::create_callback_container();
    condition c1, c2;

    auto callback = [](const condition& waitFor, const condition& signal)
    {
        signal.signal(std::this_thread::get_id());
        waitFor.wait(std::this_thread::get_id());
    };
    auto callbackToken = container->register_callback(std::move(callback));

    auto asyncMethod = [&](const condition& waitFor, const condition& signal) { container->operator()(waitFor, signal); };

    auto async1 = std::async(std::launch::async, asyncMethod, std::cref(c1), std::cref(c2));
    auto async2 = std::async(std::launch::async, asyncMethod, std::cref(c2), std::cref(c1));

    ASSERT_EQ(std::future_status::ready, async1.wait_for(timeout));
    ASSERT_EQ(std::future_status::ready, async2.wait_for(timeout));

    ASSERT_NE(c1.signal_thread_id(), c1.wait_thread_id()) << "callbacks were not executed on different threads";
    ASSERT_NE(c2.signal_thread_id(), c2.wait_thread_id()) << "callbacks were not executed on different threads";

    ASSERT_TRUE(c1.wait_successful());
    ASSERT_TRUE(c2.wait_successful());
}