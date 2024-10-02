#pragma once

#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <tuple>
#include <string>

namespace mozaic
{
	template<typename T, typename = void>
	struct __reg_casts_to_bool : std::false_type {};
	template<typename T>
	struct __reg_casts_to_bool<T, std::void_t<decltype(static_cast<bool>(std::declval<T>()))>> : std::true_type {};
	template<typename T>
	static constexpr bool __reg_casts_to_bool_v = __reg_casts_to_bool<T>::value;
	template<typename T, typename = void>
	struct __reg_overloads_equals : std::false_type {};
	template<typename T>
	struct __reg_overloads_equals<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : std::true_type {};
	template<typename T>
	static constexpr bool __reg_overloads_equals_v = __reg_overloads_equals<T>::value;
	template<typename T, typename = void>
	struct __reg_is_hashable : std::false_type {};
	template<typename T>
	struct __reg_is_hashable<T, std::void_t<decltype(std::hash<T>{}(std::declval<T>()))>> : std::true_type {};
	template<typename T>
	static constexpr bool __reg_is_hashable_v = __reg_is_hashable<T>::value;

	template<typename Element, typename Handle, typename... Constructors>
	class registry
	{
		static_assert(std::is_integral_v<Handle> && std::is_unsigned_v<Handle>, "Handle type must be an unsigned integral.");
		static_assert(((__reg_is_hashable_v<Constructors> && __reg_overloads_equals_v<Constructors>) && ...), "All constructor types must be hashable - i.e., specialize std::hash and overload ==.");
		static constexpr bool VALIDATE_CONSTRUCTION = __reg_casts_to_bool_v<Element>;

		Handle _current = Handle(1);
		std::unordered_map<Handle, Element> _data;
		std::tuple<std::unordered_map<Constructors, Handle>...> _lookups;

	public:
		static constexpr Handle CAP = Handle(-1);

		registry() = default;
		registry(const registry<Element, Handle, Constructors...>&) = default;
		registry(registry<Element, Handle, Constructors...>&&) = default;
		~registry() = default;

		Element const* get(Handle handle) const;
		Element* get(Handle handle);
		bool destroy(Handle handle);
		Handle add(Element&& element);
		template<typename Constructor>
		Handle construct(const Constructor& constructor);
		template<typename Constructor>
		Handle construct(Constructor&& constructor);

		struct full_error : public std::runtime_error
		{
			full_error() : std::runtime_error("Registry is full: CAP=" + std::to_string(CAP)) {}
		};
	};
	template<typename Element, typename Handle, typename ...Constructors>
	inline Element const* registry<Element, Handle, Constructors...>::get(Handle handle) const
	{
		if (handle == Handle(0)) return nullptr;
		auto iter = _data.find(handle);
		return iter != _data.end() ? &iter->second : nullptr;
	}
	template<typename Element, typename Handle, typename ...Constructors>
	inline Element* registry<Element, Handle, Constructors...>::get(Handle handle)
	{
		if (handle == Handle(0)) return nullptr;
		auto iter = _data.find(handle);
		return iter != _data.end() ? &iter->second : nullptr;
	}
	template<typename Element, typename Handle, typename ...Constructors>
	inline bool registry<Element, Handle, Constructors...>::destroy(Handle handle)
	{
		if (handle == Handle(0)) return false;
		auto iter = _data.find(handle);
		if (iter == _data.end())
			return false;
		_data.erase(iter);
		return true;
	}
	template<typename Element, typename Handle, typename ...Constructors>
	inline Handle registry<Element, Handle, Constructors...>::add(Element&& element)
	{
		if (_current == CAP)
			throw registry::full_error();
		if constexpr (VALIDATE_CONSTRUCTION)
		{
			if (!element)
				return Handle(0);
		}
		Handle handle = _current++;
		_data.emplace(handle, std::move(element));
		return handle;
	}
	template<typename Element, typename Handle, typename ...Constructors>
	template<typename Constructor>
	inline Handle registry<Element, Handle, Constructors...>::construct(const Constructor& constructor)
	{
		auto& lookup = std::get<std::unordered_map<Constructor, Handle>>(_lookups);
		auto iter = lookup.find(constructor);
		if (iter != lookup.end())
			return iter->second;
		if (_current == CAP)
			throw registry::full_error();
		Element element(constructor);
		if constexpr (VALIDATE_CONSTRUCTION)
		{
			if (!element)
				return Handle(0);
		}
		Handle handle = _current++;
		_data.emplace(handle, std::move(element));
		lookup[constructor] = handle;
		return handle;
	}
	template<typename Element, typename Handle, typename ...Constructors>
	template<typename Constructor>
	inline Handle registry<Element, Handle, Constructors...>::construct(Constructor&& constructor)
	{
		auto& lookup = std::get<std::unordered_map<Constructor, Handle>>(_lookups);
		auto iter = lookup.find(constructor);
		if (iter != lookup.end())
			return iter->second;
		Element element(constructor);
		if constexpr (VALIDATE_CONSTRUCTION)
		{
			if (!element)
				return Handle(0);
		}
		if (_current == CAP)
			throw registry::full_error();
		Handle handle = _current++;
		_data.emplace(handle, std::move(element));
		lookup[std::move(constructor)] = handle;
		return handle;
	}
}
