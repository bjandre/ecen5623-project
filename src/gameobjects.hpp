##ifndef GAME_OBJECTS_HPP
#define GAME_OBJECTS_HPP

#include <opencv2/opencv.hpp>
#include <vector>

class GameObj
{
  public:
    Point pos;
    int size;
    virtual void draw(Mat image) = 0;
};

class Goal: public GameObj
{
  public:
    Goal(Point position, int radius);
    void draw(Mat image);
};

class Obstacle: public GameObj
{
  public:
    Point speed;
    Obstacle(Point position, int radius, Point moveSpeed);
    void move();
    void draw(Mat image);
};

#endif