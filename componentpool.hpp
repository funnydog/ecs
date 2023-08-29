#pragma once

#include <cassert>
#include <vector>

template<typename Entity, typename Component>
class ComponentPool
{
public:
	using entity_type = Entity;
	using size_type = typename std::vector<Component>::size_type;
	using iterator = typename std::vector<Entity>::iterator;
	using const_iterator = typename std::vector<Entity>::const_iterator;

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

	iterator begin() noexcept
	{
		return mIndexToEntity.begin();
	}

	iterator end() noexcept
	{
		return mIndexToEntity.end();
	}

	const_iterator cbegin() noexcept
	{
		return mIndexToEntity.cbegin();
	}

	const_iterator cend() noexcept
	{
		return mIndexToEntity.end();
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
			mIndexToEntity[index] = mIndexToEntity[last];
			mComponents[index] = std::move(mComponents[last]);

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
