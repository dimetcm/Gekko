#include "callable.h"
#include <chrono>

class ClockCallable : public ICallable
{
public:
    ClockCallable()
        : m_creationTime(std::chrono::system_clock::now())
    {}

protected:
    virtual Value Call(const std::vector<Value>& arguments) const override
    {
        using namespace std::chrono;
        system_clock::time_point time = system_clock::now();
        double secondsPassed = duration_cast<milliseconds>(time - m_creationTime).count()/1000.0;
        return Value(secondsPassed);
    }

    virtual int Arity() const override{ return 0; }

    virtual std::string ToString() const override { return "<native clock fn>"; }

    std::chrono::system_clock::time_point m_creationTime;
};