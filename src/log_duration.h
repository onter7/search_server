#pragma once

#include <chrono>
#include <iostream>
#include <string>

class LogDuration {
public:
	explicit LogDuration() = default;
	explicit LogDuration(std::string operation_name, std::ostream& os = std::cerr)
		: operation_name_(operation_name)
		, os_(os)
	{}
	~LogDuration() {
		using namespace std::literals;
		const auto end_time = std::chrono::steady_clock::now();
		const auto dur = end_time - start_time_;
		os_ << operation_name_
			<< ": " << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count()
			<< " ms"s << std::endl;
	}
private:
	const std::chrono::steady_clock::time_point start_time_ = std::chrono::steady_clock::now();
	const std::string operation_name_;
	std::ostream& os_;
};

#define PROFILE_CONCAT_INTERNAL(X, Y) X ## Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)