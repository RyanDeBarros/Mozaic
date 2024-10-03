#pragma once

#include <type_traits>

namespace mozaic
{
	template<typename T>
	class copy_ptr;
	template<typename, typename, typename = void>
	struct __cpy_is_copy_ptr_poly : std::false_type {};
	template<typename Base, typename Derived>
	struct __cpy_is_copy_ptr_poly<copy_ptr<Base>, copy_ptr<Derived>, std::void_t<std::enable_if_t<std::is_base_of_v<Base, Derived>>>> : std::true_type {};
	template<typename Base, typename Derived>
	static constexpr bool __cpy_is_copy_ptr_poly_v = __cpy_is_copy_ptr_poly<Base, Derived>::value;

	template<typename T>
	class copy_ptr
	{
		template<typename U>
		friend class copy_ptr;

		T* _ptr = nullptr;

	public:
		copy_ptr() = default;
		~copy_ptr() { delete _ptr; }
		template<typename U> explicit copy_ptr(U* raw_heap_ptr);
		template<typename U, typename = std::void_t<std::enable_if_t<!__cpy_is_copy_ptr_poly_v<T, U>>>> copy_ptr(const U& obj);
		template<typename U, typename = std::void_t<std::enable_if_t<!__cpy_is_copy_ptr_poly_v<T, U>>>> copy_ptr(U&& obj) noexcept;
		copy_ptr(const copy_ptr<T>& other);
		copy_ptr(copy_ptr<T>&& other) noexcept;
		template<typename U> copy_ptr(const copy_ptr<U>& other);
		template<typename U> copy_ptr(copy_ptr<U>&& other) noexcept;
		template<typename... Args> explicit copy_ptr(Args&&... args);
		copy_ptr& operator=(const copy_ptr<T>& other);
		copy_ptr& operator=(copy_ptr<T>&& other) noexcept;
		template<typename U> copy_ptr& operator=(const copy_ptr<U>& other);
		template<typename U> copy_ptr& operator=(copy_ptr<U>&& other) noexcept;
		T& operator*() { return *_ptr; }
		const T& operator*() const { return *_ptr; }
		T* operator->() { return _ptr; }
		const T* operator->() const { return _ptr; }
		T* get() { return _ptr; }
		const T* get() const { return _ptr; }
	};
	template<typename T>
	template<typename U>
	copy_ptr<T>::copy_ptr(U* raw_heap_ptr) : _ptr(raw_heap_ptr)
	{
		static_assert(std::is_base_of_v<T, U>, "copy_ptr can only be initialized with polymorphic subtype.");
	}
	template<typename T>
	template<typename U, typename>
	copy_ptr<T>::copy_ptr(const U& obj) : _ptr(new U(obj))
	{
		static_assert(std::is_base_of_v<T, U>, "copy_ptr can only be initialized with polymorphic subtype.");
	}
	template<typename T>
	template<typename U, typename>
	copy_ptr<T>::copy_ptr(U&& obj) noexcept : _ptr(new U(std::move(obj)))
	{
		static_assert(std::is_base_of_v<T, U>, "copy_ptr can only be initialized with polymorphic subtype.");
	}
	template<typename T>
	copy_ptr<T>::copy_ptr(const copy_ptr<T>& other) : _ptr(new T(*other._ptr))
	{
	}
	template<typename T>
	copy_ptr<T>::copy_ptr(copy_ptr<T>&& other) noexcept : _ptr(other._ptr)
	{
		other._ptr = nullptr;
	}
	template<typename T>
	template<typename U>
	copy_ptr<T>::copy_ptr(const copy_ptr<U>&other) : _ptr(new U(*other._ptr))
	{
		static_assert(std::is_base_of_v<T, U>, "copy_ptr can only be initialized with polymorphic subtype.");
	}
	template<typename T>
	template<typename U>
	copy_ptr<T>::copy_ptr(copy_ptr<U>&& other) noexcept : _ptr(other._ptr)
	{
		static_assert(std::is_base_of_v<T, U>, "copy_ptr can only be initialized with polymorphic subtype.");
		other._ptr = nullptr;
	}
	template<typename T>
	template<typename... Args>
	copy_ptr<T>::copy_ptr(Args&&... args) : _ptr(new T(std::forward<Args>(args)...))
	{
	}
	template<typename T>
	copy_ptr<T>& copy_ptr<T>::operator=(const copy_ptr<T>& other)
	{
		if (_ptr != other._ptr)
		{
			if (_ptr)
			{
				_ptr->~T();
				new (_ptr) T(*other._ptr);
			}
			else
				_ptr = new T(*other._ptr);
		}
		return *this;
	}
	template<typename T>
	copy_ptr<T>& copy_ptr<T>::operator=(copy_ptr<T>&& other) noexcept
	{
		if (_ptr != other._ptr)
		{
			delete _ptr;
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		return *this;
	}
	template<typename T>
	template<typename U>
	copy_ptr<T>& copy_ptr<T>::operator=(const copy_ptr<U>& other)
	{
		static_assert(std::is_base_of_v<T, U>, "copy_ptr can only be assigned with polymorphic subtype.");
		if (_ptr != other._ptr)
		{
			if (_ptr)
			{
				_ptr->~T();
				new (_ptr) U(*other._ptr);
			}
			else
				_ptr = new U(*other._ptr);
		}
		return *this;
	}
	template<typename T>
	template<typename U>
	copy_ptr<T>& copy_ptr<T>::operator=(copy_ptr<U>&& other) noexcept
	{
		static_assert(std::is_base_of_v<T, U>, "copy_ptr can only be assigned with polymorphic subtype.");
		if (_ptr != other._ptr)
		{
			delete _ptr;
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		return *this;
	}
}
