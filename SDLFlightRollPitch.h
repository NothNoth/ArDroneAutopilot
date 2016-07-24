extern "C"
{
  #include <SDL/SDL_video.h>
  #include <SDL/SDL_ttf.h>
}


class SDLFlightRollPitch
{
  public:
    SDLFlightRollPitch(int dimension);
    ~SDLFlightRollPitch();
    SDL_Surface * GetSurface();
    int GetWidth() { return _w; }
    int GetHeight() { return _h; }
    void SetPitch(float pitch) {_pitch = pitch;}
    void SetMinMaxPitch(float minP, float maxP) {_minPitch = minP; _maxPitch = maxP;}
    void SetRoll(float roll) {_roll = roll;}
    void SetMinMaxRoll(float minR, float maxR) {_minRoll = minR; _maxRoll = maxR;}
    void Debug(bool flag) {_debug = flag;}
  private:
    int _w;
    int _h;
    SDL_Surface * _back;
    SDL_Surface * _front;
    SDL_Surface * _current;
    TTF_Font * _font;
    float _pitch;
    float _minPitch;
    float _maxPitch;
    float _roll;
    float _minRoll;
    float _maxRoll;
    bool  _debug;
};
