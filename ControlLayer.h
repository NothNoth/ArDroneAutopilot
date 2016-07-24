/*
 * ControlLayer.h
 *
 *  Created on: 27 févr. 2011
 *      Author: benoitgirard
 */

#ifndef CONTROLLAYER_H_
#define CONTROLLAYER_H_


typedef enum
{
  CAMERA_HORI=0,
  CAMERA_VERT,
  CAMERA_LARGE_HORI_SMALL_VERT,
  CAMERA_LARGE_VERT_SMALL_HORI,
  CAMERA_NEXT
} tCamera;


class ControlLayer
{
  public:
    static ControlLayer * GetInstance();

    bool TakeOff();
    bool Land();
    bool SetNavdataDemo(bool flag);
    bool SetCamera(tCamera camera);
    bool SetEmergency();
    bool CancelEmergency();
    bool SetFlatTrim();
    bool StartRawCapture();
    bool SetAngles(int enable, float roll, float pitch, float gaz, float yaw);
  private:
    ControlLayer();
    virtual ~ControlLayer();

    static ControlLayer * _instance;


};

#endif /* CONTROLLAYER_H_ */
