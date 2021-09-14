#pragma once
#include <vector>
#include <string>
#include <set>
#include <unordered_map>

namespace game_data
{
	struct paint_kit
	{
		int id;
		std::string name;
		std::string szImagePath = { };
		int rarity = 0;
		auto operator < (const paint_kit& other) const -> bool
		{
			return name < other.name;
		}
	};



	extern std::vector<paint_kit> skin_kits;
	extern std::vector<paint_kit> glove_kits;
	extern std::vector<paint_kit> sticker_kits;

	extern auto initialize_kits() -> void;
	extern int ParseSkins();



}

struct skinInfo {
	int seed = -1;
	int paintkit;
	int rarity;
	std::string tagName;
	std::string     shortname; // shortname
	std::string     name;      // full skin name
};

class SkinData {
public:
	std::unordered_map<std::string, std::set<std::string>> weaponSkins;
	std::unordered_map<std::string, skinInfo> skinMap;
	std::unordered_map<std::string, std::string> skinNames;
};

inline SkinData g_SkinData;