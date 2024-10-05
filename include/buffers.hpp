#pragma once

#include <memory>
#include <functional>

namespace mozaic
{
	class buffer2d
	{
		std::byte* _buf;
		size_t _width, _height;
		unsigned char _bpp;

	public:
		buffer2d(std::byte* heap_arr, size_t width, size_t height, unsigned char bpp) : _buf(heap_arr), _width(width), _height(height), _bpp(bpp) {}
		~buffer2d() { delete[] _buf; }
		buffer2d(const buffer2d&);
		buffer2d(buffer2d&&) noexcept;
		buffer2d& operator=(const buffer2d&);
		buffer2d& operator=(buffer2d&&) noexcept;
		operator bool() const { return _width > 0 && _height > 0 && _bpp > 0; /* TODO validity checking throughout other methods. */ }

		const std::byte* buffer() const { return _buf; }
		std::byte* buffer() { return _buf; }
		size_t width() const { return _width; }
		size_t height() const { return _height; }
		size_t bpp() const { return _bpp; }
		size_t stride() const { return _width * _bpp; }
		size_t area() const { return _width * _height * _bpp; }

		void flip_vertically();
		void flip_horizontally();
		void rotate_90();
		void rotate_180() { flip_vertically(); flip_horizontally(); /* TODO combine into 1 operation */ }
		void rotate_270();

		struct path_iterator
		{
			size_t x = 0;
			size_t y = 0;
			buffer2d* b = nullptr;

			path_iterator(buffer2d* buffer) : b(buffer) {}

			std::byte* operator*() const { return b->buffer() + y * b->stride() + x * b->bpp(); }

			bool operator==(const path_iterator& other) const { return x == other.x && y == other.y && b == other.b; }
			bool operator!=(const path_iterator& other) const { return x != other.x || y != other.y || b != other.b; }
		};
		struct path
		{
			virtual void first(path_iterator&) const = 0;
			virtual void last(path_iterator&) const = 0;
			virtual void prev(path_iterator&) const = 0;
			virtual void next(path_iterator&) const = 0;

			path_iterator first(buffer2d* buffer) const { path_iterator pit(buffer); first(pit); return pit; }
			path_iterator last(buffer2d* buffer) const { path_iterator pit(buffer); last(pit); return pit; }
		};

		void iterate_path(const path& path_, const std::function<void(std::byte*)>& func);
		void riterate_path(const path& path_, const std::function<void(std::byte*)>& func);
		void set(std::byte* pixel, const path& path_);
	};
	inline buffer2d::buffer2d(const buffer2d& other) : _width(other._width), _height(other._height), _bpp(other._bpp)
	{
		_buf = new std::byte[other.area()];
		memcpy(_buf, other._buf, other.area());
	}
	inline buffer2d::buffer2d(buffer2d&& other) noexcept : _buf(other._buf), _width(other._width), _height(other._height), _bpp(other._bpp)
	{
		other._buf = nullptr;
	}
	inline buffer2d& buffer2d::operator=(const buffer2d& other)
	{
		if (this != &other)
		{
			if (area() != other.area())
			{
				delete[] _buf;
				_buf = new std::byte[other.area()];
			}
			memcpy(_buf, other._buf, other.area());
			_width = other._width;
			_height = other._height;
			_bpp = other._bpp;
		}
		return *this;
	}
	inline buffer2d& buffer2d::operator=(buffer2d&& other) noexcept
	{
		if (this != &other)
		{
			delete[] _buf;
			_buf = other._buf;
			_width = other._width;
			_height = other._height;
			_bpp = other._bpp;
			other._buf = nullptr;
		}
		return *this;
	}
	inline void buffer2d::flip_vertically()
	{
		size_t stride = _width * _bpp;
		std::byte* temp = new std::byte[stride];
		std::byte* bottom = _buf;
		std::byte* top = _buf + (_height - 1) * stride;
		for (size_t _ = 0; _ < _height >> 1; ++_)
		{
			memcpy(temp, bottom, stride);
			memcpy(bottom, top, stride);
			memcpy(top, temp, stride);
			bottom += stride;
			top -= stride;
		}
		delete[] temp;
	}
	inline void buffer2d::flip_horizontally()
	{
		size_t stride = _width * _bpp;
		std::byte* temp = new std::byte[_bpp];
		std::byte* row = _buf;
		std::byte* left = nullptr;
		std::byte* right = nullptr;
		for (size_t _ = 0; _ < _height; ++_)
		{
			left = row;
			right = row + (_width - 1) * _bpp;
			for (size_t i = 0; i < _width >> 1; ++i)
			{
				memcpy(temp, left, _bpp);
				memcpy(left, right, _bpp);
				memcpy(right, temp, _bpp);
				left += _bpp;
				right -= _bpp;
			}
			row += stride;
		}
		delete[] temp;
	}
	inline void buffer2d::rotate_90()
	{
		// TODO optimize?
		//std::byte* temp = new std::byte[area()];

		// TODO implement

		//delete[] _buf;
		//_buf = temp;
		//std::swap(_width, _height);
	}
	inline void buffer2d::rotate_270()
	{
		// TODO optimize?
	}
	inline void buffer2d::iterate_path(const path& path_, const std::function<void(std::byte*)>& func)
	{
		path_iterator pit = path_.first(this);
		const path_iterator plast = path_.last(this);
		while (true)
		{
			std::byte* pixel = *pit;
			func(pixel);
			if (pit == plast)
				break;
			else
				path_.next(pit);
		}
	}
	inline void buffer2d::riterate_path(const path& path_, const std::function<void(std::byte*)>& func)
	{
		path_iterator pit = path_.last(this);
		const path_iterator pfirst = path_.first(this);
		while (true)
		{
			std::byte* pixel = *pit;
			func(pixel);
			if (pit == pfirst)
				break;
			else
				path_.prev(pit);
		}
	}
	inline void buffer2d::set(std::byte* pixel, const path& path_)
	{
		iterate_path(path_, [pixel, this](std::byte* pos) { memcpy(pos, pixel, _bpp); });
	}

	struct horizontal_line : public buffer2d::path
	{
		size_t x0 = 0, x1 = 0, y = 0;
		void first(buffer2d::path_iterator& pit) const override { pit.x = x0; pit.y = y; }
		void last(buffer2d::path_iterator& pit) const override { pit.x = x1; pit.y = y; }
		void prev(buffer2d::path_iterator& pit) const override { --pit.x; }
		void next(buffer2d::path_iterator& pit) const override { ++pit.x; }
	};
	struct vertical_line : public buffer2d::path
	{
		size_t x = 0, y0 = 0, y1 = 0;
		void first(buffer2d::path_iterator& pit) const override { pit.x = x; pit.y = y0; }
		void last(buffer2d::path_iterator& pit) const override { pit.x = x; pit.y = y1; }
		void prev(buffer2d::path_iterator& pit) const override { --pit.y; }
		void next(buffer2d::path_iterator& pit) const override { ++pit.y; }
	};
	struct upright_rect: public buffer2d::path
	{
		size_t x0 = 0, x1 = 0, y0 = 0, y1 = 0;
		void first(buffer2d::path_iterator& pit) const override { pit.x = x0; pit.y = y0; }
		void last(buffer2d::path_iterator& pit) const override { pit.x = x1; pit.y = y1; }
		void prev(buffer2d::path_iterator& pit) const override { if (pit.x == x0) { pit.x = x1; --pit.y; } else { --pit.x; } }
		void next(buffer2d::path_iterator& pit) const override { if (pit.x == x1) { pit.x = x0; ++pit.y; } else { ++pit.x; } }
	};
}
