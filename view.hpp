#pragma once

#include <bitset>
#include <tuple>
#include <vector>

template<typename Pool, std::size_t...>
class View;

template<typename Pool, std::size_t Index, std::size_t... Other>
class View<Pool, Index, Other...>
{
	using pool_type = Pool;
	using mask_type = std::bitset<std::tuple_size<Pool>::value + 1>;
	using pool_iterator = typename std::tuple_element_t<Index, Pool>::iterator;
	using entity_type = typename std::tuple_element_t<Index, Pool>::entity_type;

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

		entity_type operator*() noexcept
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
	pool_type &mPool;
	const std::vector<mask_type> &mEntities;
	mask_type mBitmask;
	pool_iterator mBegin;
	pool_iterator mEnd;
};
