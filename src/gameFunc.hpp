#ifndef GAMEFUNC_HPP_INCLUDED
#define GAMEFUNC_HPP_INCLUDED

struct Position{
    Position() {x=0;y=0;z=0;};
    Position (double x_, double y_, double z_)
    {
        x = x_;
        y = y_;
        z = z_;
    };
    ~Position() {};

    double x, y, z;
};

Position thisFunc();

#endif // GAMEFUNC_HPP_INCLUDED
