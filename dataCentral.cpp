#include <sys/times.h>
extern "C"
{
  #include <ardrone_api.h>
  #include <ardrone_tool/UI/ardrone_tool_ui.h>
}
#include "dataCentral.h"
#include "BlackBox.h"

using namespace std;

DataCentral * DataCentral::_instance = 0;

DataCentral::DataCentral()
{
  vp_os_mutex_init(&_update_lock);
//ardrone_at_set_navdata_all();
//ardrone_at_start_raw_capture();
  ardrone_at_set_led_animation ( BLINK_ORANGE, 10, 2);
//  cout << "RET :" << (ardrone_tool_set_ui_pad_start(0)==C_OK?"ok":"failed") << endl;
  BlackBox::GetInstance()->DeclareColumn("Alt");
  BlackBox::GetInstance()->DeclareColumn("Pitch");
  BlackBox::GetInstance()->DeclareColumn("Roll");
  BlackBox::GetInstance()->DeclareColumn("Yaw");
  BlackBox::GetInstance()->DeclareColumn("vX");
  BlackBox::GetInstance()->DeclareColumn("vY");
  BlackBox::GetInstance()->DeclareColumn("Bat");
  
  
}


DataCentral::~DataCentral()
{
}

DataCentral * DataCentral::GetInstance()
{
  if (!_instance)
    _instance = new DataCentral();

  return _instance;
}

void DataCentral::UpdateNavData(navdata_unpacked_t* const navdata)
{
  _Lock();
  memcpy(&_navdata_demo, &navdata->navdata_demo, sizeof(navdata_demo_t));
  memcpy(&_navdata_time, &navdata->navdata_time, sizeof(navdata_time_t));
  
  //cout << "DataCentral> Frame num " << navdata->navdata_video_stream.frame_number <<endl;

  _UnLock();
  
  ostringstream s;
  s << _navdata_demo.altitude << ";" << _navdata_demo.theta << ";" << _navdata_demo.phi << ";" << _navdata_demo.psi << ";" << _navdata_demo.vx << ";" << _navdata_demo.vy << ";" << _navdata_demo.vbat_flying_percentage;

  BlackBox::GetInstance()->PushData("Alt", _navdata_demo.altitude);
  BlackBox::GetInstance()->PushData("Pitch", _navdata_demo.theta);
  BlackBox::GetInstance()->PushData("Roll", _navdata_demo.phi);
  BlackBox::GetInstance()->PushData("Yaw", _navdata_demo.psi);
  BlackBox::GetInstance()->PushData("vX", _navdata_demo.vx);
  BlackBox::GetInstance()->PushData("vY", _navdata_demo.vy);
  BlackBox::GetInstance()->PushData("Bat", _navdata_demo.vbat_flying_percentage);
  
 /*
printf("Navdata process :\n");
printf("Alt : %f [%d %d]\n", navdata->navdata_altitude.altitude_vz,
navdata->navdata_altitude.tag,
navdata->navdata_altitude.size);

printf("Phys Acces : %f [%d %d]\n", navdata->navdata_phys_measures.accs_temp,
navdata->navdata_phys_measures.tag,
navdata->navdata_phys_measures.size);

printf("Roll : %d [%d %d]\n", navdata->navdata_references.ref_roll, 
navdata->navdata_references.tag,
navdata->navdata_references.size);

printf("rcRoll : %d [%d %d]\n", navdata->navdata_rc_references.rc_ref_roll, 
navdata->navdata_rc_references.tag,
navdata->navdata_rc_references.size);
*/


/*
printf("Flying : %s\n", (_navdata_demo.ctrl_state&ARDRONE_FLY_MASK)?"yes":"no");
printf("demo alt : %d theta : %f Phi : %f Psi : %f [%d %d]\n", 
_navdata_demo.altitude, 
_navdata_demo.theta, 
_navdata_demo.phi, 
_navdata_demo.psi, 
_navdata_demo.tag,
_navdata_demo.size);

printf("Speed : vx:%f vy:%f\n", _navdata_demo.vx, _navdata_demo.vy);
printf("Battery : %d%%\n", _navdata_demo.vbat_flying_percentage);
*/
}

long  DataCentral::GetLastUpdateDelayMs() 
{
  _Lock();
  long t = times(NULL)*10 - ardrone_time_to_usec(_navdata_time.time)*1000;
  _UnLock();
  return  t;
}

bool DataCentral::IsFlying()
{
  bool ret;
  _Lock();
  if (_navdata_demo.ctrl_state&ARDRONE_FLY_MASK)
    ret = true;
  else
    ret = false;
  _UnLock();
  return ret;
}
