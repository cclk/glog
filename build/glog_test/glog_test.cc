#include "../logger/logger.h"

#ifdef _WIN32
#include <Windows.h>
#endif

int main(int argc, char* argv[])
{
	InitLogger("ttt", "", 0, 1);
	LOG_INFO << "========================";

	while (1)
	{
		LOG_DEBUG << "debug";
		LOG_INFO << "info";
		LOG_WARN << "warn";
		LOG_ERROR << "local time is " << 0;

#ifdef _WIN32
		Sleep(10);
#else
		usleep(100);
#endif
	}

	return 0;
}
