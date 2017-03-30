#pragma once

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define HL_LOG(...) printf(__VA_ARGS__)	

#define HL_ASSERT(cond) if (!(cond)) { return Result::Error; }

namespace HexaLab {
	using js_ptr = uintptr_t;

	using Index = int32_t;

	enum class Result : uint8_t {
		Success,
		Error
	};
}
