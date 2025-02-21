/**
 * CriticalSection.h
 *
 * @author Patrick Lavigne
 */


#pragma once

#include <pico/sync.h>

namespace kilight::core {

    class CriticalSection final {
    public:
        class ScopeLock final {
            friend class CriticalSection;
        public:
            ScopeLock() = delete;

            ScopeLock(ScopeLock const &) = delete;

            ScopeLock(ScopeLock &&) = default;

            ~ScopeLock();

            ScopeLock & operator=(ScopeLock const & other) = delete;

            ScopeLock & operator=(ScopeLock && other) = default;

        private:
            explicit ScopeLock(CriticalSection * owner);

            CriticalSection * m_owner;
        };

        CriticalSection();

        CriticalSection(CriticalSection const &) = delete;

        CriticalSection(CriticalSection &&) = delete;

        ~CriticalSection();

        CriticalSection & operator=(CriticalSection const & other) = delete;

        CriticalSection & operator=(CriticalSection && other) = delete;

        void enter();

        void exit();

        [[nodiscard]]
        ScopeLock lock();

    private:
        critical_section_t m_criticalSection {};
    };

}
