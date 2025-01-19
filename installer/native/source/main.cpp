#include "platform.h"
#include "cpuinfo_x86.h"

#include <filesystem>
#include <iostream>
#include <fstream>

#include <chrono>
#include <thread>

using namespace cpu_features;
using namespace std::filesystem;

static const X86Features features = GetX86Info().features;

int main(int argc, char* argv[])
{
	path output_filepath(argv[1]);
	std::ofstream file(output_filepath, std::ios::out);
	file << get_user_cep().string() << std::endl;

	if (features.avx)
	{
		file << "avx,";
	}

	if (features.avx2)
	{
		file << "avx2,";
	}

	if (features.sse)
	{
		file << "sse,";
	}

	if (features.sse2)
	{
		file << "sse2,";
	}

	if (features.sse3)
	{
		file << "sse3,";
	}

	if (features.sse4a)
	{
		file << "sse4a,";
	}

	if (features.sse4_1)
	{
		file << "sse41,";
	}

	if (features.sse4_2)
	{
		file << "sse42";
	}

	return 0;
}