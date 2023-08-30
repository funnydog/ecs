#pragma once

#include <bitset>
#include <cassert>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <vector>

namespace ecs
{

using Entity = std::uint32_t;

template<typename Component>
class ComponentPool
{
public:
	using size_type = typename std::vector<Component>::size_type;
	using iterator = typename std::vector<Entity>::const_iterator;

public:
	bool empty() const noexcept
	{
		return mComponents.empty();
	}

	size_type capacity() const noexcept
	{
		return mComponents.capacity();
	}

	size_type size() const noexcept
	{
		return mComponents.size();
	}

	iterator begin() const noexcept
	{
		return mIndexToEntity.cbegin();
	}

	iterator end() const noexcept
	{
		return mIndexToEntity.cend();
	}

	bool has(Entity entity) const noexcept
	{
		return entity < mEntityToIndex.size()
			&& mEntityToIndex[entity] < mIndexToEntity.size()
			&& mIndexToEntity[mEntityToIndex[entity]] == entity;
	}

	const Component& get(Entity entity) const noexcept
	{
		assert(has(entity) && "Component not found for given entity");
		return mComponents[mEntityToIndex[entity]];
	}

	Component& get(Entity entity) noexcept
	{
		return const_cast<Component&>(
			const_cast<const ComponentPool *>(this)->get(entity));
	}

	Component& add(Entity entity, Component &&component)
	{
		assert(!has(entity) && "Component already added for given entity");
		if (entity >= mEntityToIndex.size())
		{
			mEntityToIndex.resize(entity+1);
		}
		mEntityToIndex[entity] = mIndexToEntity.size();
		mIndexToEntity.push_back(entity);
		mComponents.push_back(std::move(component));

		return mComponents.back();
	}

	void remove(Entity entity)
	{
		if (has(entity))
		{
			auto last = mIndexToEntity.size()-1;
			auto index = mEntityToIndex[entity];
			mEntityToIndex[mIndexToEntity[last]] = index;
			mIndexToEntity[index] = mIndexToEntity[last];
			std::swap(mComponents[index], mComponents[last]);
			mIndexToEntity.pop_back();
			mComponents.pop_back();
		}
	}

	void clear()
	{
		mComponents.clear();
		mIndexToEntity.clear();
		mEntityToIndex.resize(0);
	}

private:
	std::vector<Component>   mComponents;
	std::vector<Entity>      mIndexToEntity;
	std::vector<std::size_t> mEntityToIndex;
};

template<typename Pool, std::size_t...>
class View;

template<typename Pool, std::size_t Index, std::size_t... Other>
class View<Pool, Index, Other...>
{
	using pool_type = Pool;
	using mask_type = std::bitset<std::tuple_size<Pool>::value + 1>;
	using pool_iterator = typename std::tuple_element_t<Index, Pool>::iterator;

private:
	class ViewIterator
	{
	public:
		ViewIterator(pool_iterator begin, pool_iterator end,
			     const std::vector<mask_type> &entities,
			     const mask_type bitmask)
			: mBegin(begin)
			, mEnd(end)
			, mEntities(entities)
			, mBitmask(bitmask)
		{
			while (mBegin != mEnd && !valid())
			{
				++(*this);
			}
		}

		ViewIterator& operator++() noexcept
		{
			do
			{
				++mBegin;
			} while (mBegin != mEnd && !valid());
			return *this;
		}

		bool operator==(const ViewIterator &other) const noexcept
		{
			return mBegin == other.mBegin;
		}

		bool operator!=(const ViewIterator &other) const noexcept
		{
			return mBegin != other.mBegin;
		}

		Entity operator*() noexcept
		{
			return *mBegin;
		}

	private:
		inline bool valid() const noexcept
		{
			return (mEntities[*mBegin] & mBitmask) == mBitmask;
		}

	private:
		pool_iterator mBegin;
		pool_iterator mEnd;
		const std::vector<mask_type> &mEntities;
		const mask_type mBitmask;
	};


public:
	explicit View(pool_type &pool, const std::vector<mask_type> &entities) noexcept
		: mPool(pool)
		, mEntities(entities)
		, mBegin(std::get<Index>(pool).begin())
		, mEnd(std::get<Index>(pool).end())
	{
		std::size_t size = std::get<Index>(mPool).size();
		mBitmask.set(Index);
		(void(mBitmask.set(Other)), ...);
		(void(prefer<Other>(size)), ...);
		(void)size;
	}

	ViewIterator begin() noexcept
	{
		return ViewIterator(mBegin, mEnd, mEntities, mBitmask);
	}

	ViewIterator end() noexcept
	{
		return ViewIterator(mEnd, mEnd, mEntities, mBitmask);
	}

private:
	template<std::size_t I>
	void prefer(std::size_t &size) noexcept
	{
		auto &cpool = std::get<I>(mPool);
		auto cpsize = cpool.size();
		if (cpsize < size)
		{
			size = cpsize;
			mBegin = cpool.begin();
			mEnd = cpool.end();
		}
	}

private:
	const pool_type &mPool;
	const std::vector<mask_type> &mEntities;
	mask_type mBitmask;
	pool_iterator mBegin;
	pool_iterator mEnd;
};

template<typename Pool, std::size_t Index>
class View<Pool, Index>
{
	using pool_type = std::tuple_element_t<Index, Pool>;
public:
	using iterator_type = typename pool_type::iterator;
	using size_type = typename pool_type::size_type;
public:
	explicit View(Pool &pool)
		: mPool(std::get<Index>(pool))
	{
	}

	iterator_type begin() const noexcept
	{
		return mPool.begin();
	}

	iterator_type end() const noexcept
	{
		return mPool.end();
	}

private:
	const pool_type &mPool;
};

// template trick to get the index of the Component in a pack
template<typename...>
struct Index;

// type found
template<typename T, typename... Ts>
struct Index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

// type still not found
template<typename T, typename U, typename... Ts>
struct Index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value> {};

template<typename... Components>
class Registry
{
	using pool_type = std::tuple<ComponentPool<Components>...>;
	using mask_type = std::bitset<sizeof...(Components)+1>;

	template<typename C>
	static constexpr auto indexOf = Index<C, Components...>::value;
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
		(void(std::get<indexOf<Components>>(mPool).remove(entity)), ...);
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

}
