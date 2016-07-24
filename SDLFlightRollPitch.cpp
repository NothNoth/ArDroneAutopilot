#include <iostream>
#include "SDLFlightRollPitch.h"
extern "C"
{
  #include "SDL/SDL_gfxPrimitives.h"
  #include <math.h>
}

using namespace std;
#define MARGIN 5

#define RMASK 0x00FF0000
#define GMASK 0x0000FF00
#define BMASK 0x000000FF
#define AMASK 0x00000000



SDLFlightRollPitch::SDLFlightRollPitch(int dimension) :
  _w(dimension),
  _h(dimension),
  _pitch(0.0),
  _minPitch(-M_PI/2.0),
  _maxPitch(M_PI/2.0),
  _roll(M_PI/2.0),
  _minRoll(-M_PI),
  _maxRoll(M_PI),
  _debug(false)
{
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
  SDL_FillRect(_back, NULL, 0x00AA4444);

  SDL_FillRect(_front, NULL, 0x00222222);
  filledCircleRGBA(_front, (int)(_w/2), (int) (_h/2), (int) (_w/2) - MARGIN, 0x00, 0xFF, 0x00, 0xFF); 
  SDL_SetColorKey(_front, SDL_SRCCOLORKEY, 0x0000FF00);

  circleRGBA(_front, (int)(_w/2), (int) (_h/2), (int) (_w/2) - MARGIN, 0xAA, 0xAA, 0xAA, 0xFF); 
  filledCircleRGBA(_front, 8, 8, 5, 0x88, 0x88, 0x88, 0xFF); 
  filledCircleRGBA(_front, _w-8, 8, 5, 0x88, 0x88, 0x88, 0xFF); 
  filledCircleRGBA(_front, 8, _h-8, 5, 0x88, 0x88, 0x88, 0xFF); 
  filledCircleRGBA(_front, _w-8, _h-8, 5, 0x88, 0x88, 0x88, 0xFF); 

}


SDLFlightRollPitch::~SDLFlightRollPitch()
{
  SDL_FreeSurface(_back);
}

SDL_Surface * SDLFlightRollPitch::GetSurface()
{
  int angle;
  int xAlt;
/*
Computing pitch ...
pitch -pi/2 : r.h = 400
pitch 0     : r.h = 400/2
pitch pi/2  : r.h = 0

y = ax + b;

400 = a*maxP + b;
0 = a*minP + b;

400 =a*(maxP - minP);
a = _h/(maxP - minP);

b = -(_h/(maxP - minP))) * minP;
*/
  xAlt = (_h/(_maxPitch - _minPitch))*_pitch - (_h * _minPitch)/(_maxPitch - _minPitch);

  angle = _roll;

  SDL_Rect r;
  r.w = _w;
  r.h = xAlt;
  r.x = 0;
  r.y = 0;
  SDL_BlitSurface(_back, NULL, _current, NULL); // draw background (blue)
  filledPieRGBA(_current, (_h/2.0), r.h, _w,
	    			    180+angle, angle, 
                0x22, 0x22, 0x99, 0xFF);

  SDL_BlitSurface(_front, NULL, _current, NULL); // draw foreground (front metal)
  
  if (_debug)
  {
    char dbg[64];
    SDL_Color color;
    sprintf(dbg, "pitch %.2f roll %.2f", _pitch, _roll);
    color.r = 0x00;
    color.g = 0xFF;
    color.b = 0x00;
    SDL_Surface * s = SDLCALL TTF_RenderText_Solid(_font, dbg, color);
    if (s)
    {
      r.x = 0;
      r.y = 0;
      r.w = -1;
      r.h = -1;
      SDL_BlitSurface(s, NULL, _current, &r); 
      SDL_FreeSurface(s);
    }

  }
  return _current;
}
