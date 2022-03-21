#ifndef PIBITI_H
#define PIBITI_H

class Sensor{
  public:
    void wakeup();
    void write(int reg, int val);
  
};

#endif
