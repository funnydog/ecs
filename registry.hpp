#pragma once

#include <bitset>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>

#include "componentpool.hpp"
#include "view.hpp"

namespace priv
{
// template trick to get the index of the Component in a pack
template<typename...>
struct Index;

// type found
template<typename T, typename... Ts>
struct Index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

// type still not found
template<typename T, typename U, typename... Ts>
struct Index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value> {};
}

template<typename Entity, typename... Components>
class Registry
{
	using pool_type = std::tuple<ComponentPool<Entity, Components>...>;
	using mask_type = std::bitset<sizeof...(Components)+1>;

	template<typename C>
	static constexpr auto indexOf = priv::Index<C, Components...>::value;
	static constexpr auto valid_bit = sizeof...(Components);

public:
	template<typename C>
	static constexpr std::size_t getIndex()
	{
		return indexOf<C>;
	}

	template<typename C>
	std::size_t size() const noexcept
	{
		return std::get<indexOf<C>>(mPool).size();
	}

	template<typename C>
	std::size_t capacity() const noexcept
	{
		return std::get<indexOf<C>>(mPool).capacity();
	}

	template<typename C>
	bool empty() const noexcept
	{
		return std::get<indexOf<C>>(mPool).empty();
	}

	template<typename C>
	bool has(Entity entity) const noexcept
	{
		return std::get<indexOf<C>>(mPool).has(entity);
	}

	template<typename C>
	C& get(Entity entity) noexcept
	{
		return std::get<indexOf<C>>(mPool).get(entity);
	}

	template<typename C>
	C& add(Entity entity, C &&component)
	{
		assert(valid(entity) && "Entity not in registry");
		mEntities[entity].set(indexOf<C>);
		return std::get<indexOf<C>>(mPool).add(
			entity, std::move(component));
	}

	template<typename C>
	void remove(Entity entity)
	{
		assert(valid(entity) && "Entity not in registry");
		mEntities[entity].reset(indexOf<C>);
		std::get<indexOf<C>>(mPool).remove(entity);
	}

	std::size_t size() const noexcept
	{
		return mEntities.size() - mAvailable.size();
	}

	bool valid(Entity entity) const noexcept
	{
		return entity < mEntities.size()
			&& mEntities[entity].test(valid_bit);
	}

	Entity create() noexcept
	{
		Entity entity;
		if (mAvailable.empty())
		{
			entity = mEntities.size();
			mEntities.emplace_back();
		}
		else
		{
			entity = mAvailable.back();
			mAvailable.pop_back();
		}
		mEntities[entity].set(valid_bit);
		return entity;
	}

	void remove(Entity entity)
	{
		assert(valid(entity) && "The entity doesn't exist.");
		(void(remove<Components>(entity)), ...);
		mEntities[entity].reset();
		mAvailable.push_back(entity);
	}

	template<typename... Cs>
	std::enable_if_t<(sizeof...(Cs) == 1), View<pool_type, indexOf<Cs>...>> extract() noexcept
	{
		return View<pool_type, indexOf<Cs>...>(mPool);
	}

	template<typename... Cs>
	std::enable_if_t<(sizeof...(Cs) > 1), View<pool_type, indexOf<Cs>...>> extract() noexcept
	{
		return View<pool_type, indexOf<Cs>...>(mPool, mEntities);
	}

private:
	std::vector<mask_type> mEntities;
	std::vector<Entity>    mAvailable;
	pool_type              mPool;
};

template<typename... Cs>
using DefaultRegistry = Registry<std::uint32_t, Cs...>;
