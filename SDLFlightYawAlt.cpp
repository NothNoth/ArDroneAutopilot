#include <iostream>
#include "SDLFlightYawAlt.h"
extern "C"
{
  #include "SDL/SDL_gfxPrimitives.h"
  #include <math.h>
  #include <SDL/SDL_rotozoom.h>
}

using namespace std;
#define MARGIN 5

#define RMASK 0x00FF0000
#define GMASK 0x0000FF00
#define BMASK 0x000000FF
#define AMASK 0x00000000



SDLFlightYawAlt::SDLFlightYawAlt(int dimension) :
  _w(dimension),
  _h(dimension),
  _yaw(0),
  _minYaw(-M_PI/2.0),
  _maxYaw(M_PI/2.0),
  _alt(0),
  _debug(false)
{
  //Rendering surfaces
  _back = SDL_CreateRGBSurface
			(SDL_HWSURFACE, _w, _h, 24, 
			RMASK,GMASK, BMASK, AMASK);
  _front = SDL_CreateRGBSurface
			(SDL_HWSURFACE, _w, _h, 24, 
			RMASK,GMASK, BMASK, AMASK);
  _current = SDL_CreateRGBSurface
			(SDL_HWSURFACE, _w, _h, 24, 
			RMASK,GMASK, BMASK, AMASK);
  TTF_Init();
  _font = TTF_OpenFont("./resources/FreeMono.ttf", 14);
  SDL_FillRect(_back, NULL, 0x00666666);


  //Prepare front layer
  SDL_FillRect(_front, NULL, 0x00222222);
  filledCircleRGBA(_front, (int)(_w/2), (int) (_h/2), (int) (_w/2) - MARGIN, 0x00, 0xFF, 0x00, 0xFF); 
  SDL_SetColorKey(_front, SDL_SRCCOLORKEY, 0x0000FF00);

  circleRGBA(_front, (int)(_w/2), (int) (_h/2), (int) (_w/2) - MARGIN, 0xAA, 0xAA, 0xAA, 0xFF); 
  filledCircleRGBA(_front, 8, 8, 5, 0x88, 0x88, 0x88, 0xFF); 
  filledCircleRGBA(_front, _w-8, 8, 5, 0x88, 0x88, 0x88, 0xFF); 
  filledCircleRGBA(_front, 8, _h-8, 5, 0x88, 0x88, 0x88, 0xFF); 
  filledCircleRGBA(_front, _w-8, _h-8, 5, 0x88, 0x88, 0x88, 0xFF); 

  //altitude box
  boxRGBA(_back, (_w/2.0) - 25, (3.0*_h/4.0), 
                  (_w/2.0)+25, (3.0*_h/4.0) + 14 ,
                  0x44, 0x44, 0x44, 0xFF);
  rectangleRGBA(_back, (_w/2.0) - 25, (3.0*_h/4.0), 
                  (_w/2.0)+25, (3.0*_h/4.0) + 14 ,
                  0xAA, 0xAA, 0xAA, 0xFF);
  SDL_Color color;
  SDL_Rect p;
  color.r = 0x00;
  color.g = 0x00;
  color.b = 0x00;
  SDL_Surface * s = SDLCALL TTF_RenderText_Blended(_font, "CM", color);
  p.x = (_w/2.0) + 25 + 5;
  p.y =   (3.0*_h/4.0) ;
  p.w = -1;
  p.h = -1;
  SDL_BlitSurface(s, NULL, _back, &p); 

  //yaw pic
  {
    int cx, cy;
    cx =  (_w/2.0)/2.0;
    cy = (_h/2.0)/2.0;
    _yawPic = SDL_CreateRGBSurface(SDL_HWSURFACE, _w/2.0, _h/2.0, 24, 
			                              RMASK, GMASK, BMASK, AMASK);
    SDL_SetColorKey(_yawPic, SDL_SRCCOLORKEY, 0x0000FF00);
    SDL_FillRect(_yawPic, NULL, 0x0000FF00);  

    filledCircleRGBA(_yawPic, cx, cy, _yawPic->w/2.0, 0x44, 0x44, 0x44, 0xFF); 


    circleRGBA(_yawPic,       cx, cy, (_yawPic->w/2.0)-1, 0xAA, 0xAA, 0xAA, 0xFF); 
    filledEllipseRGBA(_yawPic,cx, cy, 10, _yawPic->h/3.0, 0xAA, 0xAA, 0xAA, 0xFF);

    //large wing

    filledTrigonRGBA(_yawPic,  
                      15, _yawPic->h/2.0, 
                      _yawPic->w-15, _yawPic->h/2.0, 
                      _yawPic->w/2.0, _yawPic->h/2.0-20,
				              0xAA, 0xAA, 0xAA, 0xFF);

    //small wing
    int ellipseH = (int)(2*_yawPic->h/3.0);
    filledTrigonRGBA(_yawPic,  
                      (_yawPic->w/2.0)-20, _yawPic->h - (_yawPic->h-ellipseH)/2.0, 
                      (_yawPic->w/2.0)+20, _yawPic->h - (_yawPic->h-ellipseH)/2.0, 
                      _yawPic->w/2.0,      _yawPic->h - (_yawPic->h-ellipseH)/2.0 - 15,
				              0xAA, 0xAA, 0xAA, 0xFF);

    filledCircleRGBA(_yawPic, cx, cy, 4, 0x66, 0x66, 0x66, 0xFF);  //screw
  }
}


SDLFlightYawAlt::~SDLFlightYawAlt()
{
  SDL_FreeSurface(_back);
}

SDL_Surface * SDLFlightYawAlt::GetSurface()
{
  SDL_Color color;
  SDL_Rect p;
  char tmp[16];
  sprintf(tmp, "%05d", _alt);

  //Draw background
  SDL_BlitSurface(_back, NULL, _current, NULL); 

  //Draw altitude
  color.r = 0xBB;
  color.g = 0xBB;
  color.b = 0xBB;
  SDL_Surface * s = SDLCALL TTF_RenderText_Solid(_font, tmp, color);
  if (s)
  {
    p.x = (_w/2.0) - 25 + 5;
    p.y =   (3.0*_h/4.0) ;
    p.w = -1;
    p.h = -1;
    SDL_BlitSurface(s, NULL, _current, &p); 
    SDL_FreeSurface(s);
  }

  //draw yaw picture
  SDL_Surface * yawtmp = rotozoomSurfaceXY(_yawPic, (double)_yaw, 1, 1, 1);
  p.x = (_w/2.0) - (yawtmp->w/2.0);
  p.y = (_h/2.0) - (2.0*yawtmp->h/3.0);
  p.w = -1;
  p.h = -1;
  SDL_SetColorKey(yawtmp, SDL_SRCCOLORKEY, 0x0000FF00);
  SDL_BlitSurface(yawtmp, NULL, _current, &p); 
  SDL_FreeSurface(yawtmp);

  // draw foreground (front metal)
  SDL_BlitSurface(_front, NULL, _current, NULL); 

  if (_debug)
  {
    char dbg[64];
    sprintf(dbg, "alt %05d Yaw %.2f", _alt, _yaw);
    color.r = 0x00;
    color.g = 0xFF;
    color.b = 0x00;
    SDL_Surface * s = SDLCALL TTF_RenderText_Solid(_font, dbg, color);
    if (s)
    {
      p.x = 0;
      p.y = 0;
      p.w = -1;
      p.h = -1;
      SDL_BlitSurface(s, NULL, _current, &p);
      SDL_FreeSurface(s); 
    }

  }

  return _current;
}
