#pragma once
#include "base/macros.hpp"
#include "base/logging.hpp"

#include "geometry/rect2d.hpp"

#include "std/atomic.hpp"
#include "std/vector.hpp"

namespace dp {


#define INCREMENT_CAPTURE_CNT(condition)                  \
do{ if(condition) {                                       \
    ScreenCapturer::Instance().IncrementCounter();        \
    LOG(LDEBUG, ("Incremented Capture Counter: ",         \
            ScreenCapturer::Instance().GetCounter()));    \
    }                                                     \
}while(false)

#define DECREMENT_CAPTURE_CNT(condition)                    \
do{ if(condition){                                          \
    ScreenCapturer::Instance().DecrementCounter();          \
    LOG(LDEBUG, ("Decremented Capture Counter: ",           \
            ScreenCapturer::Instance().GetCounter()));      \
    }                                                       \
}while(false)

class ScreenCapturer
{
public:

    using TSaveCallback = std::function<void(string const&filename,
            uint32_t w, uint32_t h, vector<unsigned char> const &buffer)>;

    static ScreenCapturer & Instance();

    void IncrementCounter();
    void DecrementCounter();
    bool CanCapture();
    void Capture(TSaveCallback const& fn);
    void StartCapture(string const & filename, m2::RectU32 const &rec);
    void EndCapture();
    bool IsActive() {return m_active.load(); }
    int32_t GetCounter() { return m_counter.load(); }
private:
    DISALLOW_COPY_AND_MOVE(ScreenCapturer);

    ScreenCapturer()
        : m_counter(0)
        , m_active(false)
    {}

    atomic<int32_t> m_counter;
    atomic<bool> m_active;
    string m_filename;
    m2::RectU32 m_rect;
    int32_t m_bucketNumber;

};
}

