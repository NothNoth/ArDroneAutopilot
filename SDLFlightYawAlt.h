extern "C"
{
  #include <SDL/SDL_video.h>
  #include <SDL/SDL_ttf.h>
}


class SDLFlightYawAlt
{
  public:
    SDLFlightYawAlt(int dimension);
    ~SDLFlightYawAlt();
    SDL_Surface * GetSurface();
    int GetWidth() { return _w; }
    int GetHeight() { return _h; }
    void SetYaw(float yaw) {_yaw = yaw;}
    void SetMinMaxYaw(float minY, float maxY) {_minYaw = minY; _maxYaw = maxY;}
    void SetAlt(int alt) {_alt = alt;}
    void Debug(bool flag) {_debug = flag;}
  private:
    int _w;
    int _h;
    SDL_Surface * _back;
    SDL_Surface * _front;
    SDL_Surface * _current;
    SDL_Surface * _yawPic;
    TTF_Font * _font;
    float _yaw;
    float _minYaw;
    float _maxYaw;
    int _alt;
    bool _debug;
};
