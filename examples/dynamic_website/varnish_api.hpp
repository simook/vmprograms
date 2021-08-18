#include "../api.h"
#include <string>
#include <vector>

struct Backend {
	static void response(uint16_t status, const std::string& ctype, const std::string& content)
	{
		backend_response(status, ctype.c_str(), ctype.size(), content.c_str(), content.size());
		__builtin_unreachable();
	}

	static void response(uint16_t status, const std::string& ctype, const std::vector<uint8_t>& content)
	{
		backend_response(status, ctype.c_str(), ctype.size(), content.data(), content.size());
		__builtin_unreachable();
	}

	static void response(uint16_t status, const std::string& ctype, const void* data, size_t len)
	{
		backend_response(status, ctype.c_str(), ctype.size(), data, len);
		__builtin_unreachable();
	}
};

struct Storage {
	static size_t call(storage_func func, std::string& str) {
		return storage_call(func, nullptr, 0, str.data(), str.size());
	}
	static size_t call(storage_func func, std::vector<uint8_t>& vec) {
		return storage_call(func, nullptr, 0, vec.data(), vec.size());
	}
	static size_t call(storage_func func, void* in_data, size_t in_len, std::string& str) {
		return storage_call(func, in_data, in_len, str.data(), str.size());
	}
	static size_t call(storage_func func, void* in_data, size_t in_len, std::vector<uint8_t>& vec) {
		return storage_call(func, in_data, in_len, vec.data(), vec.size());
	}

	static void response(const std::string& result) {
		storage_return(result.data(), result.size());
	}
	static void response(const std::vector<uint8_t>& result) {
		storage_return(result.data(), result.size());
	}
};
