#pragma once
#include "../Game/d3dApp.h"
class stuff
{
private:
	float position;
	float angle;
	float size;

	stuff() = delete;

protected:

	std::string mName;

public:
	stuff(std::string name) : mName(name) {};

	virtual void transform() {};
	virtual void rotate() {};
	virtual void scale() {};
};

class Stuff : public stuff
{
public:
	Stuff(std::string name) : stuff(name) {};

};
