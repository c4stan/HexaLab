#ifndef _HL_COMMON_H_
#define _HL_COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define HL_LOG(...) printf(__VA_ARGS__)	

#define HL_ASSERT(cond) assert(cond)
//if (!(cond)) { return false; }

#define HL_ASSERT_LOG(cond, ...) if (!(cond)) { printf(__VA_ARGS__); return false; }

namespace HexaLab {
	using js_ptr = uintptr_t;

	using Index = int32_t;
}

#endif