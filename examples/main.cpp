#include "include/registry.hpp"

#include <iostream>

struct Constructor1
{
	float x;

	bool operator==(const Constructor1& other) const { return x == other.x; }
};

struct Constructor2
{
	bool y;

	bool operator==(const Constructor2& other) const { return y == other.y; }
};

template<>
struct std::hash<Constructor1>
{
	size_t operator()(const Constructor1& c) const { return std::hash<float>{}(c.x); }
};

template<>
struct std::hash<Constructor2>
{
	size_t operator()(const Constructor2& c) const { return std::hash<bool>{}(c.y); }
};

struct MyStruct
{
	float x;
	bool y;

	MyStruct(const Constructor1& c1) : x(c1.x), y(false) {}
	MyStruct(const Constructor2& c2) : x(-1.0f), y(c2.y) {}

	void print() const { std::cout << "x=" << x << ", y=" << (y ? "true" : "false") << std::endl; }
};

int main()
{
	std::cout << "Welcome to Mozaic!" << std::endl;
	
	mozaic::registry<MyStruct, unsigned short, Constructor1, Constructor2> reg;
	
	auto h1 = reg.construct(Constructor1{ 3.0f });
	auto h2 = reg.construct(Constructor2{ true });

	reg.get(h1)->print();
	reg.get(h2)->print();
	
	std::cin.get();
	return 0;
}
