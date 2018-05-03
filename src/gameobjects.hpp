#ifndef GAME_OBJECTS_HPP
#define GAME_OBJECTS_HPP

#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

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

class Player: public GameObj
{
  public:
    Player(Point position, int radius);
    void reposition(Point position);
    void draw(Mat image);
};

int move_all(Vector<Obstacle> &goCollection);
int draw_all(Mat image, Vector<GameObj*> &goCollection);
int detect_collision(GameObj &go1, GameObj* go2);
int detect_collision(GameObj &go1, Vector<GameObj*> &goCollection);


#endif