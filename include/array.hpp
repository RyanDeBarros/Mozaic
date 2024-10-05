#pragma once

#include <stdexcept>
#include <type_traits>
#include <string>
#include <utility>

// TODO add optional index access range checking. could be a wrapper arround array/var_array
namespace mozaic
{
	typedef std::make_signed_t<size_t> ssize_t;

	// TODO polymorphic subtypes
	template<typename T, size_t Len, bool Initialize = true>
	class array
	{
		template<typename U, size_t L, bool I>
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
		template<bool I> void swap(array<T, Len, I>& other) noexcept;

		struct bad_length_error : std::runtime_error
		{
			bad_length_error(const std::string& message = "array length is incompatible with allocated length (" + std::to_string(Len) + ")") : std::runtime_error(message) {}
			bad_length_error(size_t len) : std::runtime_error("array length (" + std::to_string(len) + ") is incompatible with allocated length (" + std::to_string(Len) + ")") {}
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
	inline array<T, Len, Initialize>::array(const T& val) : _arr(new T[Len](val))
	{
		static_assert(Initialize, "Cannot initialize non-initializing array.");
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
	inline void array<T, Len, Initialize>::swap(array<T, Len, I>& other) noexcept
	{
		std::swap(_arr, other._arr);
	}

	template<typename T>
	class var_array
	{
		// TODO polymorphic subtypes
		// subarray can return a variable run-time lengthed array
		template<typename U>
		friend class var_array;

		T* _arr = nullptr;
		size_t _len;

	public:
		var_array(size_t len = 0, bool initialize = true);
		~var_array();
		explicit var_array(T* raw_heap_array, size_t len);
		explicit var_array(const T& val, size_t len);
		var_array(const var_array<T>& other);
		var_array(var_array<T>&& other) noexcept;
		var_array& operator=(const var_array<T>& other);
		var_array& operator=(var_array<T>&& other) noexcept;
		operator bool() const { return static_cast<bool>(_len); }
		T* get() { return _arr; }
		const T* get() const { return _arr; }
		size_t length() const { return _len; }
		T& operator[](size_t i) { return _arr[i]; }
		const T& operator[](size_t i) const { return _arr[i]; }
		template<bool GrowToFit = false> void copy(size_t pos, T* arr, size_t len);
		template<bool GrowToFit = false> void move(size_t pos, T* arr, size_t len);
		void resize(size_t len, bool initialize = true);
		var_array<T> subarray(size_t pos, size_t len) const;
		var_array<T> subwindow(ssize_t pos, size_t len, bool initialize = true) const;
		void swap(var_array<T>& other) noexcept;
	};
	template<typename T>
	inline var_array<T>::var_array(size_t len, bool initialize) : _arr(initialize ? new T[len]() : new T[len]), _len(len)
	{
	}
	template<typename T>
	inline var_array<T>::~var_array()
	{
		delete[] _arr;
	}
	template<typename T>
	inline var_array<T>::var_array(T* raw_heap_array, size_t len) : _arr(raw_heap_array), _len(len)
	{
	}
	template<typename T>
	inline var_array<T>::var_array(const T& val, size_t len) : _arr(new T[len](val)), _len(len)
	{
	}
	template<typename T>
	inline var_array<T>::var_array(const var_array<T>& other) : _arr(new T[other._len]), _len(other._len)
	{
		for (size_t i = 0; i < _len; ++i)
			new (_arr + i) T(other._arr[i]);
	}
	template<typename T>
	inline var_array<T>::var_array(var_array<T>&& other) noexcept : _arr(other._arr), _len(other._len)
	{
		other._arr = nullptr;
		other._len = 0;
	}
	template<typename T>
	inline var_array<T>& var_array<T>::operator=(const var_array<T>& other)
	{
		if (_arr != other._arr)
		{
			if (_len == other._len)
			{
				for (size_t i = 0; i < _len; ++i)
				{
					_arr[i].~T();
					new (_arr + i) T(other._arr[i]);
				}
			}
			else
			{
				delete[] _arr;
				_len = other._len;
				_arr = new T[_len];
				for (size_t i = 0; i < _len; ++i)
					new (_arr + i) T(other._arr[i]);
			}
		}
		return *this;
	}
	template<typename T>
	inline var_array<T>& var_array<T>::operator=(var_array<T>&& other) noexcept
	{
		if (_arr != other._arr)
		{
			delete[] _arr;
			_arr = other._arr;
			_len = other._len;
			other._arr = nullptr;
			other._len = 0;
		}
		return *this;
	}
	template<typename T>
	template<bool GrowToFit>
	inline void var_array<T>::copy(size_t pos, T* arr, size_t len)
	{
		len += pos;
		if constexpr (GrowToFit)
		{
			if (len <= _len)
			{
				for (size_t i = pos; i < len; ++i)
					_arr[i] = arr[i];
			}
			else
			{
				T* temp = new T[len];
				for (size_t i = 0; i < pos; ++i)
					temp[i] = std::move(_arr[i]);
				for (size_t i = pos; i < len; ++i)
					temp[i] = arr[i];
				delete[] _arr;
				_arr = temp;
				_len = len;
			}
		}
		else
		{
			for (size_t i = pos; i < _len && len; ++i)
				_arr[i] = arr[i];
		}
	}
	template<typename T>
	template<bool GrowToFit>
	inline void var_array<T>::move(size_t pos, T* arr, size_t len)
	{
		len += pos;
		if constexpr (GrowToFit)
		{
			if (len <= _len)
			{
				for (size_t i = pos; i < len; ++i)
					_arr[i] = arr[i];
			}
			else
			{
				T* temp = new T[len];
				for (size_t i = 0; i < pos; ++i)
					temp[i] = std::move(_arr[i]);
				for (size_t i = pos; i < len; ++i)
					temp[i] = std::move(arr[i]);
				delete[] _arr;
				_arr = temp;
				_len = len;
			}
		}
		else
		{
			for (size_t i = pos; i < _len && len; ++i)
				_arr[i] = std::move(arr[i]);
		}
	}
	template<typename T>
	inline void var_array<T>::resize(size_t len, bool initialize)
	{
		if (len < _len)
		{
			T* temp = new T[len];
			for (size_t i = 0; i < len; ++i)
				temp[i] = std::move(_arr[i]);
			delete[] _arr;
			_arr = temp;
			_len = len;
		}
		else if (len > _len)
		{
			T* temp = new T[len];
			for (size_t i = 0; i < _len; ++i)
				temp[i] = std::move(_arr[i]);
			if (initialize)
			{
				for (size_t i = _len; i < len; ++i)
					new (temp + i) T();
			}
			delete[] _arr;
			_arr = temp;
			_len = len;
		}
	}
	template<typename T>
	inline var_array<T> var_array<T>::subarray(size_t pos, size_t len) const
	{
		if (pos > _len)
			return var_array<T>(nullptr, 0);
		if (pos + len > _len)
			len = _len - pos;
		T* arr = new T[len];
		for (size_t i = 0; i < len; ++i)
			arr[i] = _arr[pos + i];
		return var_array<T>(arr, len);
	}
	template<typename T>
	inline var_array<T> var_array<T>::subwindow(ssize_t pos, size_t len, bool initialize) const
	{
		T* temp = new T[len];
		if (initialize)
		{
			for (ssize_t i = 0; i < len && i + pos < 0; ++i)
				new (temp + i) T();
		}
		for (size_t i = 0; i < _len && i < len + pos; ++i)
			new (temp + i - pos) T(_arr[i]);
		if (initialize)
		{
			for (ssize_t i = 0; i < len + pos - _len; ++i)
				new (temp + i + _len - pos) T();
		}
		return var_array<T>(temp, len);
	}
	template<typename T>
	inline void var_array<T>::swap(var_array<T>& other) noexcept
	{
		std::swap(_arr, other._arr);
		std::swap(_len, other._len);
	}
}

namespace std
{
	template<typename T>
	inline void swap(mozaic::var_array<T>& a, mozaic::var_array<T>& b) noexcept
	{
		a.swap(b);
	}

	template<typename T, size_t Len, bool I1, bool I2>
	inline void swap(mozaic::array<T, Len, I1>& a, mozaic::array<T, Len, I2>& b) noexcept
	{
		a.swap(b);
	}
}
