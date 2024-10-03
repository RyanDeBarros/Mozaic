#include "include/registry.hpp"
#include "include/copy_ptr.hpp"

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
	float x = -1.0f;
	bool y = false;

	MyStruct(float x, bool y) : x(x), y(y) {}
	MyStruct(const Constructor1& c1) : x(c1.x), y(false) {}
	MyStruct(const Constructor2& c2) : x(-1.0f), y(c2.y) {}

	virtual void print() const { std::cout << "x=" << x << ", y=" << (y ? "true" : "false") << std::endl; }
};

struct MyChild : public MyStruct
{
	std::string s;

	MyChild(float x, bool y, std::string s) : MyStruct(x, y), s(s) {}

	virtual void print() const override { std::cout << "x=" << x << ", y=" << (y ? "true" : "false") << ", s=\"" << s << "\"" << std::endl; }
};

int main()
{
	std::cout << "Welcome to Mozaic!" << std::endl;
	
	mozaic::registry<MyStruct, unsigned short, Constructor1, Constructor2> reg;
	
	auto h1 = reg.construct(Constructor1{ 3.0f });
	auto h2 = reg.construct(Constructor2{ true });

	reg.get(h1)->print();
	reg.get(h2)->print();

	mozaic::copy_ptr<MyChild> p1(new MyChild(1.0f, true, "hi"));
	p1->print();
	mozaic::copy_ptr<MyStruct> p2;
	p2 = p1;
	p1->x = 3.0f;
	p1->print();
	p2->print();

	const mozaic::copy_ptr<MyStruct> p3(4.0f, false);
	p3->print();
	
	std::cin.get();
	return 0;
}
