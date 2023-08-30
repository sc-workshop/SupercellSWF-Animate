#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "Publisher/AtlasGenerator.h"

#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

int main()
{
	vector<fs::path> filepathList;

	fs::path curr = fs::current_path();

	for (const auto& entry : fs::directory_iterator(fs::path(curr) / "images")) {
		filepathList.push_back(entry.path());
	}

	if (filepathList.empty()) {
		cout << "Folder is empty" << endl;
		return 0;
	}

	vector<sc::AtlasGeneratorItem> items;
	for (fs::path& path : filepathList) {
		sc::AtlasGeneratorItem item(
			cv::imread(path.string(), cv::IMREAD_UNCHANGED)
		);
		items.push_back(item);
	}

	sc::AtlasGeneratorConfig config;
	config.maxSize = { 2048, 2048 };
	config.scaleFactor = 2;
	config.progress = function{ [](unsigned cnt) {
			std::cout << "Left: " << cnt << std::endl;
	} };

	vector<cv::Mat> atlases;
	sc::AtlasGeneratorResult res = sc::AtlasGenerator::Generate(items, atlases, config);
	if (res != sc::AtlasGeneratorResult::OK) {
		cout << "Error " << (int)res << endl;
		return 0;
	}

	for (uint32_t i = 0; atlases.size() > i; i++) {
		string postfix = string(i, '_');
		cv::imwrite((fs::path(curr) / "output" / ("sheet" + postfix + ".png")).string(), atlases[i]);
	}

	ofstream file(fs::path(curr) / "output" / "output.txt");

	for (uint32_t i = 0; items.size() > i; i++) {
		sc::AtlasGeneratorItem& item = items[i];
		string name = filepathList[i].string();

		file << name << endl;
		file << (int)item.textureIndex << endl;
		file << item.polygon.size() << endl;

		for (sc::AtlasGeneratorVertex& point : item.polygon) {
			file << point.uv.first << " " << point.uv.second << endl;
		}

		for (sc::AtlasGeneratorVertex& point : item.polygon) {
			file << point.xy.first << " " << point.xy.second << endl;
		}

		file << endl;
	}

	return 0;
}