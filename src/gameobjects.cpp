#include "gameobjects.hpp"
#include <iostream>
#include <math.h>


#define SQUARE(x) ((x)*(x))

#define GOAL_THICKNESS 2
#define GOAL_COLOR Scalar(0,200,0)
#define OBSTACLE_THICKNESS 2
#define OBSTACLE_COLOR Scalar(0,0,200)
#define PLAYER_THICKNESS 1
#define PLAYER_COLOR_1 Scalar(0,0,0)
#define PLAYER_COLOR_2 Scalar(200,200,200)

Goal::Goal(Point position, int radius)
{
  pos = position;
  size = radius;
}

void Goal::draw(Mat image)
{
  circle(image, pos, size, GOAL_COLOR, GOAL_THICKNESS,8,0);
}

Obstacle::Obstacle(Point position, int radius, Point moveSpeed)
{
  pos = position;
  size = radius;
  speed = moveSpeed;
}

void Obstacle::move()
{
  pos.x += speed.x;
  pos.y += speed.y;
}

void Obstacle::draw(Mat image)
{
  circle(image, pos, size, OBSTACLE_COLOR, OBSTACLE_THICKNESS,8,0);
}

Player::Player(Point position, int radius)
{
  pos = position;
  size = radius;
}

void Player::reposition(Point position)
{
  pos = position;
}

void Player::draw(Mat image)
{
  circle(image, pos, size - PLAYER_THICKNESS, PLAYER_COLOR_1, PLAYER_THICKNESS,8,0);
  circle(image, pos, size, PLAYER_COLOR_2, PLAYER_THICKNESS,8,0);
}

int move_all(Vector<Obstacle*> &goCollection)
{
  unsigned int i;
  for(i = 0; i < goCollection.size(); ++i)
  {
    goCollection[i]->move();
  }

  return 0;
}

int draw_all(Mat image, Vector<GameObj*> &goCollection)
{
  unsigned int i;
  for(i = 0; i < goCollection.size(); ++i)
  {
    goCollection[i]->draw(image);
  }

  return 0;
}

int detect_collision(GameObj &go1, GameObj* go2)
{
  int distance = sqrt(SQUARE(go1->pos.x - go2->pos.x) + SQUARE(go1->pos.y - go1->pos.y));

  if(distance < (go1->size + go2->size))
  {
    return 1;
  }

  return 0;
}

int detect_collision(GameObj &go1, Vector<GameObj*> &goCollection)
{
  unsigned int i;
  for(i = 0; i < goCollection.size(); ++i)
  {
    if(detect_collision(go1, goCollection[i]))
    {
      return i;
    }
  }

  return 0;
}

