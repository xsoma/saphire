#include "custom_sounds.hpp"

__forceinline void setup_sounds()
{
	CreateDirectory("csgo\\sound", nullptr);
	FILE* file = nullptr;

	file = fopen(crypt_str("csgo\\sound\\cod.wav"), crypt_str("wb"));
	fwrite(cod, sizeof(unsigned char), 11752, file);
	fclose(file);

	file = fopen(crypt_str("csgo\\sound\\bubble.wav"), crypt_str("wb"));
	fwrite(bubble, sizeof(unsigned char), 41550, file);
	fclose(file);

	file = fopen(crypt_str("csgo\\sound\\flick.wav"), crypt_str("wb"));
	fwrite(flick, sizeof(unsigned char), 152168, file);
	fclose(file);

}