/**
 * CriticalSection.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/core/CriticalSection.h"

#include <cassert>

namespace kilight::core {

    CriticalSection::ScopeLock::ScopeLock(CriticalSection* const owner) :
        m_owner(owner) {
        assert(m_owner != nullptr);
        m_owner->enter();
    }


    CriticalSection::ScopeLock::ScopeLock(ScopeLock&& other) noexcept :
        m_owner(other.m_owner) {
        other.m_owner = nullptr;
    }

    CriticalSection::ScopeLock::~ScopeLock() {
        m_owner->exit();
    }

    CriticalSection::CriticalSection() {
        critical_section_init(&m_criticalSection);
    }

    CriticalSection::~CriticalSection() {
        critical_section_deinit(&m_criticalSection);
    }

    void CriticalSection::enter() {
        critical_section_enter_blocking(&m_criticalSection);
    }

    void CriticalSection::exit() {
        critical_section_exit(&m_criticalSection);
    }

    CriticalSection::ScopeLock CriticalSection::lock() {
        return ScopeLock(this);
    }
}
