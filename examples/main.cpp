#include "include/registry.hpp"
#include "include/copy_ptr.hpp"
#include "include/array.hpp"
#include "include/buffers.hpp"

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

static void print_buf(const mozaic::buffer2d& buf)
{
	for (mozaic::ssize_t row = buf.height() - 1; row >= 0; --row)
	{
		for (size_t col = 0; col < buf.stride(); ++col)
		{
			std::cout << static_cast<char>(buf.buffer()[row * buf.stride() + col]) << " ";
		}
		std::cout << std::endl;
	}
}

int main()
{
	std::cout << "Welcome to Mozaic!" << std::endl;
	
	//mozaic::registry<MyStruct, unsigned short, Constructor1, Constructor2> reg;
	
	//auto h1 = reg.construct(Constructor1{ 3.0f });
	//auto h2 = reg.construct(Constructor2{ true });

	//reg.get(h1)->print();
	//reg.get(h2)->print();

	//mozaic::copy_ptr<MyChild> p1(new MyChild(1.0f, true, "hi"));
	//p1->print();
	//mozaic::copy_ptr<MyStruct> p2;
	//p2 = p1;
	//p1->x = 3.0f;
	//p1->print();
	//p2->print();

	//const mozaic::copy_ptr<MyStruct> p3(4.0f, false);
	//p3->print();

	size_t w = 4;
	size_t h = 5;
	char* buf_data = new char[w * h * 1]{
		'a', 'b', 'c', 'd',
		'e', 'f', 'g', 'h',
		'i', 'j', 'k', 'l',
		'm', 'n', 'o', 'p',
		'q', 'r', 's', 't'
	};
	mozaic::buffer2d buf((std::byte*)buf_data, w, h, 1);
	print_buf(buf);
	std::cout << std::endl;
	buf.flip_vertically();
	print_buf(buf);
	std::cout << std::endl;
	buf.flip_horizontally();
	print_buf(buf);
	std::cout << std::endl;
	buf.flip_vertically();
	print_buf(buf);
	std::cout << std::endl;
	
	mozaic::horizontal_line hline;
	hline.y = 1;
	hline.x0 = 1;
	hline.x1 = 3;
	char p0[1] = {'x'};
	buf.set((std::byte*)p0, hline);
	print_buf(buf);
	std::cout << std::endl;

	mozaic::vertical_line vline;
	vline.x = 0;
	vline.y0 = 1;
	vline.y1 = 4;
	char p1[1] = { 'y' };
	buf.set((std::byte*)p1, vline);
	print_buf(buf);
	std::cout << std::endl;

	mozaic::upright_rect urect;
	urect.x0 = 1;
	urect.x1 = 2;
	urect.y0 = 0;
	urect.y1 = 3;
	char p2[1] = { 'z' };
	buf.set((std::byte*)p2, urect);
	print_buf(buf);
	std::cout << std::endl;

	std::cin.get();
	return 0;
}
