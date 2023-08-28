#include <iostream>

#define PROJECT_NAME "ecs"

struct Gravity
{
	float acceleration;
};

struct RigidBody
{
	float velocity;
};

struct Transform
{
	float x;
	float y;
};

int main(int argc, char **argv)
{
    if(argc != 1)
    {
        std::cout << argv[0] <<  "takes no arguments.\n";
        return 1;
    }
    std::cout << "This is project " << PROJECT_NAME << ".\n";
    return 0;
}
