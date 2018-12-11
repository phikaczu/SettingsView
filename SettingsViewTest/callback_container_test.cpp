#include "pch.h"

#include <callback_container.h>

#include <array>

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
    std::array<bool,3> called{};
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

    std::unique_ptr<callback_token<decltype(callback)>> callbackToken;

    {
        auto container = callback_container<decltype(callback)>::create_callback_container();
        callbackToken = std::make_unique<callback_token<decltype(callback)>>(container->register_callback(std::move(callback)));
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

    callback_token<lifetime_tracker> callbackToken;

    {
        bool destructed{false};
        lifetime_tracker callback(destructed);

        {
            auto container = callback_container<lifetime_tracker>::create_callback_container();
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