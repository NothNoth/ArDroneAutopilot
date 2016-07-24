#ifndef BLACKBOX_H_
#define BLACKBOX_H_

#include <iostream>
#include <fstream>
#include <map>
extern "C"
{
  #include <VP_Os/vp_os_signal.h>
  #include <ardrone_api.h>
}


using namespace std;
class BlackBox
{
  public:
    static BlackBox * GetInstance();
    void AddTitle(ostringstream& data);     
    void SetOutput(string logPath, string logName);
    void DeclareColumn(string column);
    void PushData(string column, float data);
    void StartLogging(long logInterval);
    void StopLogging();
    void * StartLoggingCb(void * userData);
  private:
    BlackBox();
    virtual ~BlackBox();
    static BlackBox * _instance;
    bool       _ready;
    string     _logPath;
    string     _logName;
    ofstream * _f;
    map<string, float> _currentData;
    long      _logInterval;
    
    bool          _writeThreadRunning;
    vp_os_mutex_t  _accessLock;
    pthread_t      _writeThread;
};


#endif /* BLACKBOX_H_ */
