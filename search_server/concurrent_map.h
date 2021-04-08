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

	static const size_t DEFAULT_BUCKET_COUNT = 8;
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

	explicit ConcurrentMap(size_t bucket_count = DEFAULT_BUCKET_COUNT)
		: buckets_(bucket_count)
	{}

	Access operator[](const Key& key) {
		auto index = GetBucketIndex(key);
		return Access(key, buckets_[index]);
	}

	void Erase(const Key& key) {
		auto index = GetBucketIndex(key);
		Bucket& bucket = buckets_[index];
		std::lock_guard guard(bucket.mutex);
		bucket.map.erase(key);
	}

	std::map<Key, Value> BuildOrdinaryMap() {
		std::map<Key, Value> result;
		for (auto& bucket : buckets_) {
			std::lock_guard guard(bucket.mutex);
			for (const auto& [key, value] : bucket.map) {
				result[key] = value;
			}
		}
		return result;
	}

private:
	std::vector<Bucket> buckets_;

	uint64_t GetBucketIndex(const Key& key) const {
		return static_cast<uint64_t>(key) % buckets_.size();
	}
};