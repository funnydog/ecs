#include <iostream>
#include <random>

#include <glm/glm.hpp>

#include "ecs.hpp"

#define PROJECT_NAME "ecs"

struct A
{
};

struct B
{
};

struct C
{
};

struct D
{
};

int main(int argc, char **argv)
{
    if(argc != 1)
    {
        std::cout << argv[0] <<  "takes no arguments.\n";
        return 1;
    }
    std::cout << "This is project " << PROJECT_NAME << ".\n";

    std::default_random_engine engine;
    std::uniform_int_distribution<> randInt(0, 16);

    ecs::Registry<A, B, C, D> reg;
    std::size_t counts[4] = {};
    for (std::size_t i = 0; i < 1000; ++i)
    {
	    auto e = reg.create();
	    int value = randInt(engine);
	    if (value & 1)
	    {
		    reg.add<A>(e, A{});
		    counts[0]++;
	    }
	    if (value & 2)
	    {
		    reg.add<B>(e, B{});
		    counts[1]++;
	    }
	    if (value & 4)
	    {
		    reg.add<C>(e, C{});
		    counts[2]++;
	    }
	    if (value & 8)
	    {
		    reg.add<D>(e, D{});
		    counts[3]++;
	    }
    }

    std::cerr << "Counts for A, B, C, D: " << counts[0]
	      << ", " << counts[1]
	      << ", " << counts[2]
	      << ", " << counts[3] << "\n";

    std::cerr << "Pool Counts:           " << reg.size<A>()
	      << ", " << reg.size<B>()
	      << ", " << reg.size<C>()
	      << ", " << reg.size<D>() << "\n";

    std::vector<ecs::Entity> remove;
    std::size_t count = 0;
    for (auto e: reg.extract<A, B>())
    {
	    if (randInt(engine) == 0)
	    {
		    remove.push_back(e);
	    }
	    count++;
    }
    std::cerr << "Pool count <A, B>: " << count << "\n";
    std::cerr << "Removing " << remove.size() << " entities\n";
    for (auto e: remove)
    {
		reg.destroy(e);
    }
    count = 0;
    for (auto e: reg.extract<A, B>())
    {
	    ++count;
	    auto &a = reg.get<A>(e);
	    auto &b = reg.get<B>(e);
	    (void)a;
	    (void)b;
    }
    std::cerr << "Pool count <A, B>: " << count << "\n";
    return 0;
}
