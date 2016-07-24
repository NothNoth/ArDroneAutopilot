#ifndef DATA_CENTRAL_H
#define DATA_CENTRAL_H

#include <iostream>

extern "C"
{
  #include <VP_Os/vp_os_signal.h>
  #include <ardrone_api.h>
}

class DataCentral
{
  public:
    static DataCentral * GetInstance();
    virtual ~DataCentral();
    void  UpdateNavData(navdata_unpacked_t* const navdata);
    int   GetAltitude() {int a; _Lock(); a = _navdata_demo.altitude; _UnLock();return a;}
    float GetPitch()    {float t; _Lock(); t = _navdata_demo.theta; _UnLock();return t;}
    float GetRoll()      {float p; _Lock(); p = _navdata_demo.phi; _UnLock();return p;}
    float GetYaw()      {float p; _Lock(); p = _navdata_demo.psi; _UnLock();return p;}
    int   GetSpeedX()   {int vx; _Lock(); vx = _navdata_demo.vx; _UnLock();return vx;}
    int   GetSpeedY()   {int vy; _Lock(); vy = _navdata_demo.vy; _UnLock();return vy;}

    float GetBatteryLevel()  {float bl; _Lock(); bl = _navdata_demo.vbat_flying_percentage; _UnLock();return bl;}
    long  GetLastUpdateDelayMs();
    bool  IsFlying();
    
  private:
    static DataCentral * _instance;
    DataCentral();
    void _Lock() {vp_os_mutex_lock(&_update_lock);}
    void _UnLock() {vp_os_mutex_unlock(&_update_lock);}

    vp_os_mutex_t            _update_lock;

    navdata_demo_t           _navdata_demo;
    navdata_time_t           _navdata_time;
};

#endif
