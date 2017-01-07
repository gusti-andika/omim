#include "screen_capturer.hpp"

#include "drape/glfunctions.hpp"

#include "std/vector.hpp"

#include <memory.h>

namespace dp {

struct Guard{
    ~Guard() {
        ScreenCapturer::Instance().EndCapture();
    }
};


ScreenCapturer & ScreenCapturer::Instance()
{
    static ScreenCapturer instance;
    return instance;
}

void ScreenCapturer::IncrementCounter()
{
    m_counter++;
}

void ScreenCapturer::DecrementCounter()
{
    m_counter--;
}

void ScreenCapturer::StartCapture(string const &filename, m2::RectU32 const &rect)
{
    m_active = true;
    m_counter = 0;
    m_filename = filename;
    m_rect = rect;
    LOG(LDEBUG, ("Start capture. ","counter:", GetCounter(), ",filename:", m_filename, ", rect:", m_rect));
}

void ScreenCapturer::EndCapture()
{
    m_active = false;
}

bool ScreenCapturer::CanCapture()
{
    return m_active && m_counter <= 0;
}

void ScreenCapturer::Capture(TSaveCallback const &fn)
{
    Guard guard;

    uint32_t x0 = m_rect.minX();
    uint32_t y0 = m_rect.minY();
    uint32_t w = m_rect.SizeX();
    uint32_t h = m_rect.SizeY();

    LOG(LDEBUG, ("Try capture map at viewport: ", m_rect));

    vector<unsigned char> temp(w * h * 4);
    GLFunctions::glReadPixels(x0, y0, w, h,
                       gl_const::GLRGBA, gl_const::GL8BitOnChannel,
                       &temp[0]);

    vector<unsigned char> flipped(w * h * 4);
    for (int i=0; i < h; i++)                 // Doesn't matter the order now
        memcpy(&flipped[i*w*4],               // address of destination
               &temp[(h-i-1)*w*4],            // address of source
               w*4*sizeof(unsigned char) );   // number of bytes to copy

    if (fn)
        fn(m_filename, w, h, flipped);
}

}


