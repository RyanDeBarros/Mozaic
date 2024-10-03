#pragma once

#include <stdexcept>
#include <string>

namespace mozaic
{
	// TODO polymorphic subtypes
	template<typename T, size_t Len, bool Initialize = true>
	class array
	{
		template<typename U>
		friend class array;

		T* _arr = nullptr;

	public:
		array();
		~array();
		explicit array(T* raw_heap_array, size_t len);
		explicit array(const T& val);
		template<typename... Args> explicit array(Args&&... args);
		template<bool I> array(const array<T, Len, I>& other);
		template<bool I> array(array<T, Len, I>&& other) noexcept;
		template<bool I> array& operator=(const array<T, Len, I>& other);
		template<bool I> array& operator=(array<T, Len, I>&& other) noexcept;
		operator bool() const { return static_cast<bool>(_arr); }
		T* get() { return _arr; }
		const T* get() const { return _arr; }
		constexpr size_t length() const { return Len; }
		T& operator[](size_t i) { return _arr[i]; }
		const T& operator[](size_t i) const { return _arr[i]; }
		void copy(size_t pos, T* arr, size_t len);
		void move(size_t pos, T* arr, size_t len);
		template<size_t SubLen, bool SubInit = Initialize> array<T, SubLen, SubInit> subarray(size_t pos) const;
		template<bool I> void swap(array<T, Len, I>& other);

		struct bad_length_error : std::runtime_exception
		{
			bad_length_error(const std::string& message = "Array length is incompatible with allocated length (" + std::to_string(Len) + ")") : std::runtime_exception(message) {}
			bad_length_error(size_t len) : std::runtime_exception("Array length (" + std::to_string(len) + ") is incompatible with allocated length (" + std::to_string(Len) + ")") {}
		};
	};
	template<typename T, size_t Len, bool Initialize>
	inline array<T, Len, Initialize>::array()
	{
		if constexpr (Initialize)
			_arr = new T[Len]();
		else
			_arr = new T[Len];
	}
	template<typename T, size_t Len, bool Initialize>
	inline array<T, Len, Initialize>::~array()
	{
		delete[] _arr;
	}
	template<typename T, size_t Len, bool Initialize>
	inline array<T, Len, Initialize>::array(T* raw_heap_array, size_t len)
	{
		if (len != Len)
			throw bad_length_error(len);
		_arr = raw_heap_array;
	}
	template<typename T, size_t Len, bool Initialize>
	inline array<T, Len, Initialize>::array(const T& val)
	{
		static_assert(Initialize, "Cannot initialize non-initializing array.");
		_arr = new T[Len](val);
	}
	template<typename T, size_t Len, bool Initialize>
	template<typename ...Args>
	inline array<T, Len, Initialize>::array(Args && ...args)
	{
		_arr = new T[Len](std::forward<Args>(args)...);
	}
	template<typename T, size_t Len, bool Initialize>
	template<bool I>
	inline array<T, Len, Initialize>::array(const array<T, Len, I>& other) : _arr(new T[Len])
	{
		for (size_t i = 0; i < Len; ++i)
			new (_arr + i) T(other._arr[i]);
	}
	template<typename T, size_t Len, bool Initialize>
	template<bool I>
	inline array<T, Len, Initialize>::array(array<T, Len, I>&& other) noexcept : _arr(other._arr)
	{
		other._arr = nullptr;
	}
	template<typename T, size_t Len, bool Initialize>
	template<bool I>
	inline array<T, Len, Initialize>& array<T, Len, Initialize>::operator=(const array<T, Len, I>& other)
	{
		if (_arr != other._arr)
		{
			for (size_t i = 0; i < Len; ++i)
				_arr[i] = T(other._arr[i]);
		}
		return *this;
	}
	template<typename T, size_t Len, bool Initialize>
	template<bool I>
	inline array<T, Len, Initialize>& array<T, Len, Initialize>::operator=(array<T, Len, I>&& other) noexcept
	{
		if (_arr != other._arr)
		{
			delete[] _arr;
			_arr = other._arr;
			other._arr = nullptr;
		}
		return *this;
	}
	template<typename T, size_t Len, bool Initialize>
	inline void array<T, Len, Initialize>::copy(size_t pos, T* arr, size_t len)
	{
		if (pos + len > Len)
			throw bad_length_error(pos + len);
		for (size_t i = pos; i < Len; ++i)
			_arr[i] = arr[i];
	}
	template<typename T, size_t Len, bool Initialize>
	inline void array<T, Len, Initialize>::move(size_t pos, T* arr, size_t len)
	{
		if (pos + len > Len)
			throw bad_length_error(pos + len);
		for (size_t i = pos; i < Len; ++i)
			_arr[i] = std::move(arr[i]);
	}
	template<typename T, size_t Len, bool Initialize>
	template<size_t SubLen, bool SubInit>
	inline array<T, SubLen, SubInit> array<T, Len, Initialize>::subarray(size_t pos) const
	{
		array<T, SubLen, SubInit> sub;
		sub.copy(0, _arr + pos, SubLen);
		return sub;
	}
	template<typename T, size_t Len, bool Initialize>
	template<bool I>
	inline void array<T, Len, Initialize>::swap(array<T, Len, I>& other)
	{
		T* temp = _arr;
		_arr = other._arr;
		other._arr = temp;
	}

	template<typename T>
	class var_array
	{
		// TODO
		// subarray can return a variable run-time lengthed array
	};
}
