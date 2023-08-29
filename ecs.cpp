#include <iostream>

#include <glm/glm.hpp>

#include "registry.hpp"

#define PROJECT_NAME "ecs"

struct Gravity
{
	glm::vec3 force;
};

struct RigidBody
{
	glm::vec3 velocity;
	glm::vec3 acceleration;
};

struct Transform
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

int main(int argc, char **argv)
{
    if(argc != 1)
    {
        std::cout << argv[0] <<  "takes no arguments.\n";
        return 1;
    }
    std::cout << "This is project " << PROJECT_NAME << ".\n";

    Registry<uint32_t, Gravity, RigidBody, Transform> registry;
    std::cout << "Transform is at: " << registry.getIndex<Transform>() << std::endl
	      << "RigidBody is at: " << registry.getIndex<RigidBody>() << std::endl
	      << "Gravity is at:   " << registry.getIndex<Gravity>() << std::endl;

    std::cout << "Sizeof Pool<Transform>: " << registry.size<Transform>() << std::endl;
    auto entity = registry.create();
    registry.add(entity, Transform{ {1.0f, 1.0f, 1.0f}, {0.f, 0.f, 0.f}, {1.0f, 1.0f, 1.0f} });
    registry.add(entity, Gravity{ {0.f, 9.8f, 0.f }});
    std::cout << "Sizeof Pool<Transform>: " << registry.size<Transform>() << std::endl;

    auto &t = registry.get<Transform>(entity);
    std::cout << t.position[0] << "," << t.position[1] << "," << t.position[2] << std::endl
	      << t.rotation[0] << "," << t.rotation[1] << "," << t.rotation[2] << std::endl
	      << t.scale[0] << "," << t.scale[1] << "," << t.scale[2] << std::endl;

    entity = registry.create();
    registry.add(entity, Gravity{ {0.f, 9.8f, 0.f} });
    entity = registry.create();
    registry.add(entity, Transform{ {2.0f, 2.0f, 2.0f}, {0.f, 0.f, 0.f}, {1.0f, 1.0f, 1.0f} });
    registry.add(entity, Gravity{ {0.f, 9.8f, 0.f} });

    for (auto entity: registry.extract<Transform, Gravity>())
    {
	    std::cout << "Entity " << entity << std::endl;

	    auto &t = registry.get<Transform>(entity);
	    auto &g = registry.get<Gravity>(entity);
	    std::cout << " " << t.position[0] << "," << t.position[1] << "," << t.position[2] << std::endl
		      << " " << t.rotation[0] << "," << t.rotation[1] << "," << t.rotation[2] << std::endl
		      << " " << t.scale[0] << "," << t.scale[1] << "," << t.scale[2] << std::endl;
	    std::cout << " " << g.force[0] << "," << g.force[1] << "," << g.force[2] << std::endl;
    }

    for (auto entity: registry.extract<Gravity>())
    {
	    auto &g = registry.get<Gravity>(entity);
	    std::cout << "Entity: " << entity
		      << " " << g.force[0] << "," << g.force[1] << "," << g.force[2] << std::endl;
    }

    registry.remove(entity);
    std::cout << "remove the trasform for 0" << std::endl;
    std::cout << "Sizeof Pool<Transform>: " << registry.size<Transform>() << std::endl;

    return 0;
}
