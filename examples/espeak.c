#include "api.h"
#include <espeak-ng/speak_lib.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

char *path = NULL;
void* user_data;
unsigned int *identifier;

constexpr uint32_t LE32(char a, char b, char c, char d) {
	return (a << 0) | (b << 8) | (c << 16) | (d << 24);
}

struct WAV_header {
	uint32_t riff = LE32('R', 'I', 'F', 'F');
	uint32_t size;
	uint32_t wave = LE32('W', 'A', 'V', 'E');
	uint32_t fmt  = LE32('f', 'm', 't', ' ');
	uint32_t fmt_length;
	uint16_t format   = 1; // PCM 16-bit
	uint16_t channels = 1; // Channels
	uint32_t samplerate = 22050;
	uint32_t byterate;
	uint16_t blockalign;
	uint16_t bits_per_sample = 16;
	uint32_t begin_data = LE32('d', 'a', 't', 'a');
	uint32_t data_size;
} __attribute__((packed));
static_assert(sizeof(WAV_header) == 44);

static void writeWAV(FILE* f, const std::vector<char>& wav)
{
	fwrite(wav.data(), 1, wav.size(), f);
}

static void setupWAV(std::vector<char>& wav,
	int samplerate, int channels)
{
	WAV_header header;
	const uint32_t subchunk1_size = 16;
	const uint32_t subchunk2_size = 0 * header.bits_per_sample / 8;

	header.size = 36 + subchunk2_size;
	header.fmt_length = subchunk1_size;
	header.samplerate = samplerate;
	header.channels   = channels;
	header.byterate = header.samplerate * channels * header.bits_per_sample / 8;
	header.blockalign = channels * header.bits_per_sample / 8;
	header.data_size = subchunk2_size;

	auto* d = (const char *)&header;
	wav = { d, d + sizeof(header) };
}
static void finalizeWAV(std::vector<char>& wav,
	size_t samples)
{
	assert(wav.size() >= sizeof(WAV_header));

	auto& header = *(WAV_header*)wav.data();
	const uint32_t subchunk2_size = samples * header.bits_per_sample / 8;

	header.size = 36 + subchunk2_size;
	header.data_size = subchunk2_size;
}

static std::vector<char> wav;
static size_t sampleCount = 0;

static int callback(short* s, int length, espeak_EVENT* ev)
{
	printf("Event: %u  Samples: %d\n", ev->type, length);
	auto* d = (const char *)s;
	wav.insert(wav.end(), d, d + 2 * length);
	sampleCount += length;
	return 0;
}

static const int buflength = 500;
int main()
{
	const char voicename[] = {"English"}; // Set voice by its name
	espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, buflength, path, 0x0);
	espeak_SetVoiceByName(voicename);
	espeak_SetSynthCallback(callback);

	printf("*** espeak-ng initialized ***\n");
}

extern "C" __attribute__((used))
void my_backend(const char *text)
{
	static const unsigned int flags = espeakCHARS_AUTO;
	setupWAV(wav, 22050, 1);

	printf("Saying  '%s'...\n", text);
	espeak_ERROR result =
		espeak_Synth(text, buflength, 0, POS_CHARACTER, 0, flags, identifier, user_data);
	//printf("Result: %d\n", result);
	(void) result;

	finalizeWAV(wav, sampleCount);

	const char mtype[] = "audio/wav";
	backend_response(mtype, sizeof(mtype)-1, wav.data(), wav.size());
}
