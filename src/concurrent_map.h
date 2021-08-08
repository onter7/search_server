#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <type_traits>
#include <vector>

template <typename Key, typename Value>
class ConcurrentMap {
private:
	struct Bucket {
		std::mutex mutex;
		std::map<Key, Value> map;
	};

	static const std::size_t DEFAULT_BUCKET_COUNT = 8;
public:
	static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

	struct Access {
		std::lock_guard<std::mutex> guard;
		Value& ref_to_value;
		Access(const Key& key, Bucket& bucket)
			: guard(bucket.mutex)
			, ref_to_value(bucket.map[key])
		{}
	};

	explicit ConcurrentMap(std::size_t bucket_count = DEFAULT_BUCKET_COUNT)
		: buckets_(bucket_count)
	{}

	Access operator[](const Key& key) {
		return Access(key, GetBucket(key));
	}

	void Erase(const Key& key) {
		Bucket& bucket = GetBucket(key);
		std::lock_guard guard(bucket.mutex);
		bucket.map.erase(key);
	}

	std::map<Key, Value> BuildOrdinaryMap() {
		std::map<Key, Value> result;
		for (auto& [mutex, map] : buckets_) {
			std::lock_guard guard(mutex);
			result.insert(map.begin(), map.end());
		}
		return result;
	}

private:
	std::vector<Bucket> buckets_;

	Bucket& GetBucket(const Key& key) {
		return buckets_[static_cast<uint64_t>(key) % buckets_.size()];
	}
};