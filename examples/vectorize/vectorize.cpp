#include "../api.h"
#include <cmath>
#include <array>
#include <numeric>
#include <string>
#include <immintrin.h>
//#include <simdpp/simd.h>

template <size_t N>
struct VectorArray {
	alignas(64) std::array<float, N> f32;
};

template <size_t N>
float dotprod_nosimd(const VectorArray<N>& va, const VectorArray<N>& vb)
{
	auto* f1 = &va.f32[0];
	auto* f2 = &vb.f32[0];
	float sum = 0.f;

	for (size_t i = 0; i < N; i++) {
		sum += f1[i] * f2[i];
	}

	return sum;
}

template <size_t N>
float dotprod_sse(const VectorArray<N>& va, const VectorArray<N>& vb)
{
	auto* f1 = (__m128*) &va.f32[0];
	auto* f2 = (__m128*) &vb.f32[0];
	union {
		float f32[4];
		__m128 f32x4;
	} sum;
	sum.f32x4 = _mm_set1_ps(0.0f);

	#pragma unroll(4)
	for (size_t i = 0; i < N / 4; i += 4) {
		sum.f32x4 = _mm_add_ps(sum.f32x4, _mm_mul_ps(f1[i], f2[i]));
		//sum.f32x4 = _mm_fmadd_ps(sum.f32x4, f1[i], f2[i]);
	}

	return sum.f32[0] + sum.f32[1] + sum.f32[2] + sum.f32[3];
}

template <size_t N>
float dotprod_avx(const VectorArray<N>& va, const VectorArray<N>& vb)
{
	auto* f1 = (__m256*) &va.f32[0];
	auto* f2 = (__m256*) &vb.f32[0];
	union {
		std::array<float, 8> f32;
		__m256 f32x8;
	} sum;
	sum.f32x8 = _mm256_set1_ps(0.0f);

	#pragma unroll(4)
	for (size_t i = 0; i < N / 8; i += 8) {
		sum.f32x8 = _mm256_fmadd_ps(sum.f32x8, f1[i], f2[i]);
	}

	return std::accumulate(sum.f32.begin(), sum.f32.end(), 0);
}

template <size_t N>
float dotprod_avx512(const VectorArray<N>& va, const VectorArray<N>& vb)
{
	auto* f1 = (__m512*) &va.f32[0];
	auto* f2 = (__m512*) &vb.f32[0];
	union {
		std::array<float, 16> f32;
		__m512 f32x16;
	} sum;
	sum.f32x16 = _mm512_set1_ps(0.0f);

	#pragma unroll(4)
	for (size_t i = 0; i < N / 16; i += 16) {
		sum.f32x16 = _mm512_fmadd_ps(sum.f32x16, f1[i], f2[i]);
	}

	return _mm512_reduce_add_ps(sum.f32x16);
}

extern "C"
void my_backend(const char*, int, int)
{
	constexpr size_t N = 1024*1024;
	auto a = new VectorArray<N>;
	auto b = new VectorArray<N>;

#if defined(NOSIMD)
	auto result = dotprod_nosimd(*a, *b);
#elif defined(AVX512)
	auto result = dotprod_avx512(*a, *b);
#elif defined(SSE)
	auto result = dotprod_sse(*a, *b);
#else // AVX2
	auto result = dotprod_avx(*a, *b);
#endif

	const char* ctype = "application/octet-stream";
	const std::string result_str = std::to_string(result);
	backend_response(200, ctype, strlen(ctype),
		result_str.c_str(), result_str.size());
}

int main()
{
	printf("-== Vector SIMD program ready ==-\n");
	//my_backend(nullptr, 0, 0);
}
