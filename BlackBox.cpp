/*
 * BlackBox.cpp
 *
 *  Created on: Nov 20, 2010
 *      Author: noth
 */

#include "BlackBox.h"
#include <time.h>
#include <fstream>
#include <sys/times.h>
#include <limits.h>

BlackBox * BlackBox::_instance = 0;

BlackBox::BlackBox() :
  _ready(false),
  _writeThreadRunning(false)
{
  vp_os_mutex_init(&_accessLock);
}

BlackBox::~BlackBox()
{
  if (!_ready)
    return;
  _f->close();
  delete(_f);
  vp_os_mutex_destroy(&_accessLock);  
}

BlackBox * BlackBox::GetInstance()
{
  if (!_instance)
    _instance = new BlackBox();

  return _instance;
}

void BlackBox::SetOutput(string logPath, string logName)
{
  _logPath = logPath; 
  _logName = logName;
  time_t t;
  char filestr[64];
  time(&t);
  strftime(filestr, 64, "_%Y-%m-%d_%H_%M", localtime(&t));


  _f = new ofstream((_logPath+_logName+string(filestr)+string(".csv")).c_str(), ios::out);
  if (! _f->is_open())
    return;
    
  _ready = true;
}

void BlackBox::DeclareColumn(string column)
{
  vp_os_mutex_lock(&_accessLock);
  _currentData[column] = UINT_MAX;
  vp_os_mutex_unlock(&_accessLock);
}

void BlackBox::PushData(string column, float data)
{
  vp_os_mutex_lock(&_accessLock);
  _currentData[column] = data;
  vp_os_mutex_unlock(&_accessLock);
}


void * _writeThreadStart(void * data)
{
  return BlackBox::GetInstance()->StartLoggingCb(data);
}

void BlackBox::StartLogging(long logInterval)
{
  pthread_attr_t stAttr;
  pthread_attr_init(&stAttr);
   if (!_ready)
    return;
  _logInterval = logInterval;
  if (pthread_create(&_writeThread, &stAttr, _writeThreadStart, NULL) != 0)
  {
    printf("Can't create BlackBox thread!!\n");
  }  
}

void BlackBox::StopLogging()
{
  void * ret;
  _writeThreadRunning = false;
  pthread_join(_writeThread, &ret);
}


void * BlackBox::StartLoggingCb(void * userData)
{
  long tStart;
  map<string, float>::iterator it;
  _writeThreadRunning = true;
  
  //Write column titles
  it = _currentData.begin();
  (*_f) << "TimeMs";
  while (it != _currentData.end())
  {
    (*_f) << ";" << (*it).first;
    it++;
  }
  (*_f) << endl;

  //Write log line every _logInterval ms
  tStart = times(NULL);
  while (_writeThreadRunning)
  {
    vp_os_mutex_lock(&_accessLock);
    it = _currentData.begin();
    (*_f) << (times(NULL) - tStart);
    while (it != _currentData.end())
    {
      (*_f) << ";";
      if ((*it).second != UINT_MAX)
      {
        (*_f) << (*it).second;
        _currentData[(*it).first] = UINT_MAX;
      }
      it++;
    }
    (*_f) << endl;

    vp_os_mutex_unlock(&_accessLock);
    usleep(_logInterval*1000);
  }
  return NULL;
}

 
