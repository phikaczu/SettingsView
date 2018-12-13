#include "pch.h"

#include <monitor.h>
#include "instance_tracker.h"

namespace
{
    enum class lock_type
    {
        unlocked,
        sharedlock,
        exclusivelock
    };

    class test_mutex : private tracked<test_mutex>
    {
    public:
        test_mutex()
            : m_lock{lock_type::unlocked}
            , m_lockCount{0}
            , m_unlockCount{0}
        {
        }

        void lock()
        {
            m_lockCount++;
            m_lock = lock_type::exclusivelock;
        }

        void unlock()
        {
            m_unlockCount++;
            m_lock = lock_type::unlocked;
        }

        lock_type state() const
        {
            return m_lock;
        }

        int lock_count() const
        {
            return m_lockCount;
        }

        int unlock_count() const
        {
            return m_unlockCount;
        }

    private:
        lock_type m_lock;
        int m_lockCount;
        int m_unlockCount;
    };

    class test_shared_mutex : private tracked<test_shared_mutex>
    {
    public:
        test_shared_mutex()
            : m_lock{lock_type::unlocked}
            , m_lockCount{0}
            , m_sharedLockCount{0}
            , m_unlockCount{0}
            , m_sharedUnlockCount{0}
        {
        }

        void lock()
        {
            m_lockCount++;
            m_lock = lock_type::exclusivelock;
        }

        void lock_shared()
        {
            m_sharedLockCount++;
            m_lock = lock_type::sharedlock;
        }

        void unlock()
        {
            m_unlockCount++;
            m_lock = lock_type::unlocked;
        }

        void unlock_shared()
        {
            m_sharedUnlockCount++;
            m_lock = lock_type::unlocked;
        }

        lock_type state() const
        {
            return m_lock;
        }

        int lock_count() const
        {
            return m_lockCount;
        }

        int unlock_count() const
        {
            return m_unlockCount;
        }

        int shared_lock_count() const
        {
            return m_sharedLockCount;
        }

        int shared_unlock_count() const
        {
            return m_sharedUnlockCount;
        }

    private:
        lock_type m_lock;
        int m_lockCount;
        int m_sharedLockCount;
        int m_unlockCount;
        int m_sharedUnlockCount;
    };
}  // namespace

TEST(MonitorWithSharedMutexTest, SharedLockIsUsedWhenTakingCopy)
{
    monitor<int, test_shared_mutex> m;
    const auto& mutexes = instance_tracker<test_shared_mutex>::get();
    ASSERT_EQ(1, mutexes.size());
    auto& mutex = mutexes.front().get();
    auto mutexState = mutex.state();
    ASSERT_EQ(lock_type::unlocked, mutexState);

    m([&](int x) { mutexState = mutex.state(); });
    ASSERT_EQ(lock_type::sharedlock, mutexState);

    mutexState = mutex.state();
    ASSERT_EQ(lock_type::unlocked, mutexState);

    ASSERT_EQ(1, mutexes.size());
    ASSERT_EQ(0, mutex.lock_count());
    ASSERT_EQ(0, mutex.unlock_count());
    ASSERT_EQ(1, mutex.shared_lock_count());
    ASSERT_EQ(1, mutex.shared_unlock_count());
}

TEST(MonitorWithSharedMutexTest, SharedLockIsUsedWhenTakingConstReference)
{
    monitor<int, test_shared_mutex> m;
    const auto& mutexes = instance_tracker<test_shared_mutex>::get();
    ASSERT_EQ(1, mutexes.size());
    auto& mutex = mutexes.front().get();
    auto mutexState = mutex.state();
    ASSERT_EQ(lock_type::unlocked, mutexState);

    m([&](const int& x) { mutexState = mutex.state(); });
    ASSERT_EQ(lock_type::sharedlock, mutexState);

    mutexState = mutex.state();
    ASSERT_EQ(lock_type::unlocked, mutexState);

    ASSERT_EQ(1, mutexes.size());
    ASSERT_EQ(0, mutex.lock_count());
    ASSERT_EQ(0, mutex.unlock_count());
    ASSERT_EQ(1, mutex.shared_lock_count());
    ASSERT_EQ(1, mutex.shared_unlock_count());
}

TEST(MonitorWithSharedMutexTest, ExclusiveLockIsUsedWhenTakingMutableReference)
{
    monitor<int, test_shared_mutex> m;
    const auto& mutexes = instance_tracker<test_shared_mutex>::get();
    ASSERT_EQ(1, mutexes.size());
    auto& mutex = mutexes.front().get();
    auto mutexState = mutex.state();
    ASSERT_EQ(lock_type::unlocked, mutexState);

    m([&](int& x) { mutexState = mutex.state(); });
    ASSERT_EQ(lock_type::exclusivelock, mutexState);

    mutexState = mutex.state();
    ASSERT_EQ(lock_type::unlocked, mutexState);

    ASSERT_EQ(1, mutexes.size());
    ASSERT_EQ(1, mutex.lock_count());
    ASSERT_EQ(1, mutex.unlock_count());
    ASSERT_EQ(0, mutex.shared_lock_count());
    ASSERT_EQ(0, mutex.shared_unlock_count());
}

template <typename T>
class MonitorWithMutexTest : public ::testing::Test
{
public:
    // Cannot use TypeParam in the test, because it seems that TypeParam type is decayed
    using type = T;
};

using MonitorArgumentTypes = ::testing::Types<int, int&, const int&>;
TYPED_TEST_CASE(MonitorWithMutexTest, MonitorArgumentTypes);

TYPED_TEST(MonitorWithMutexTest, ExclusiveLockIsUsedAlways)
{
    monitor<int, test_mutex> m;
    const auto& mutexes = instance_tracker<test_mutex>::get();
    ASSERT_EQ(1, mutexes.size());
    auto& mutex = mutexes.front().get();
    auto mutexState = mutex.state();
    ASSERT_EQ(lock_type::unlocked, mutexState);

    m([&](typename TestFixture::type x) { mutexState = mutex.state(); });
    ASSERT_EQ(lock_type::exclusivelock, mutexState);

    mutexState = mutex.state();
    ASSERT_EQ(lock_type::unlocked, mutexState);

    ASSERT_EQ(1, mutexes.size());
    ASSERT_EQ(1, mutex.lock_count());
    ASSERT_EQ(1, mutex.unlock_count());
}