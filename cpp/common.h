#pragma once

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define HL_LOG(...) printf(__VA_ARGS__)	

namespace HexaLab {
	using u8 = uint8_t;
	using u32 = uint32_t;
	using u64 = uint64_t;
	using js_ptr = uintptr_t;

	using Index = int32_t;
	static const Index NullIndex = 0xffffffff;

	enum class Result : u8 {
		Success,
		Error
	};
}
