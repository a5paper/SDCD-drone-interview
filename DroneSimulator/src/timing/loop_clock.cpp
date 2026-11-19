#include "timing/loop_clock.h"

#include <thread>

#include "config/config.h"
#ifdef _WIN32
// clang-format off
// timeapi.h needs windows.h first.
#include <windows.h>
#include <timeapi.h>
// clang-format on
#endif

namespace drone_simulator::timing {
namespace {

using Seconds = std::chrono::duration<double>;

LoopClock::Clock::duration ToDuration(double seconds) {
  return std::chrono::duration_cast<LoopClock::Clock::duration>(
      Seconds(seconds));
}

}

LoopClock::LoopClock(double period_s)
    : period_(ToDuration(period_s)), next_deadline_(start_ + period_) {
#ifdef _WIN32
  timeBeginPeriod(config::kWindowsTimerResolutionMs);
#endif
}

LoopClock::~LoopClock() {
#ifdef _WIN32
  timeEndPeriod(config::kWindowsTimerResolutionMs);
#endif
}

double LoopClock::Tick() {
  const Clock::time_point now = Clock::now();
  const double delta_time_s = Seconds(now - tick_start_).count();
  tick_start_ = now;
  return delta_time_s;
}

void LoopClock::SleepUntilNextTick() {
  const Clock::time_point now = Clock::now();
  if (now > next_deadline_ + period_) {
    next_deadline_ = now + period_;
  }
  std::this_thread::sleep_until(next_deadline_);
  next_deadline_ += period_;
}

LoopClock::Clock::duration LoopClock::GetUptime() const {
  return tick_start_ - start_;
}

IntervalTimer::IntervalTimer(double interval_s) : interval_s_(interval_s) {}

bool IntervalTimer::Elapsed(double delta_time_s) {
  accumulated_s_ += delta_time_s;
  if (accumulated_s_ < interval_s_) {
    return false;
  }
  accumulated_s_ = 0.0;
  return true;
}

}
