/*
 * ControlLayer.cpp
 *
 *  Created on: 27 févr. 2011
 *      Author: benoitgirard
 */

#include "ControlLayer.h"
#include "BlackBox.h"

ControlLayer * ControlLayer::_instance = 0;
extern "C"
{
  #include <ardrone_api.h>
  #include <ardrone_tool/UI/ardrone_tool_ui.h>
  #include <ardrone_tool/UI/ardrone_input.h>
}



ControlLayer * ControlLayer::GetInstance()
{
  if (!_instance)
    _instance = new ControlLayer();

  return _instance;
}

ControlLayer::ControlLayer()
{
  BlackBox::GetInstance()->DeclareColumn("CmdTakeOff");
  BlackBox::GetInstance()->DeclareColumn("CmdSOS");
  BlackBox::GetInstance()->DeclareColumn("CmdFlatTrim");
}

ControlLayer::~ControlLayer()
{

}

bool ControlLayer::TakeOff()
{
  BlackBox::GetInstance()->PushData("CmdTakeOff", 1.0);
  if (ui_pad_start_stop(1) == C_OK)
    return true;
  else
    return false;
}

bool ControlLayer::Land()
{
  BlackBox::GetInstance()->PushData("CmdTakeOff", -1.0);
  if (ui_pad_start_stop(1) == C_OK)
    return true;
  else
    return false;
}


bool ControlLayer::SetNavdataDemo(bool flag)
{
  if (flag)
    ardrone_at_set_toy_configuration( "general:navdata_demo", "TRUE" );
  else
    ardrone_at_set_toy_configuration( "general:navdata_demo", "FALSE" );
  return true;
}

bool ControlLayer::SetCamera(tCamera camera)
{
  ZAP_VIDEO_CHANNEL c;

  switch (camera)
  {
    case CAMERA_HORI:
      c = ZAP_CHANNEL_HORI;
    break;
    case CAMERA_VERT:
      c = ZAP_CHANNEL_VERT;
    break;
    case CAMERA_LARGE_HORI_SMALL_VERT:
      c = ZAP_CHANNEL_LARGE_HORI_SMALL_VERT;
    break;
    case CAMERA_LARGE_VERT_SMALL_HORI:
      c = ZAP_CHANNEL_LARGE_VERT_SMALL_HORI;
    break;
    case CAMERA_NEXT:
      c = ZAP_CHANNEL_NEXT;
    break;
    default:
      return false;
  }
  ardrone_at_zap(c);
  return true;
}

bool ControlLayer::SetEmergency()
{
  BlackBox::GetInstance()->PushData("CmdSOS",  1.0);

  return (ui_pad_select(1) == C_OK?true:false);
}

bool ControlLayer::CancelEmergency()
{
  BlackBox::GetInstance()->PushData("CmdSOS",  -1.0);
  return (ui_pad_select(0) == C_OK?true:false);
}

bool ControlLayer::SetFlatTrim()
{
  BlackBox::GetInstance()->PushData("CmdFlatTrim",  1.0);
  ardrone_at_set_flat_trim();
  return true;
}

bool ControlLayer::StartRawCapture()
{
  ardrone_at_start_raw_capture();
  return true;
}
    

bool ControlLayer::SetAngles(int enable, float roll, float pitch, float gaz, float yaw)
{
  ardrone_at_set_progress_cmd(enable, roll, pitch, gaz, yaw);
  return true;
}


