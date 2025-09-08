#ifndef CMovingAvgProxy_HPP
#define CMovingAvgProxy_HPP

class CMovingAvgProxy : public CAccelProxy {
  public:
    CMovingAvgProxy(bool Logging = false)
      : CAccelProxy(Logging) {}

    ~CMovingAvgProxy() {}

    // Blocking operation: waits until accel finishes.
    // Non-blocking: Returns immediately.
    uint32_t MovingAvg_HW(uint32_t * input, uint32_t * output, uint32_t length, bool blocking = true);

    // Checks if the HW has finished.
    bool CheckEnd();
};

#endif  // CMovingAvgProxy_HPP

