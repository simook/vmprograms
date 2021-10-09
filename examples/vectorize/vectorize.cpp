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

// x = ( x7, x6, x5, x4, x3, x2, x1, x0 )
__attribute__((target("avx2")))
static inline float sum8(__m256 x)
{
    // hiQuad = ( x7, x6, x5, x4 )
    const __m128 hiQuad = _mm256_extractf128_ps(x, 1);
    // loQuad = ( x3, x2, x1, x0 )
    const __m128 loQuad = _mm256_castps256_ps128(x);
    // sumQuad = ( x3 + x7, x2 + x6, x1 + x5, x0 + x4 )
    const __m128 sumQuad = _mm_add_ps(loQuad, hiQuad);
    // loDual = ( -, -, x1 + x5, x0 + x4 )
    const __m128 loDual = sumQuad;
    // hiDual = ( -, -, x3 + x7, x2 + x6 )
    const __m128 hiDual = _mm_movehl_ps(sumQuad, sumQuad);
    // sumDual = ( -, -, x1 + x3 + x5 + x7, x0 + x2 + x4 + x6 )
    const __m128 sumDual = _mm_add_ps(loDual, hiDual);
    // lo = ( -, -, -, x0 + x2 + x4 + x6 )
    const __m128 lo = sumDual;
    // hi = ( -, -, -, x1 + x3 + x5 + x7 )
    const __m128 hi = _mm_shuffle_ps(sumDual, sumDual, 0x1);
    // sum = ( -, -, -, x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 )
    const __m128 sum = _mm_add_ss(lo, hi);
    return _mm_cvtss_f32(sum);
}

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

	return sum8(sum.f32x8);
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

template <size_t N>
struct MPdata {
	const VectorArray<N>& va;
	const VectorArray<N>& vb;
	const int concurrency;
	std::array<float, 8> results;
};

extern "C" __attribute__((target("fma")))
void dotprod_mp_avx(const int vcpu, void *vdata)
{
	constexpr size_t N = 8*1024*1024;
	MPdata<N>& data = *(MPdata<N> *)vdata;
	const size_t SLICE = data.concurrency;
	const size_t offset = (N / SLICE) * vcpu;
	const size_t slice = (N / SLICE) * 1;

	auto* f1 = (__m256*) &data.va.f32[offset];
	auto* f2 = (__m256*) &data.vb.f32[offset];
	union {
		std::array<float, 8> f32;
		__m256 f32x8;
	} sum;
	sum.f32x8 = _mm256_set1_ps(0.0f);

	#pragma unroll(4)
	for (size_t i = 0; i < slice / 8; i += 8) {
		sum.f32x8 = _mm256_fmadd_ps(sum.f32x8, f1[i], f2[i]);
	}

	data.results[vcpu] = sum8(sum.f32x8);
}

extern "C"
void my_backend(const char*, int, int)
{
	constexpr size_t N = 8*1024*1024;
	auto a = new VectorArray<N>;
	auto b = new VectorArray<N>;

#if defined(MULTIPROCESS)
	MPdata<N> data { *a, *b, 8, {0.0f} };
	multiprocess(7, (multiprocess_t)dotprod_mp_avx, &data);
	dotprod_mp_avx(0, &data);
	multiprocess_wait();
	const float result = std::accumulate(
		data.results.begin(), data.results.end(), 0);
#elif defined(NOSIMD)
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
