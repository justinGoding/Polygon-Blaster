#pragma once

#include "Common.h"

class c_Transform
{
public: 
	c_Vec2 pos= { 0.0, 0.0 };
	c_Vec2 velocity= { 0.0, 0.0 };
	float angle= 0;

	c_Transform(const c_Vec2 &p, const c_Vec2 &v, float a)
		: pos(p), velocity(v), angle(a) {}
};

class c_Circle
{
public:
	sf::CircleShape circle;

	c_Circle(float radius, int points, const sf::Color &fill, const sf::Color &outline, float thickness)
		: circle(radius, points)
	{
		circle.setFillColor(fill);
		circle.setOutlineColor(outline);
		circle.setOutlineThickness(thickness);
		circle.setOrigin(radius, radius);
	}
};

class c_Rectangle
{
public:
	sf::RectangleShape rectangle;

	c_Rectangle(float width, float height, float angle, const sf::Color &color)
		: rectangle(sf::Vector2f(width, height))
	{
		rectangle.setFillColor(color);
		rectangle.setOrigin(width, height / 2);
		rectangle.rotate(angle * 57.2958);
	}
};

class c_Collision
{
public:
	float radius= 0;
	c_Collision(float r)
		: radius(r) {}
};

class c_Score
{
public:
	int score= 0;
	c_Score(int s)
		: score(s) {}
};

class c_Lifespan
{
public:
	int remaining= 0;
	int total= 0;
	c_Lifespan(int total)
		: remaining(total), total(total) {}
};

class c_Input
{
public:
	bool up= false;
	bool left= false;
	bool right= false;
	bool down= false;
	bool shoot= false;

	c_Input() {}
};

class c_Line
{
public:
	sf::Vertex line[2];

	c_Line(c_Vec2 &start_point, float length, float angle, const sf::Color &color)
	{
		line[0].position.x= start_point.x;
		line[0].position.y= start_point.y;
		line[0].color= color;

		line[1].position.x= start_point.x + (length * cosf(angle));
		line[1].position.y= start_point.y + (length * sinf(angle));
		line[1].color= color;
	}
};

class c_Tethered
{
public:
	int duration;
	c_Vec2 tether_point;

	c_Tethered(c_Vec2 &tether_point, int d)
		: tether_point(tether_point), duration(d) {}
};

class c_Ethereal
{
public:
	int duration;

	c_Ethereal(int d)
		: duration(d) {}
};