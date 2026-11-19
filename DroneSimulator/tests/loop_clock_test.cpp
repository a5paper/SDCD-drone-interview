#include "timing/loop_clock.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <limits>
#include <thread>

namespace drone_simulator::timing {
namespace {

using Clock = LoopClock::Clock;
using Milliseconds = std::chrono::duration<double, std::milli>;

constexpr double kPeriodS = 0.02;
constexpr double kPeriodMs = kPeriodS * 1000.0;
constexpr int kManyTicks = 30;
constexpr double kRateUnderMs = 1.0;
// A host stall can land in any one window; the best window shows the clock.
// Its bound sits between the deadline clock (about 601 ms) and the old
// "from now" clock (609 ms or more on Windows).
constexpr int kRateWindows = 3;
constexpr double kRateOverMs = 5.0;
constexpr int kOverrunPeriods = 3;
constexpr int kTicksAfterOverrun = 3;
constexpr double kMinRecoveryPeriods = 3.5;

double TimeManyTicksMs() {
  LoopClock clock(kPeriodS);
  const Clock::time_point begin = Clock::now();
  for (int i = 0; i < kManyTicks; ++i) {
    clock.Tick();
    clock.SleepUntilNextTick();
  }
  return Milliseconds(Clock::now() - begin).count();
}

TEST(LoopClockTest, HoldsRateOverManyTicks) {
  double best_elapsed_ms = std::numeric_limits<double>::infinity();
  for (int window = 0; window < kRateWindows; ++window) {
    const double elapsed_ms = TimeManyTicksMs();
    EXPECT_GE(elapsed_ms, kManyTicks * kPeriodMs - kRateUnderMs);
    best_elapsed_ms = std::min(best_elapsed_ms, elapsed_ms);
  }
  EXPECT_LE(best_elapsed_ms, kManyTicks * kPeriodMs + kRateOverMs);
}

TEST(LoopClockTest, NoBurstAfterOverrun) {
  LoopClock clock(kPeriodS);
  clock.Tick();
  std::this_thread::sleep_for(
      std::chrono::duration<double>(kOverrunPeriods * kPeriodS));

  const Clock::time_point before = Clock::now();
  clock.SleepUntilNextTick();
  for (int i = 0; i < kTicksAfterOverrun; ++i) {
    clock.Tick();
    clock.SleepUntilNextTick();
  }
  const double took_ms = Milliseconds(Clock::now() - before).count();
  EXPECT_GE(took_ms, kMinRecoveryPeriods * kPeriodMs);
}

}
}
