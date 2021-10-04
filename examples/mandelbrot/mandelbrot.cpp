#include "../api.h"
#include <lodepng.h>
#include <cmath>
#include <array>

inline constexpr uint32_t bgr24(uint32_t r, uint32_t g, uint32_t b) {
	return r | (g << 8) | (b << 16) | (255 << 24);
}

static constexpr std::array<uint32_t, 16> color_mapping {
	bgr24(66, 30, 15),
	bgr24(25, 7, 26),
	bgr24(9, 1, 47),
	bgr24(4, 4, 73),
	bgr24(0, 7, 100),
	bgr24(12, 44, 138),
	bgr24(24, 82, 177),
	bgr24(57, 125, 209),
	bgr24(134, 181, 229),
	bgr24(211, 236, 248),
	bgr24(241, 233, 191),
	bgr24(248, 201, 95),
	bgr24(255, 170, 0),
	bgr24(204, 128, 0),
	bgr24(153, 87, 0),
	bgr24(106, 52, 3),
};

inline void encode_color(uint32_t* px, int count)
{
	*px = color_mapping[count & 15];
}
inline void encode_colors(uint32_t* px, int count)
{
	for (int i = 0; i < count; i += 4) {
		px[i+0] = color_mapping[px[i+0] & 15];
		px[i+1] = color_mapping[px[i+1] & 15];
		px[i+2] = color_mapping[px[i+2] & 15];
		px[i+3] = color_mapping[px[i+3] & 15];
	}
}

template <int DimX, int DimY, int MaxCount>
std::array<uint32_t, DimX * DimY>
fractal_nosimd(float left, float top, float xside, float yside)
{
	std::array<uint32_t, DimX * DimY> bitmap {};

	// setting up the xscale and yscale
	const float xscale = xside / DimX;
	const float yscale = yside / DimY;

	// scanning every point in that rectangular area.
	// Each point represents a Complex number (x + yi).
	// Iterate that complex number
	for (int y = 0; y < DimY; y++)
	for (int x = 0; x < DimX; x++)
	{
		float c_real = x * xscale + left;
		float c_imag = y * yscale + top;
		float z_real = 0;
		float z_imag = 0;
		int count = 0;

		// Calculate whether c(c_real + c_imag) belongs
		// to the Mandelbrot set or not and draw a pixel
		// at coordinates (x, y) accordingly
		// If you reach the Maximum number of iterations
		// and If the distance from the origin is
		// greater than 2 exit the loop
		while ((z_real * z_real + z_imag * z_imag < 4)
			&& (count < MaxCount))
		{
			// Calculate Mandelbrot function
			// z = z*z + c where z is a complex number
			float tempx =
				z_real * z_real - z_imag * z_imag + c_real;
			z_imag = 2 * z_real * z_imag + c_imag;
			z_real = tempx;
			count++;
		}

		encode_color(&bitmap[x + y * DimX], count);
	}
	return bitmap;
}

#include <simdpp/simd.h>

// Function to draw mandelbrot set
template <int N, int DimX, int DimY, int MaxCount>
std::array<uint32_t, DimX * DimY>
fractal(float left, float top, float xside, float yside)
{
	SIMDPP_ALIGN(64) std::array<uint32_t, DimX * DimY> bitmap {};
	using namespace simdpp;
	using v_fp32 = float32<N>;
	using v_i32  = int32<N>;

	// setting up the xscale and yscale
	const v_fp32 xscale = splat(xside / DimX);
	const v_fp32 yscale = splat(yside / DimY);
	const v_fp32 vleft = splat(left);
	const v_fp32 vtop  = splat(top);
	const v_fp32 bailout = splat(4.0f);

	const v_i32 zero = splat(0);
	const v_i32 one  = splat(1);
	const v_fp32 vx_array =
		make_float(0, 1, 2, 3, 4, 5, 6, 7,
			8, 9, 10, 11, 12, 13, 14, 15);

	// scanning every point in that rectangular area.
	// Each point represents a Complex number (x + yi).
	// Iterate that complex number
	for (int y = 0; y < DimY; y++) {
		const v_fp32 vy = splat(y);
		const v_fp32 c_imag = vy * yscale + vtop;

		for (int x = 0; x < DimX; x += N)
		{
			const v_fp32 vx = (v_fp32)splat(x) + vx_array;

			const v_fp32 c_real = vx * xscale + vleft;
			v_fp32 z_real = splat(0.0f);
			v_fp32 z_imag = splat(0.0f);
			v_i32 nv = zero;

			// Calculate whether c(c_real + c_imag) belongs
			// to the Mandelbrot set or not and draw a pixel
			// at coordinates (x, y) accordingly
			// If you reach the Maximum number of iterations
			// and If the distance from the origin is
			// greater than 2 exit the loop
			for (int n = 0; n < MaxCount; n++)
			{
				v_fp32 a = fmadd(z_real, z_real, fmadd(0.f - z_imag, z_imag, c_real));
				v_fp32 b = fmadd(z_real, z_imag + z_imag, c_imag);
				a = z_real * z_real - z_imag * z_imag + c_real;
				b = z_real * (z_imag + z_imag) + c_imag;
				z_real = a;
				z_imag = b;

				v_fp32 m = fmadd(a, a, b * b);
				auto mask = m < bailout;
				if (!test_bits_any(blend(one, zero, mask)))
					break;
				nv = nv + (mask & one);
			}

			store(&bitmap[x + y * DimX], nv);
		}
		for (int x = 0; x < DimX; x += N)
		{
			encode_colors(&bitmap[x + y * DimX], N);
		}
	}
	return bitmap;
}

extern "C"
void my_backend(const char*, int, int)
{
	constexpr int counter = 0;
	constexpr size_t width  = 512;
	constexpr size_t height = 512;

	const float factor = powf(2.0, counter * -0.1);
	const float x1 = -1.5;
	const float x2 =  2.0 * factor;
	const float y1 = -1.0 * factor;
	const float y2 =  2.0 * factor;

#ifdef NOSIMD
	auto bitmap = fractal_nosimd<width, height, 120> (x1, y1, x2, y2);
#elifdef AVX512
	auto bitmap = fractal<64, width, height, 120> (x1, y1, x2, y2);
#elifdef SSE
	auto bitmap = fractal<4, width, height, 120> (x1, y1, x2, y2);
#else
	auto bitmap = fractal<8, width, height, 120> (x1, y1, x2, y2);
#endif

	constexpr bool USE_PNG = false;
	if constexpr(USE_PNG) {
		std::vector<uint8_t> png;
		png.reserve(48*1024);
		lodepng::encode(png, (const uint8_t *)bitmap.data(), width, height);
		const char* ctype = "image/png";
		backend_response(200, ctype, strlen(ctype),
			png.data(), png.size());
	} else {
		const char* ctype = "image/raw";
		backend_response(200, ctype, strlen(ctype),
			bitmap.data(), 0);
	}
}

int main()
{
	printf("-== Mandelbrot program ready ==-\n");
	//my_backend(nullptr, 0, 0);
}
