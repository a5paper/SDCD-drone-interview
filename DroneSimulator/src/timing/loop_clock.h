#ifndef DRONE_SIMULATOR_TIMING_LOOP_CLOCK_H_
#define DRONE_SIMULATOR_TIMING_LOOP_CLOCK_H_

#include <chrono>

namespace drone_simulator::timing {

class LoopClock {
 public:
  using Clock = std::chrono::steady_clock;

  explicit LoopClock(double period_s);
  ~LoopClock();
  LoopClock(const LoopClock&) = delete;
  LoopClock& operator=(const LoopClock&) = delete;

  double Tick();
  void SleepUntilNextTick();
  Clock::duration GetUptime() const;

 private:
  Clock::duration period_;
  Clock::time_point start_ = Clock::now();
  Clock::time_point tick_start_ = start_;
  Clock::time_point next_deadline_;
};

class IntervalTimer {
 public:
  explicit IntervalTimer(double interval_s);

  [[nodiscard]] bool Elapsed(double delta_time_s);

 private:
  double interval_s_;
  double accumulated_s_ = 0.0;
};

}

#endif
