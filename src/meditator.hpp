#pragma once

#include<boost/noncopyable.hpp>
#include<cassert>

class Colleague;
class Meditator {
    public:
    Meditator() noexcept = default;
    virtual ~Meditator() = default;

    virtual void create_colleagues() noexcept = 0;
    virtual void colleague_change(Colleague* colleague) noexcept = 0;
};

class Colleague : private boost::noncopyable {
    public:
    Colleague() noexcept
    :m_meditator(nullptr){};
    virtual ~Colleague() = default;

    virtual void set_meditator(Meditator* meditator) noexcept{
        assert(meditator != nullptr);
        m_meditator = meditator;
    }
    virtual void notify_change() noexcept{
        assert(m_meditator != nullptr);
        m_meditator->colleague_change(this);
    }
    private:
    Meditator* m_meditator;
};