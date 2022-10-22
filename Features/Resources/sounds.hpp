#include "custom_sounds.hpp"

__forceinline void setup_sounds()
{
	CreateDirectory("csgo\\sound", nullptr);
	FILE* file = nullptr;

	file = fopen(crypt_str("csgo\\sound\\metallic.wav"), crypt_str("wb"));
	fwrite(metallic, sizeof(unsigned char), 64700, file); //-V575
	fclose(file);

	file = fopen(crypt_str("csgo\\sound\\flick.wav"), crypt_str("wb"));
	fwrite(flick, sizeof(unsigned char), 152168, file);
	fclose(file);

	file = fopen(crypt_str("csgo\\sound\\ding.wav"), crypt_str("wb"));
	fwrite(ding, sizeof(unsigned char), 14102, file);
	fclose(file);

	file = fopen(crypt_str("csgo\\sound\\primordial.wav"), crypt_str("wb"));
	fwrite(primordial, sizeof(unsigned char), 8190, file);
	fclose(file);

	file = fopen(crypt_str("csgo\\sound\\magic.wav"), crypt_str("wb"));
	fwrite(magic, sizeof(unsigned char), 294990, file);
	fclose(file);

	file = fopen(crypt_str("csgo\\sound\\bell.wav"), crypt_str("wb"));
	fwrite(bell, sizeof(unsigned char), 25478, file);
	fclose(file);
}