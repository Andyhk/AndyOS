#include <AndyOS.h>
#include "stdio.h"
#include "string.h"
#include "math.h"

int main()
{
	//while (1)
	//{
	//	print("T1");;
	//}

	int result = 0;

	int hour = 0;
	int minute = 0;
	int second = 0;

	gettime(hour, minute, second);

	srand(hour * 100 + minute * 10 + second);

	char buf[128];

	uint32* framebuffer = new uint32[1024 * 768];

	while (1)
	{
		for (int i = 0; i < 1024 * 768; i++)
		{
			uint8 r = rand() / 0xFF;
			uint8 g = rand() / 0xFF;
			uint8 b = rand() / 0xFF;
			//framebuffer[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
		}

		gettime(hour, minute, second);
		vprintf(buf, "%i:%i:%i\n", hour, minute, second);
		print(buf);

		sleep(1000);
		//draw(framebuffer);
	}

	while (1);
	return 2;
}