#ifndef ANIMATION_H

class Animation {
  public:
    virtual void begin() = 0;  // Called once when you start the animation
    virtual void run() = 0;    // Called repeatedly in loop()
};

#endif