// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <ShlObj_core.h>
#include <unordered_map>
#include "menu.h"
#include "../ImGui/code_editor.h"
#include "../constchars.h"
#include "../cheats/misc/logs.h"
#include <cheats/postprocessing/PostProccessing.h>
#include <cheats/postprocessing/PostProccessing.h>
#include "../cheats/misc/misc.h"
#include "visuals/logger.hpp"
#define ALPHA (ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar| ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)
#define NOALPHA (ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)

std::vector <std::string> files;
std::vector <std::string> scripts;
std::string editing_script;

auto selected_script = 0;
auto loaded_editing_script = false;

static auto menu_setupped = false;
static auto should_update = true;

std::string comp_name() {

	char buff[MAX_PATH];
	GetEnvironmentVariableA("USERNAME", buff, MAX_PATH);

	return std::string(buff);
}

IDirect3DTexture9* all_skins[36];

std::string get_wep(int id, int custom_index = -1, bool knife = true)
{
	if (custom_index > -1)
	{
		if (knife)
		{
			switch (custom_index)
			{
			case 0: return crypt_str("weapon_knife");
			case 1: return crypt_str("weapon_bayonet");
			case 2: return crypt_str("weapon_knife_css");
			case 3: return crypt_str("weapon_knife_skeleton");
			case 4: return crypt_str("weapon_knife_outdoor");
			case 5: return crypt_str("weapon_knife_cord");
			case 6: return crypt_str("weapon_knife_canis");
			case 7: return crypt_str("weapon_knife_flip");
			case 8: return crypt_str("weapon_knife_gut");
			case 9: return crypt_str("weapon_knife_karambit");
			case 10: return crypt_str("weapon_knife_m9_bayonet");
			case 11: return crypt_str("weapon_knife_tactical");
			case 12: return crypt_str("weapon_knife_falchion");
			case 13: return crypt_str("weapon_knife_survival_bowie");
			case 14: return crypt_str("weapon_knife_butterfly");
			case 15: return crypt_str("weapon_knife_push");
			case 16: return crypt_str("weapon_knife_ursus");
			case 17: return crypt_str("weapon_knife_gypsy_jackknife");
			case 18: return crypt_str("weapon_knife_stiletto");
			case 19: return crypt_str("weapon_knife_widowmaker");
			}
		}
		else
		{
			switch (custom_index)
			{
			case 0: return crypt_str("ct_gloves"); //-V1037
			case 1: return crypt_str("studded_bloodhound_gloves");
			case 2: return crypt_str("t_gloves");
			case 3: return crypt_str("ct_gloves");
			case 4: return crypt_str("sporty_gloves");
			case 5: return crypt_str("slick_gloves");
			case 6: return crypt_str("leather_handwraps");
			case 7: return crypt_str("motorcycle_gloves");
			case 8: return crypt_str("specialist_gloves");
			case 9: return crypt_str("studded_hydra_gloves");
			}
		}
	}
	else
	{
		switch (id)
		{
		case 17: return crypt_str("weapon_ak47");
		case 23: return crypt_str("weapon_aug");
		case 25: return crypt_str("weapon_awp");
		case 5: return crypt_str("weapon_cz75a");
		case 8: return crypt_str("weapon_deagle");
		case 3: return crypt_str("weapon_elite");
		case 20: return crypt_str("weapon_famas");
		case 4: return crypt_str("weapon_fiveseven");
		case 26: return crypt_str("weapon_g3sg1");
		case 21: return crypt_str("weapon_galilar");
		case 0: return crypt_str("weapon_glock");
		case 28: return crypt_str("weapon_m249");
		case 18: return crypt_str("weapon_m4a1_silencer");
		case 19: return crypt_str("weapon_m4a1");
		case 10: return crypt_str("weapon_mac10");
		case 29: return crypt_str("weapon_mag7");
		case 11: return crypt_str("weapon_mp5sd");
		case 12: return crypt_str("weapon_mp7");
		case 13: return crypt_str("weapon_mp9");
		case 30: return crypt_str("weapon_negev");
		case 31: return crypt_str("weapon_nova");
		case 6: return crypt_str("weapon_hkp2000");
		case 1: return crypt_str("weapon_p250");
		case 14: return crypt_str("weapon_p90");
		case 15: return crypt_str("weapon_bizon");
		case 9: return crypt_str("weapon_revolver");
		case 32: return crypt_str("weapon_sawedoff");
		case 27: return crypt_str("weapon_scar20");
		case 24: return crypt_str("weapon_ssg08");
		case 22: return crypt_str("weapon_sg556");
		case 2: return crypt_str("weapon_tec9");
		case 16: return crypt_str("weapon_ump45");
		case 7: return crypt_str("weapon_usp_silencer");
		case 33: return crypt_str("weapon_xm1014");
		case 34: return crypt_str("knife");
		case 35: return crypt_str("gloves");
		default: return crypt_str("unknown");
		}
	}
}

IDirect3DTexture9* get_skin_preview(const char* weapon_name, const std::string& skin_name, IDirect3DDevice9* device)
{
	IDirect3DTexture9* skin_image = nullptr;
	std::string vpk_path;

	if (strcmp(weapon_name, crypt_str("unknown")) && strcmp(weapon_name, crypt_str("knife")) && strcmp(weapon_name, crypt_str("gloves"))) //-V526
	{
		if (skin_name.empty() || skin_name == crypt_str("default"))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + crypt_str(".png");
		else
			vpk_path = crypt_str("resource/flash/econ/default_generated/") + std::string(weapon_name) + crypt_str("_") + std::string(skin_name) + crypt_str("_light_large.png");
	}
	else
	{
		if (!strcmp(weapon_name, crypt_str("knife")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/weapon_knife.png");
		else if (!strcmp(weapon_name, crypt_str("gloves")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else if (!strcmp(weapon_name, crypt_str("unknown")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/weapon_snowball.png");

	}
	const auto handle = m_basefilesys()->Open(vpk_path.c_str(), crypt_str("r"), crypt_str("GAME"));
	if (handle)
	{
		int file_len = m_basefilesys()->Size(handle);
		char* image = new char[file_len]; //-V121

		m_basefilesys()->Read(image, file_len, handle);
		m_basefilesys()->Close(handle);

		D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
		delete[] image;
	}

	if (!skin_image)
	{
		std::string vpk_path;

		if (strstr(weapon_name, crypt_str("bloodhound")) != NULL || strstr(weapon_name, crypt_str("hydra")) != NULL)
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + crypt_str(".png");

		const auto handle = m_basefilesys()->Open(vpk_path.c_str(), crypt_str("r"), crypt_str("GAME"));

		if (handle)
		{
			int file_len = m_basefilesys()->Size(handle);
			char* image = new char[file_len]; //-V121

			m_basefilesys()->Read(image, file_len, handle);
			m_basefilesys()->Close(handle);

			D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
			delete[] image;
		}
	}

	return skin_image;
}

// setup some styles and colors, window size and bg alpha
// dpi setup
void c_menu::menu_setup(ImGuiStyle& style) //-V688
{
	ImGui::StyleColorsClassic(); // colors setup
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Once); // window pos setup
	ImGui::SetNextWindowBgAlpha(min(style.Alpha, 0.94f)); // window bg alpha setup

	styles.WindowPadding = style.WindowPadding;
	styles.WindowRounding = style.WindowRounding;
	styles.WindowMinSize = style.WindowMinSize;
	styles.ChildRounding = style.ChildRounding;
	styles.PopupRounding = style.PopupRounding;
	styles.FramePadding = style.FramePadding;
	styles.FrameRounding = style.FrameRounding;
	styles.ItemSpacing = style.ItemSpacing;
	styles.ItemInnerSpacing = style.ItemInnerSpacing;
	styles.TouchExtraPadding = style.TouchExtraPadding;
	styles.IndentSpacing = style.IndentSpacing;
	styles.ColumnsMinSpacing = style.ColumnsMinSpacing;
	styles.ScrollbarSize = style.ScrollbarSize;
	styles.ScrollbarRounding = style.ScrollbarRounding;
	styles.GrabMinSize = style.GrabMinSize;
	styles.GrabRounding = style.GrabRounding;
	styles.TabRounding = style.TabRounding;
	styles.TabMinWidthForUnselectedCloseButton = style.TabMinWidthForUnselectedCloseButton;
	styles.DisplayWindowPadding = style.DisplayWindowPadding;
	styles.DisplaySafeAreaPadding = style.DisplaySafeAreaPadding;
	styles.MouseCursorScale = style.MouseCursorScale;

	// setup skins preview
	for (auto i = 0; i < vars.skins.skinChanger.size(); i++)
		if (!all_skins[i])
			all_skins[i] = get_skin_preview(get_wep(i, (i == 0 || i == 1) ? vars.skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), vars.skins.skinChanger.at(i).skin_name, device); //-V810

	menu_setupped = true; // we dont want to setup menu again
}

// resize current style sizes
void c_menu::dpi_resize(float scale_factor, ImGuiStyle& style) //-V688
{
	style.WindowPadding = (styles.WindowPadding * scale_factor);
	style.WindowRounding = (styles.WindowRounding * scale_factor);
	style.WindowMinSize = (styles.WindowMinSize * scale_factor);
	style.ChildRounding = (styles.ChildRounding * scale_factor);
	style.PopupRounding = (styles.PopupRounding * scale_factor);
	style.FramePadding = (styles.FramePadding * scale_factor);
	style.FrameRounding = (styles.FrameRounding * scale_factor);
	style.ItemSpacing = (styles.ItemSpacing * scale_factor);
	style.ItemInnerSpacing = (styles.ItemInnerSpacing * scale_factor);
	style.TouchExtraPadding = (styles.TouchExtraPadding * scale_factor);
	style.IndentSpacing = (styles.IndentSpacing * scale_factor);
	style.ColumnsMinSpacing = (styles.ColumnsMinSpacing * scale_factor);
	style.ScrollbarSize = (styles.ScrollbarSize * scale_factor);
	style.ScrollbarRounding = (styles.ScrollbarRounding * scale_factor);
	style.GrabMinSize = (styles.GrabMinSize * scale_factor);
	style.GrabRounding = (styles.GrabRounding * scale_factor);
	style.TabRounding = (styles.TabRounding * scale_factor);
	if (styles.TabMinWidthForUnselectedCloseButton != FLT_MAX) //-V550
		style.TabMinWidthForUnselectedCloseButton = (styles.TabMinWidthForUnselectedCloseButton * scale_factor);
	style.DisplayWindowPadding = (styles.DisplayWindowPadding * scale_factor);
	style.DisplaySafeAreaPadding = (styles.DisplaySafeAreaPadding * scale_factor);
	style.MouseCursorScale = (styles.MouseCursorScale * scale_factor);
}


std::string get_config_dir()
{
	std::string folder;
	static TCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, 0x001a, NULL, NULL, path)))
		folder = std::string(path) + crypt_str("\\saphire\\");

	CreateDirectory(folder.c_str(), NULL);
	return folder;
}

void load_config()
{
	if (cfg_manager->files.empty())
		return;

	cfg_manager->load(cfg_manager->files.at(vars.selected_config), false);
	c_lua::get().unload_all_scripts();

	for (auto& script : vars.scripts.scripts)
		c_lua::get().load_script(c_lua::get().get_script_id(script));

	scripts = c_lua::get().scripts;

	if (selected_script >= scripts.size())
		selected_script = scripts.size() - 1; //-V103

	for (auto& current : scripts)
	{
		if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
			current.erase(current.size() - 5, 5);
		else if (current.size() >= 4)
			current.erase(current.size() - 4, 4);
	}

	for (auto i = 0; i < vars.skins.skinChanger.size(); ++i)
		all_skins[i] = nullptr;

	vars.scripts.scripts.clear();

	cfg_manager->load(cfg_manager->files.at(vars.selected_config), true);
	cfg_manager->config_files();
	notify::add_log("Cheat", crypt_str("Loaded ") + files.at(vars.selected_config) + crypt_str(" config"), Color(vars.misc.menu_color));
}

void save_config()
{
	if (cfg_manager->files.empty())
		return;

	vars.scripts.scripts.clear();

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto script = c_lua::get().scripts.at(i);

		if (c_lua::get().loaded.at(i))
			vars.scripts.scripts.emplace_back(script);
	}

	cfg_manager->save(cfg_manager->files.at(vars.selected_config));
	cfg_manager->config_files();

	notify::add_log("Cheat", crypt_str("Saved ") + files.at(vars.selected_config) + crypt_str(" config"), Color(vars.misc.menu_color));
}

void remove_config()
{
	if (cfg_manager->files.empty())
		return;

	notify::add_log("Cheat", crypt_str("Removed ") + files.at(vars.selected_config) + crypt_str(" config"), Color(vars.misc.menu_color));
	cfg_manager->remove(cfg_manager->files.at(vars.selected_config));
	cfg_manager->config_files();

	files = cfg_manager->files;

	if (vars.selected_config >= files.size())
		vars.selected_config = files.size() - 1; //-V103

	for (auto& current : files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);
}

void add_config()
{
	auto empty = true;

	for (auto current : vars.new_config_name)
	{
		if (current != ' ')
		{
			empty = false;
			break;
		}
	}

	if (empty)
		vars.new_config_name = crypt_str("config");

	notify::add_log("Cheat", crypt_str("Added ") + vars.new_config_name + crypt_str(" config"), Color(vars.misc.menu_color));
	if (vars.new_config_name.find(crypt_str(".cfg")) == std::string::npos)
		vars.new_config_name += crypt_str(".cfg");

	cfg_manager->save(vars.new_config_name);
	cfg_manager->config_files();

	vars.selected_config = cfg_manager->files.size() - 1; //-V103
	files = cfg_manager->files;

	for (auto& current : files)
		if (current.size() > 2)
			current.erase(current.size() - 4, 3);
}

__forceinline void padding(float x, float y)
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x * c_menu::get().dpi_scale);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y * c_menu::get().dpi_scale);
}



void draw_combo(const char* name, int& variable, const char* labels[], int count)
{
	ImGui::SetCursorPosX(8);
	ImGui::Text(name);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * c_menu::get().dpi_scale);
	ImGui::Combo(std::string(crypt_str("##COMBO__") + std::string(name)).c_str(), &variable, labels, count);
}

void draw_combo(const char* name, int& variable, bool (*items_getter)(void*, int, const char**), void* data, int count)
{
	ImGui::SetCursorPosX(8);
	ImGui::Text(name);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * c_menu::get().dpi_scale);
	ImGui::Combo(std::string(crypt_str("##COMBO__") + std::string(name)).c_str(), &variable, items_getter, data, count);
}

void draw_multicombo(std::string name, std::vector<int>& variable, const char* labels[], int count, std::string& preview)
{
	
	ImGui::SetCursorPosX(8);
	ImGui::Text((crypt_str(" ") + name).c_str());

	auto hashname = crypt_str("##") + name; // we dont want to render name of combo

	for (auto i = 0, j = 0; i < count; i++)
	{
		if (variable[i])
		{
			if (j)
				preview += crypt_str(", ") + (std::string)labels[i];
			else
				preview = labels[i];

			j++;
		}
	}

	if (ImGui::BeginCombo(hashname.c_str(), preview.c_str())) // draw start
	{
		ImGui::Spacing();
		ImGui::BeginGroup();
		{

			for (auto i = 0; i < count; i++)
				ImGui::Selectable(labels[i], (bool*)&variable[i], ImGuiSelectableFlags_DontClosePopups);

		}
		ImGui::EndGroup();
		ImGui::Spacing();

		ImGui::EndCombo();
	}

	preview = crypt_str("None"); // reset preview to use later
}

bool LabelClick(const char* label, bool* v, const char* unique_id)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	// The concatoff/on thingies were for my weapon config system so if we're going to make that, we still need this aids.
	char Buf[64];
	_snprintf(Buf, 62, crypt_str("%s"), label);

	char getid[128];
	sprintf_s(getid, 128, crypt_str("%s%s"), label, unique_id);


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(getid);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, ImVec2(label_size.y + style.FramePadding.y * 2 + window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2));
	ImGui::ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;

	if (label_size.x > 0)
	{
		ImGui::SameLine(0, style.ItemInnerSpacing.x);
		const ImRect text_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y));

		ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}

	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
		*v = !(*v);

	if (*v)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(48 / 255.f, (119) / 255.f, (233) / 255.f, 1.f));
	if (label_size.x > 0.0f)
		ImGui::RenderText(ImVec2(check_bb.GetTL().x + 12, check_bb.GetTL().y), Buf);
	if (*v)
		ImGui::PopStyleColor();

	return pressed;

}
ImTextureID getAvatarTexture(int team) 
{
	return team == 2 ? c_menu::get().tt_a : team == 3 ? c_menu::get().ct_a : nullptr;
}
void draw_keybind(const char* label, key_bind* key_bind, const char* unique_id, bool with_color = false)
{
	// reset bind if we re pressing esc
	if (key_bind->key == KEY_ESCAPE)
		key_bind->key = KEY_NONE;
	auto clicked = false;
	bool go = false;
	auto text = " " + (std::string)m_inputsys()->ButtonCodeToString(key_bind->key);
	auto s = ImGui::GetWindowSize();
	if (key_bind->key <= KEY_NONE || key_bind->key >= KEY_MAX) {
		text = crypt_str("None");
		go = false;
	}
	else
		go = true;

	// if we clicked on keybind
	if (hooks::input_shouldListen && hooks::input_receivedKeyval == &key_bind->key)
	{
		clicked = true;
		text = crypt_str("...");
	}

	auto textsize = ImGui::CalcTextSize(text.c_str()).x + 2 * c_menu::get().dpi_scale;

	auto labelsize = ImGui::CalcTextSize(label);

	if (with_color)
		ImGui::SameLine(-1, (s.x) - ImGui::CalcTextSize(text.c_str()).x - 12 - 30);
	else
		ImGui::SameLine(-1, (s.x) - ImGui::CalcTextSize(text.c_str()).x - 12);

	if (ImGui::KeybindButton(text.c_str(), unique_id, ImVec2(ImGui::CalcTextSize(text.c_str()).x + 8, ImGui::CalcTextSize(text.c_str()).y + 4), clicked, ImGuiButtonFlags_::ImGuiButtonFlags_None))
		clicked = true;

	if (clicked)
	{
		hooks::input_shouldListen = true;
		hooks::input_receivedKeyval = &key_bind->key;
	}

	static auto hold = false, toggle = false;

	switch (key_bind->mode)
	{
	case HOLD:
		hold = true;
		toggle = false;
		break;
	case TOGGLE:
		toggle = true;
		hold = false;
		break;
	}
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupRounding, 4);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, 1 * 0.85f));
	if (ImGui::BeginPopup(unique_id))
	{
		
		if (LabelClick(crypt_str("Hold"), &hold, unique_id))
		{
			if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else
			{
				toggle = false;
				key_bind->mode = HOLD;
			}

			ImGui::CloseCurrentPopup();
		}

		if (LabelClick(crypt_str("Toggle"), &toggle, unique_id))
		{
			if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);    ImGui::PopStyleColor(1);
}

void lua_edit(std::string window_name)
{
	std::string file_path;

	auto get_dir = [&]() -> void
	{
		static TCHAR path[MAX_PATH];

		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, NULL, path)))
			file_path = std::string(path) + crypt_str("\\saphire\\lua\\");

		CreateDirectory(file_path.c_str(), NULL);
		file_path += window_name + crypt_str(".lua");
	};

	get_dir();
	const char* child_name = (window_name + window_name).c_str();

	ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_Once);
	ImGui::Begin(window_name.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 5.f);

	static TextEditor editor;

	if (!loaded_editing_script)
	{
		static auto lang = TextEditor::LanguageDefinition::Lua();

		editor.SetLanguageDefinition(lang);
		editor.SetReadOnly(false);

		std::ifstream t(file_path);

		if (t.good()) // does while exist?
		{
			std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
			editor.SetText(str); // setup script content
		}

		loaded_editing_script = true;
	}

	// dpi scale for font
	// we dont need to resize it for full scale
	ImGui::SetWindowFontScale(1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f));

	// new size depending on dpi scale
	ImGui::SetWindowSize(ImVec2(ImFloor(800 * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f))), ImFloor(700 * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f)))));
	editor.Render(child_name, ImGui::GetWindowSize() - ImVec2(0, 66 * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f))));

	// seperate code with buttons
	ImGui::Separator();

	// set cursor pos to right edge of window
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - (16.f * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f))) - (250.f * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f))));
	ImGui::BeginGroup();

	if (ImGui::CustomButton(crypt_str("Save"), (crypt_str("Save") + window_name).c_str(), ImVec2(125 * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f)), 0), true, c_menu::get().icon_font, crypt_str("S")))
	{
		std::ofstream out;

		out.open(file_path);
		out << editor.GetText() << std::endl;
		out.close();
	}

	ImGui::SameLine();

	// TOOD: close button will close window (return in start of function)
	if (ImGui::CustomButton(crypt_str("Close"), (crypt_str("Close") + window_name).c_str(), ImVec2(125 * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f)), 0)))
	{
		csgo.globals.focused_on_input = false;
		loaded_editing_script = false;
		editing_script.clear();
	}

	ImGui::EndGroup();

	ImGui::PopStyleVar();
	ImGui::End();
}
enum key_bind_num
{
	_AUTOFIRE,
	_LEGITBOT,
	_DOUBLETAP,
	_SAFEPOINT,
	_MIN_DAMAGE,
	_ANTI_BACKSHOT = 12,
	_M_BACK,
	_M_LEFT,
	_M_RIGHT,
	_DESYNC_FLIP,
	_THIRDPERSON,
	_AUTO_PEEK,
	_EDGE_JUMP,
	_FAKEDUCK,
	_SLOWWALK,
	_BODY_AIM,
	_RAGEBOT,
	_TRIGGERBOT,
	_L_RESOLVER_OVERRIDE,
	_FAKE_PEEK,
};
void c_menu::keys()
{
	

	static float main_alpha = 0.f;

	bool rage = vars.ragebot.enable, legit = vars.legitbot.enabled, aa = vars.antiaim.enable, vis = vars.player.enable;
	

	int pressed_binds = 0;

	
	static float keys_alpha[15] = {};
	if (key_binds::get().get_key_bind_state(_RAGEBOT) && rage) pressed_binds++;
	if (vars.ragebot.damage_override_key.key > KEY_NONE && vars.ragebot.damage_override_key.key < KEY_MAX && key_binds::get().get_key_bind_state(_MIN_DAMAGE) && rage) pressed_binds++;
	if (key_binds::get().get_key_bind_state(_LEGITBOT) && legit)pressed_binds++;
	if (key_binds::get().get_key_bind_state(_AUTOFIRE) && legit)pressed_binds++;
	if (vars.ragebot.double_tap && misc::get().double_tap_key || key_binds::get().get_key_bind_state(_DOUBLETAP) && rage)pressed_binds++;
	if (vars.antiaim.hide_shots && vars.antiaim.hide_shots_key.key > KEY_NONE && vars.antiaim.hide_shots_key.key < KEY_MAX && misc::get().hide_shots_key && rage)pressed_binds++;
	if (key_binds::get().get_key_bind_state(_BODY_AIM) && rage )pressed_binds++;
	if (key_binds::get().get_key_bind_state(_SAFEPOINT) && rage)pressed_binds++;
	if (key_binds::get().get_key_bind_state(_THIRDPERSON) && vis)pressed_binds++;
	if (key_binds::get().get_key_bind_state(_DESYNC_FLIP) && aa)pressed_binds++;
	if (key_binds::get().get_key_bind_state(_FAKEDUCK) && aa)pressed_binds++;
	if (key_binds::get().get_key_bind_state(_AUTO_PEEK))pressed_binds++;
	if (hooks::menu_open) pressed_binds++;

	key_bind gui;
	gui.mode = (key_bind_mode)(1);

	if (!vars.misc.keybinds)
		return;

	if (hooks::menu_open || pressed_binds > 0) {
		if (main_alpha < 1)
		    main_alpha += 5 * ImGui::GetIO().DeltaTime;
	}
	else 
		if (main_alpha > 0)
		    main_alpha -= 5 * ImGui::GetIO().DeltaTime;

	if (main_alpha < 0.05f)
		return;
	ImGuiStyle* Style = &ImGui::GetStyle();
	ImDrawList* draw;
	Style->WindowRounding = 0;
	Style->WindowBorderSize = 1;
	Style->WindowMinSize = { 1,1 };
	static float alpha = 0.f;
	Style->Colors[ImGuiCol_WindowBg] = ImColor(33, 33, 33, 215);//zochem? ia zhe ubral bg?
	Style->Colors[ImGuiCol_ChildBg] = ImColor(21, 20, 21, 255);
	Style->Colors[ImGuiCol_ResizeGrip] = ImColor(42, 40, 43, 0);
	Style->Colors[ImGuiCol_ResizeGripHovered] = ImColor(42, 40, 43, 0);
	Style->Colors[ImGuiCol_ResizeGripActive] = ImColor(42, 40, 43, 0);
	Style->Colors[ImGuiCol_Border] = ImColor(38, 39, 55, 215);
	Style->Colors[ImGuiCol_Button] = ImColor(29, 125, 229, 5);
	Style->Colors[ImGuiCol_ButtonHovered] = ImColor(29, 125, 229, 5);
	Style->Colors[ImGuiCol_ButtonActive] = ImColor(29, 125, 229, 5);
	Style->Colors[ImGuiCol_PopupBg] = ImColor(18, 17, 18, 255);
	Style->FramePadding = ImVec2(4, 3);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, main_alpha);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 3));
	ImVec2 p, s;
	
	

	ImGui::Begin("KEYS", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_::ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoNav);
	{

		auto d = ImGui::GetWindowDrawList();
		p = ImGui::GetWindowPos();
		s = ImGui::GetWindowSize();
		ImGui::PushFont(c_menu::get().font);
		ImGui::SetWindowSize(ImVec2(200, 21 + 18 * (m_engine()->IsConnected() ? pressed_binds : hooks::menu_open ? 1 : 0)));
		PostProcessing::performFullscreenBlur(d, main_alpha);
		d->AddRectFilled(p, p + ImVec2(200, 21 + 18 * pressed_binds), ImColor(39, 39, 39, int(50 * main_alpha)));
		auto main_colf = ImColor(39, 39, 39, int(240 * main_alpha));
		auto main_coll = ImColor(39, 39, 39, int(0 * main_alpha));
		d->AddRectFilledMultiColor(p, p + ImVec2(100, 20), main_coll, main_colf, main_colf, main_coll);
		d->AddRectFilledMultiColor(p + ImVec2(100, 0), p + ImVec2(200, 20), main_colf, main_coll, main_coll, main_colf);
		PostProcessing::performFullscreenBlur(d, 1.f);
		auto main_colf2 = ImColor(39, 39, 39, int(140 * min(main_alpha * 2, 1.f)));
		d->AddRectFilledMultiColor(p, p + ImVec2(100, 20), main_coll, main_colf2, main_colf2, main_coll);
		d->AddRectFilledMultiColor(p + ImVec2(100, 0), p + ImVec2(200, 20), main_colf2, main_coll, main_coll, main_colf2);
		auto line_colf = ImColor(vars.misc.keybinds_color.r(), vars.misc.keybinds_color.g(), vars.misc.keybinds_color.b(), int(200 * min(main_alpha * 2, 1.f)));
		auto line_coll = ImColor(vars.misc.keybinds_color.r(), vars.misc.keybinds_color.g(), vars.misc.keybinds_color.b(), int(255 * min(main_alpha * 2, 1.f)));
		d->AddRectFilledMultiColor(p, p + ImVec2(100, 2), line_coll, line_colf, line_colf, line_coll);
		d->AddRectFilledMultiColor(p + ImVec2(100, 0), p + ImVec2(200, 2), line_colf, line_coll, line_coll, line_colf);
		d->AddText(p + ImVec2((200) / 2 - ImGui::CalcTextSize("keybinds").x / 2, (20) / 2 - ImGui::CalcTextSize("keybinds").y / 2), ImColor(250, 250, 250, int(230 * min(main_alpha * 3, 1.f))), "keybinds");

		ImGui::SetCursorPosY(20 + 2);
		ImGui::BeginGroup();//keys 
		add_key("Menu", hooks::menu_open, gui, 200, keys_alpha[12], true);
		if (m_engine()->IsConnected()) {
			add_key("Minimum damage", key_binds::get().get_key_bind_state(_MIN_DAMAGE), vars.ragebot.damage_override_key, 200, min(main_alpha * 2, 1.f), rage, true);
			add_key("Legitbot key", key_binds::get().get_key_bind_state(_LEGITBOT), vars.legitbot.key, 200, min(main_alpha * 2, 1.f), legit);
			add_key("Trigger-bot", key_binds::get().get_key_bind_state(_AUTOFIRE), vars.legitbot.autofire_key, 200, min(main_alpha * 2, 1.f), legit);
			add_key("Double-tap", vars.ragebot.double_tap && misc::get().double_tap_key || misc::get().double_tap_checkc || key_binds::get().get_key_bind_state(_DOUBLETAP), vars.ragebot.double_tap_key, 200, min(main_alpha * 2, 1.f), rage, false);
			add_key("Hide-shots", vars.antiaim.hide_shots && vars.antiaim.hide_shots_key.key > KEY_NONE && vars.antiaim.hide_shots_key.key < KEY_MAX&& misc::get().hide_shots_key, vars.antiaim.hide_shots_key, 200, min(main_alpha * 2, 1.f), rage&& aa, false);
			add_key("Force body-aim", key_binds::get().get_key_bind_state(_BODY_AIM), vars.ragebot.body_aim_key, 200, min(main_alpha * 2, 1.f), rage);
			add_key("Force safe-point", key_binds::get().get_key_bind_state(_SAFEPOINT), vars.ragebot.safe_point_key, 200, min(main_alpha * 2, 1.f), rage);
			add_key("Third person", key_binds::get().get_key_bind_state(_THIRDPERSON), vars.misc.thirdperson_toggle, 200, min(main_alpha * 2, 1.f), vis);
			add_key("Inverter", key_binds::get().get_key_bind_state(_DESYNC_FLIP), vars.antiaim.flip_desync, 200, min(main_alpha * 2, 1.f), aa);
			add_key("Fake-duck", key_binds::get().get_key_bind_state(_FAKEDUCK), vars.misc.fakeduck_key, 200, min(main_alpha * 2, 1.f), rage && aa);
			add_key("Automatic peek", key_binds::get().get_key_bind_state(_AUTO_PEEK), vars.misc.automatic_peek, 200, min(main_alpha * 2, 1.f), true);
		}
		ImGui::EndGroup();
		ImGui::PopFont();
	}
	ImGui::End();
	ImGui::PopStyleVar(2);
}

#include "visuals/logger.hpp"
#include "visuals/hitmarker.h"
#include "../steam/steam_api.h"
void copy_convert(const uint8_t* rgba, uint8_t* out, const size_t size)
{
	auto in = reinterpret_cast<const uint32_t*>(rgba);
	auto buf = reinterpret_cast<uint32_t*>(out);
	for (auto i = 0u; i < (size / 4); ++i)
	{
		const auto pixel = *in++;
		*buf++ = (pixel & 0xFF00FF00) | ((pixel & 0xFF0000) >> 16) | ((pixel & 0xFF) << 16);
	}
}

LPDIRECT3DTEXTURE9 c_menu::steam_image(CSteamID SteamId)
{
	LPDIRECT3DTEXTURE9 asdgsdgadsg;

	int iImage = SteamFriends->GetSmallFriendAvatar(SteamId);

	if (iImage == -1)
		return nullptr;

	uint32 uAvatarWidth, uAvatarHeight;

	if (!SteamUtils->GetImageSize(iImage, &uAvatarWidth, &uAvatarHeight))
		return nullptr;

	const int uImageSizeInBytes = uAvatarWidth * uAvatarHeight * 4;
	uint8* pAvatarRGBA = new uint8[uImageSizeInBytes];

	if (!SteamUtils->GetImageRGBA(iImage, pAvatarRGBA, uImageSizeInBytes))
	{
		delete[] pAvatarRGBA;
		return nullptr;
	}

	auto res = c_menu::get().device->CreateTexture(uAvatarWidth,
		uAvatarHeight,
		1,
		D3DUSAGE_DYNAMIC,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&asdgsdgadsg,
		nullptr);

	std::vector<uint8_t> texData;
	texData.resize(uAvatarWidth * uAvatarHeight * 4u);

	copy_convert(pAvatarRGBA,
		texData.data(),
		uAvatarWidth * uAvatarHeight * 4u);

	D3DLOCKED_RECT rect;
	if (!asdgsdgadsg)
		return false;  //пофиксил краш ебать

	res = asdgsdgadsg->LockRect(0, &rect, nullptr, D3DLOCK_DISCARD);
	auto src = texData.data();
	auto dst = reinterpret_cast<uint8_t*>(rect.pBits);

	for (auto y = 0u; y < uAvatarHeight; ++y)
	{
		std::copy(src, src + (uAvatarWidth * 4), dst);

		src += uAvatarWidth * 4;
		dst += rect.Pitch;
	}
	res = asdgsdgadsg->UnlockRect(0);
	delete[] pAvatarRGBA;

	return asdgsdgadsg;
}

void c_menu::spectators()
{
	
	LPDIRECT3DTEXTURE9 photo[32];
	int specs = 0;
	int id[32];
	int modes = 0;
	std::string spect = "";
	std::string mode = "";
	if (m_engine()->IsInGame() && m_engine()->IsConnected())
	{

		int localIndex = m_engine()->GetLocalPlayer();
		player_t* pLocalEntity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(localIndex));
		if (pLocalEntity)
		{
			for (int i = 0; i < m_engine()->GetMaxClients(); i++)
			{
				player_t* pBaseEntity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));
				if (!pBaseEntity)
					continue;
				if (pBaseEntity->m_iHealth() > 0)
					continue;
				if (pBaseEntity == pLocalEntity)
					continue;
				if (pBaseEntity->IsDormant())
					continue;
				if (pBaseEntity->m_hObserverTarget() != pLocalEntity)
					continue;

				player_info_t pInfo;
				m_engine()->GetPlayerInfo(pBaseEntity->EntIndex(), &pInfo);
				if (pInfo.ishltv)
					continue;
				spect += pInfo.szName;
				spect += "\n";
				specs++;
				id[i] = pInfo.steamID64;
				photo[i] = steam_image(CSteamID((uint64)pInfo.steamID64));
				switch (pBaseEntity->m_iObserverMode())
				{
				case OBS_MODE_IN_EYE:
					mode += "Perspective";
					break;
				case OBS_MODE_CHASE:
					mode += "3rd Person";
					break;
				case OBS_MODE_ROAMING:
					mode += "No Clip";
					break;
				case OBS_MODE_DEATHCAM:
					mode += "Deathcam";
					break;
				case OBS_MODE_FREEZECAM:
					mode += "Freezecam";
					break;
				case OBS_MODE_FIXED:
					mode += "Fixed";
					break;
				default:
					break;
				}

				mode += "\n";
				modes++;
				
			}
		}
	}

	static float main_alpha = 0.f;
	if (!vars.misc.spectators)
		return;

	if (hooks::menu_open || specs > 0) {
		if (main_alpha < 1)
			main_alpha += 5 * ImGui::GetIO().DeltaTime;
	}
	else
		if (main_alpha > 0)
			main_alpha -= 5 * ImGui::GetIO().DeltaTime;

	if (main_alpha < 0.05f)
		return;

	ImGuiStyle* Style = &ImGui::GetStyle();
	ImDrawList* draw;
	Style->WindowRounding = 0;
	Style->WindowBorderSize = 1;
	Style->WindowMinSize = { 1,1 };
	Style->Colors[ImGuiCol_WindowBg] = ImColor(33, 33, 33, 215);//zochem? ia zhe ubral bg?
	Style->Colors[ImGuiCol_ChildBg] = ImColor(21, 20, 21, 255);
	Style->Colors[ImGuiCol_ResizeGrip] = ImColor(42, 40, 43, 0);
	Style->Colors[ImGuiCol_ResizeGripHovered] = ImColor(42, 40, 43, 0);
	Style->Colors[ImGuiCol_ResizeGripActive] = ImColor(42, 40, 43, 0);
	Style->Colors[ImGuiCol_Border] = ImColor(38, 39, 55, 215);
	Style->Colors[ImGuiCol_Button] = ImColor(29, 125, 229, 5);
	Style->Colors[ImGuiCol_ButtonHovered] = ImColor(29, 125, 229, 5);
	Style->Colors[ImGuiCol_ButtonActive] = ImColor(29, 125, 229, 5);
	Style->Colors[ImGuiCol_PopupBg] = ImColor(18, 17, 18, 255);
	Style->FramePadding = ImVec2(1, 1);

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, main_alpha);
	ImVec2 p, s;
	//ImGui::SetNextWindowSize(ImVec2(200, 20 + 15 * 15));

//	if (!vars.misc.spectators)
//		return;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 3));
	ImGui::Begin("SPECTATORS", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_::ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoNav);
	{


		auto d = ImGui::GetWindowDrawList();
		int think_size = 200;
		int calced_size = think_size - 5;
		
		p = ImGui::GetWindowPos();
		s = ImGui::GetWindowSize();
		ImGui::SetWindowSize(ImVec2(200, 21 + 20 * specs));
		ImGui::PushFont(c_menu::get().font);
		PostProcessing::performFullscreenBlur(d, main_alpha);
		d->AddRectFilled(p, p + ImVec2(200, 21 + 20 * specs), ImColor(39, 39, 39, int(50 * main_alpha)));
		auto main_colf = ImColor(39, 39, 39, int(240 * main_alpha));
		auto main_coll = ImColor(39, 39, 39, int(0 * main_alpha));
		d->AddRectFilledMultiColor(p, p + ImVec2(100, 20), main_coll, main_colf, main_colf, main_coll);
		d->AddRectFilledMultiColor(p + ImVec2(100, 0), p + ImVec2(200, 20), main_colf, main_coll, main_coll, main_colf);
		PostProcessing::performFullscreenBlur(d, 1.f);
		auto main_colf2 = ImColor(39, 39, 39, int(140 * min(main_alpha * 2, 1.f)));
		d->AddRectFilledMultiColor(p, p + ImVec2(100, 20), main_coll, main_colf2, main_colf2, main_coll);
		d->AddRectFilledMultiColor(p + ImVec2(100, 0), p + ImVec2(200, 20), main_colf2, main_coll, main_coll, main_colf2);
		auto line_colf = ImColor(vars.misc.spectator_color.r(), vars.misc.spectator_color.g(), vars.misc.spectator_color.b(), int(200 * min(main_alpha * 2, 1.f)));
		auto line_coll = ImColor(vars.misc.spectator_color.r(), vars.misc.spectator_color.g(), vars.misc.spectator_color.b(), int(255 * min(main_alpha * 2, 1.f)));
		d->AddRectFilledMultiColor(p, p + ImVec2(100, 2), line_coll, line_colf, line_colf, line_coll);
		d->AddRectFilledMultiColor(p + ImVec2(100, 0), p + ImVec2(200, 2), line_colf, line_coll, line_coll, line_colf);
		d->AddText(p + ImVec2((200) / 2 - ImGui::CalcTextSize("spectators").x / 2, (20) / 2 - ImGui::CalcTextSize("spectators").y / 2), ImColor(250, 250, 250, int(230 * min(main_alpha * 3, 1.f))), "spectators");
        if (specs > 0) spect += "\n";
		if (modes > 0) mode += "\n";
		

		ImGui::SetCursorPosY(22);
		if (m_engine()->IsInGame() && m_engine()->IsConnected())
		{
			int localIndex = m_engine()->GetLocalPlayer();
			player_t* pLocalEntity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(localIndex));
			if (pLocalEntity)
			{
				for (int i = 0; i < m_engine()->GetMaxClients(); i++)
				{
					player_t* pBaseEntity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));
					if (!pBaseEntity)
						continue;
					if (pBaseEntity == pLocalEntity)
						continue;
					if (pBaseEntity->m_hObserverTarget() != pLocalEntity)
						continue;

					player_info_t pInfo;
					m_engine()->GetPlayerInfo(pBaseEntity->EntIndex(), &pInfo);
					if (pInfo.ishltv)
						continue;
					ImGui::SetCursorPosX(8);
					ImGui::Text(pInfo.szName);
					if (pInfo.fakeplayer)
					{
						ImGui::SameLine(-1, s.x - 22);
						ImGui::Image(getAvatarTexture(pBaseEntity->m_iTeamNum()), ImVec2(15, 15), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.f, 1.f, 1.f, min(main_alpha * 2, 1.f)));
					}
					else
					{
						ImGui::SameLine(-1, s.x - 22);
						ImGui::Image(steam_image(CSteamID((uint64)pInfo.steamID64)), ImVec2(15, 15), ImVec2(0,0), ImVec2(1,1), ImVec4(1.f, 1.f, 1.f, min(main_alpha * 2, 1.f)));
					}

				}
			}
		}

		ImGui::PopFont();
	}
	ImGui::End();
	ImGui::PopStyleVar(2);
}
bool LabelClick2(const char* label, bool* v, const char* unique_id)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	// The concatoff/on thingies were for my weapon config system so if we're going to make that, we still need this aids.
	char Buf[64];
	_snprintf(Buf, 62, crypt_str("%s"), label);

	char getid[128];
	sprintf_s(getid, 128, crypt_str("%s%s"), label, unique_id);


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(getid);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, ImVec2(label_size.y + style.FramePadding.y * 2 + window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2));
	ImGui::ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;

	if (label_size.x > 0)
	{
		ImGui::SameLine(0, style.ItemInnerSpacing.x);
		const ImRect text_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y));

		ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}

	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
	*v = pressed;

	if (pressed || hovered)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(vars.misc.menu_color.r() / 255.f, (vars.misc.menu_color.g()) / 255.f, (vars.misc.menu_color.b()) / 255.f, ImGui::GetStyle().Alpha));
	if (label_size.x > 0.0f)
		ImGui::RenderText(ImVec2(check_bb.GetTL().x + 12, check_bb.GetTL().y), Buf);
	if (pressed || hovered)
		ImGui::PopStyleColor();

	return pressed;

}
bool draw_config_button(const char* label, const char* label_id, bool load, bool save, int curr_config, bool create = false)
{
	bool pressed = false;
	ImGui::SetCursorPosX(8);
	if (ImGui::PlusButton(label, 0, ImVec2(274, 26), label_id, ImColor(vars.misc.menu_color.r() / 255.f, (vars.misc.menu_color.g()) / 255.f, (vars.misc.menu_color.b()) / 255.f), true))vars.selected_config = curr_config;

	static char config_name[36] = "\0";
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupRounding, 4);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, csgo.gui.pop_anim * 0.85f));
	if (ImGui::BeginPopup(label_id, ImGuiWindowFlags_NoMove))
	{

		ImGui::SetNextItemWidth(min(csgo.gui.pop_anim, 0.01f) * ImGui::GetFrameHeight() * 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, csgo.gui.pop_anim);
		auto clicked = false;
		bool one, ones, two = false;
		if (!create) {
			if (LabelClick2(crypt_str("Load"), &one, label_id))
			{
				load_config();
			}
			if (LabelClick2(crypt_str("Save"), &two, label_id))
			{
				save_config();
			}
			if (LabelClick2(crypt_str("Delete"), &ones, label_id))
			{
				remove_config();
			}
		}
		else
		{
			ImGui::SetCursorPosX(8);
			ImGui::Text("Input new config name");
			ImGui::SetCursorPosX(8);
			ImGui::PushItemWidth(254);
			ImGui::InputText(crypt_str("##configname"), config_name, sizeof(config_name));
			ImGui::PopItemWidth();

			ImGui::SetCursorPosX(8);
			auto new_label = std::string("Add " + std::string(config_name) + "");
			if (ImGui::CustomButton(new_label.c_str(), crypt_str("##CONFIG__CREATE"), ImVec2(248, 26 * 1)))
			{
				vars.new_config_name = config_name;
				add_config();
			}
		}
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);    ImGui::PopStyleColor(1);
	return pressed;
}

bool draw_water_button(const char* label, const char* label_id, bool load, bool save, int curr_config, bool create = false)
{
	bool pressed = false;
	if (ImGui::PlusButton(label, 0, ImVec2(10, 10), label_id, ImColor(40 / 255.f, (40) / 255.f, (40) / 255.f), true));

	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupRounding, 4);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, csgo.gui.pop_anim * 0.85f));
	if (ImGui::BeginPopup(label_id, ImGuiWindowFlags_NoMove))
	{

		ImGui::SetNextItemWidth(min(csgo.gui.pop_anim, 0.01f) * ImGui::GetFrameHeight() * 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, csgo.gui.pop_anim);
		ImGui::SetCursorPosX(8); ImGui::Text("Color"); ImGui::SameLine();
		ImGui::ColorEdit(crypt_str("##watermark_color"), &vars.misc.watermark_color, ALPHA);
		draw_multicombo(crypt_str("Watermark additives"), vars.misc.watermarkadditives, watermark_adder, ARRAYSIZE(watermark_adder), c_menu::get().preview);
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);    ImGui::PopStyleColor(1);
	return pressed;
}

const char* GetWeaponName(int id)
{
	switch (id)
	{
	case 17: return crypt_str("AK-47");
	case 23: return crypt_str("AUG");
	case 25: return crypt_str("AWP");
	case 5: return crypt_str("CZ75 Auto");
	case 8: return crypt_str("Desert Eagle");
	case 3: return crypt_str("Dual Berettas");
	case 20: return crypt_str("Famas");
	case 4: return crypt_str("Five-Seven");
	case 26: return crypt_str("G3SG1");
	case 21: return crypt_str("Galil-AR");
	case 0: return crypt_str("Glock-18");
	case 28: return crypt_str("M249");
	case 18: return crypt_str("M4A1 Silencer");
	case 19: return crypt_str("M4A1");
	case 10: return crypt_str("MAC-10");
	case 29: return crypt_str("MAG-7");
	case 11: return crypt_str("MP5-SD");
	case 12: return crypt_str("MP7");
	case 13: return crypt_str("MP9");
	case 30: return crypt_str("Negev");
	case 31: return crypt_str("Nova");
	case 6: return crypt_str("P-2000");
	case 1: return crypt_str("P250");
	case 14: return crypt_str("P90");
	case 15: return crypt_str("PP-Bizon");
	case 9: return crypt_str("Revolver ");
	case 32: return crypt_str("Sawed-Off");
	case 27: return crypt_str("SCAR 20");
	case 24: return crypt_str("SSG 08");
	case 22: return crypt_str("SG 553");
	case 2: return crypt_str("Tec-9");
	case 16: return crypt_str("UMP 45");
	case 7: return crypt_str("USP Silencer");
	case 33: return crypt_str("XM1014");
	case 34: return crypt_str("Knife");
	case 35: return crypt_str("Gloves");
	default: return crypt_str("ERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERRORERROR");
	}
}

#define VERSION crypt_str("saphire")
void c_menu::waterwark()
{
	if (!vars.menu.watermark)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 3));
	ImVec2 p, s;
	
	auto net_channel = m_engine()->GetNetChannelInfo();

	std::string watermark = VERSION;

	if (vars.misc.watermarkadditives[WATERMARK_NAME])
		watermark += crypt_str(" | ") + comp_name();

	if (vars.misc.watermarkadditives[WATERMARK_FPS])
		watermark += crypt_str(" | ") + std::to_string(csgo.globals.framerate) + crypt_str(" fps ");

	if (m_engine()->IsInGame())
	{
		auto nci = m_engine()->GetNetChannelInfo();

		if (nci)
		{
			auto server = nci->GetAddress();

			if (!strcmp(server, crypt_str("loopback")))
				server = crypt_str("local server");
			else if (m_gamerules()->m_bIsValveDS())
				server = crypt_str("valve server");

			auto tickrate = std::to_string((int)(1.0f / m_globals()->m_intervalpertick));

			if (vars.misc.watermarkadditives[WATERMARK_SERVER])
			{
				watermark += crypt_str(" | ");
				watermark += server;
			}

			if (vars.misc.watermarkadditives[WATERMARK_PING])
				watermark += crypt_str(" | ") + std::to_string(csgo.globals.ping) + crypt_str(" ms");
		}

	}
	else
	{
		if (vars.misc.watermarkadditives[WATERMARK_SERVER])
			watermark += crypt_str(" | no connection ");

	}

	if (vars.misc.watermarkadditives[WATERMARK_TIME])
		watermark += crypt_str(" | ") + csgo.globals.time;

	ImGui::PushFont(font);
	auto size_text = ImGui::CalcTextSize(watermark.c_str());
	ImGui::PopFont();
	ImGui::SetNextWindowSize(ImVec2(size_text.x +(hooks::menu_open ? 24 : 14) , 20));
	static bool seted = true;
	if (seted)
		seted = false;
	if (seted)
		ImGui::SetNextWindowPos(ImVec2(200, 200));
	ImGui::Begin("wwwwwwww", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_::ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoNav);
	{

		auto d = ImGui::GetWindowDrawList();
		p = ImGui::GetWindowPos();
		s = ImGui::GetWindowSize();
		ImGui::PushFont(c_menu::get().font);
		ImGui::SetWindowSize(ImVec2(s.x, 21 + 18));
		PostProcessing::performFullscreenBlur(d, 1);
		d->AddRectFilled(p, p + ImVec2(s.x, 21), ImColor(39, 39, 39, int(50 * 1)));
		auto main_colf = ImColor(39, 39, 39, int(240 * 1));
		auto main_coll = ImColor(39, 39, 39, int(0 * 1));
		d->AddRectFilledMultiColor(p, p + ImVec2(s.x / 2, 20), main_coll, main_colf, main_colf, main_coll);
		d->AddRectFilledMultiColor(p + ImVec2(s.x / 2, 0), p + ImVec2(s.x, 20), main_colf, main_coll, main_coll, main_colf);
		PostProcessing::performFullscreenBlur(d, 1.f);
		auto main_colf2 = ImColor(39, 39, 39, int(140 * min(1 * 2, 1.f)));
		d->AddRectFilledMultiColor(p, p + ImVec2(s.x / 2, 20), main_coll, main_colf2, main_colf2, main_coll);
		d->AddRectFilledMultiColor(p + ImVec2(s.x / 2, 0), p + ImVec2(s.x, 20), main_colf2, main_coll, main_coll, main_colf2);
		auto line_colf = ImColor(vars.misc.watermark_color.r(), vars.misc.watermark_color.g(), vars.misc.watermark_color.b(), 255);
		auto line_coll = ImColor(vars.misc.watermark_color.r(), vars.misc.watermark_color.g(), vars.misc.watermark_color.b(), 255);
		d->AddRectFilledMultiColor(p, p + ImVec2(s.x / 2, 2), line_coll, line_colf, line_colf, line_coll);
		d->AddRectFilledMultiColor(p + ImVec2(s.x / 2, 0), p + ImVec2(s.x, 2), line_colf, line_coll, line_coll, line_colf);
		d->AddText(p + ImVec2((hooks::menu_open ? s.x - 10 : s.x) / 2 - size_text.x / 2, (20) / 2 - size_text.y / 2), ImColor(250, 250, 250, int(230 * min(1 * 3, 1.f))), watermark.c_str());
		ImGui::SetCursorPos(ImVec2(s.x - 15, 5));
		if (hooks::menu_open)
		draw_water_button("+", "Swatermark_sett", false, false, NULL, false);
		

	}
	ImGui::End();
	ImGui::PopStyleVar(2);
}

void c_menu::draw(bool is_open)
{
		auto st = ImGui::GetStyle();
		st.Colors[ImGuiCol_ResizeGrip] = ImColor(0, 0, 0, 0);
		st.Colors[ImGuiCol_ResizeGripActive] = ImColor(0, 0, 0, 0);
		st.Colors[ImGuiCol_ResizeGripHovered] = ImColor(0, 0, 0, 0);
		auto s = ImVec2{}, p = ImVec2{}, GuiSize = ImVec2{ 653, 460 };
		std::string uname, build, build_at, sponsor;
		uname = comp_name(); build = "[alpha]"; build_at = __DATE__; build_at += " "; build_at += __TIME__; sponsor = "";
		bool hellllo = GetKeyState(VK_INSERT);
		bool hellllo2 = GetKeyState(VK_HOME);
		static float tab_anim_br = 0.f;
		static int tab_anim = 42;
		bool tab_hovered = false;
		
		if (is_open && public_alpha < 1)
			c_menu::get().public_alpha += 0.55f;
		else if (!is_open && public_alpha > 0)
			c_menu::get().public_alpha -= 0.55f;
		if (public_alpha < 0.01f)
			return;
	//menu
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, public_alpha);
	ImGui::SetNextWindowSize(ImVec2(GuiSize));
	ImGui::Begin("##GUI", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground);
	{
		static int tab = 0;
		static int last_tab = tab;
		static bool active_animation = false;
		static bool preview_reverse = false;
		static float preview_alpha = 0.f;
		static bool drugs = false;
		static float switch_alpha = 1.f;
		static int next_id = -1;
		ImVec2 pad = ImGui::GetStyle().WindowPadding;
		{//draw
			s = ImVec2(ImGui::GetWindowSize().x - pad.x * 2, ImGui::GetWindowSize().y - pad.y * 2); p = ImVec2(ImGui::GetWindowPos().x + pad.x, ImGui::GetWindowPos().y + pad.y); auto draw = ImGui::GetWindowDrawList();
			draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + s.x, p.y + s.y), ImColor(33, 33, 33, int(public_alpha * 255)));
			ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + s.x, p.y + s.y), ImColor(33, 33, 33, int(((preview_alpha) * 1.2f) * 255)));
			if (tab_anim_br > 0) {
				ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + s.x, p.y + s.y), ImColor(0, 0, 0, int(public_alpha * 140 * tab_anim_br)));
				//PostProcessing::performFullscreenBlur(ImGui::GetForegroundDrawList(), (public_alpha * tab_anim_br));
			}
			ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + tab_anim, p.y + s.y - 20), ImColor(39, 39, 39, int(public_alpha * 255)));
			ImGui::PushFont(font);
			draw->AddText(ImVec2(p.x + 5, p.y + s.y - 20 + (10 - ImGui::CalcTextSize(std::string("saphire " + build + " | build at: " + build_at + " | " + sponsor).c_str()).y / 2)), ImColor(220, 220, 220, int(public_alpha * 255)), std::string("saphire " + build + " | build at: " + build_at + " | " + sponsor).c_str());
			draw->AddText(ImVec2(p.x + s.x - 5 - ImGui::CalcTextSize(std::string("Welcome back, " + uname).c_str()).x, p.y + s.y - 20 + (10 - ImGui::CalcTextSize(std::string("Welcome back, " + uname).c_str()).y / 2)), ImColor(220, 220, 220, int(public_alpha * 255)), std::string("Welcome back, " + uname).c_str());
			ImGui::PopFont();
			{
				static float ss = 0.f;
				if (tab_anim > 220 && ss < 1.f)
					ss += 0.02f;
				else if (tab_anim < 220 && ss > 0.f)
					ss -= 0.04f;
				ImGui::PushFont(name);
				ImGui::GetForegroundDrawList()->AddText(ImVec2(p.x + tab_anim / 2 - ImGui::CalcTextSize("saphire").x / 2, p.y + 20 - ImGui::CalcTextSize("saphire").y / 2), ImColor(vars.misc.menu_color.r() / 255.f, vars.misc.menu_color.g() / 255.f, vars.misc.menu_color.b() / 255.f, ImClamp(ss * 2.f, 0.f, 1.f)), "saphire");
				ImGui::PopFont();
			}
		}
		
		ImGui::SetCursorPos(pad);
		ImGui::BeginGroup(/*MAIN SPACE START*/);
		{
			{//tabs
				{
					ImGui::PushFont(icon_font);
					ImGui::SetCursorPosY(50);
					if (ImGui::tab("R", "Ragebot", "Ragebot, Weapons settings", tab == 0, tab_anim) && last_tab == tab && !preview_reverse) tab = 0; if (ImGui::IsItemHovered())tab_hovered = true;
					ImGui::SetCursorPosY(100);
					if (ImGui::tab("L", "Legitbot", "Legitbot, Triggerbot", tab == 1, tab_anim) && last_tab == tab && !preview_reverse) tab = 1; if (ImGui::IsItemHovered())tab_hovered = true;
					ImGui::SetCursorPosY(150);
					if (ImGui::tab("A", "Anti-aim", "Antiaim, Yaw, Pitch, Desync", tab == 2, tab_anim) && last_tab == tab && !preview_reverse) tab = 2; if (ImGui::IsItemHovered())tab_hovered = true;
					ImGui::SetCursorPosY(200);
					if (ImGui::tab("W", "Visuals", "Chams, ESP, World", tab == 3, tab_anim) && last_tab == tab && !preview_reverse) tab = 3; if (ImGui::IsItemHovered())tab_hovered = true;
					ImGui::SetCursorPosY(245);
					if (ImGui::tab("I", "Skins", "Skincahnger", tab == 4, tab_anim) && last_tab == tab && !preview_reverse) tab = 4; if (ImGui::IsItemHovered())tab_hovered = true;
					ImGui::SetCursorPosY(295);
					if (ImGui::tab("M", "Misc", "Movement, Gameplay", tab == 5, tab_anim) && last_tab == tab && !preview_reverse) tab = 5; if (ImGui::IsItemHovered())tab_hovered = true;
					ImGui::SetCursorPosY(345);
					if (ImGui::tab("C", "Configs", "Save, Load, Share configs", tab == 6, tab_anim) && last_tab == tab && !preview_reverse) tab = 6; if (ImGui::IsItemHovered())tab_hovered = true;
					//ImGui::SetCursorPosY(390);
					//if (ImGui::tab("S", "Lua", "Cheat improvements by user", tab == 7, tab_anim) && last_tab == tab && !preview_reverse) tab = 7; if (ImGui::IsItemHovered())tab_hovered = true;
					ImGui::PopFont();
				}
				if (!vars.menu.block_BAR) {
					if (tab_hovered && tab_anim < 220 /** ImGui::GetIO().gui_scale*/)
						tab_anim += 8;
					else if (!tab_hovered && tab_anim > 42 /** ImGui::GetIO().gui_scale*/)
						tab_anim -= 8;
					if (tab_hovered && tab_anim_br < 1.f)
						tab_anim_br += 0.05f;
					else if (!tab_hovered && tab_anim_br > 0.f)
						tab_anim_br -= 0.05f;
				}
				else
				{
					tab_anim_br = 0.f;
					tab_anim = 42;
				}
			}
			ImGui::PushFont(c_menu::get().font);
			{
				if (last_tab != tab || (last_tab == tab && preview_reverse))
				{
					if (!preview_reverse)
					{
						if (preview_alpha == 1.f)
							preview_reverse = true;

						preview_alpha = ImClamp(preview_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
					}
					else
					{
						last_tab = tab;
						if (preview_alpha == 0.01f)
						{
							preview_reverse = false;
						}

						preview_alpha = ImClamp(preview_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
					}
				}
				else
					preview_alpha = ImClamp(preview_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.0f, 1.f);
				static int stab = 0;
				static float stab_anim = 0.f;
				static int stab2 = 0;
				static float stab_anim2 = 0.f;
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f - preview_alpha);
				static bool is_sure_check = false;
				static float started_think = 0;
				static std::string selected_name = "";
				switch (last_tab) {
				case 0: //ragubot
				{
					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y));
					ImGui::BeginGroup(/*SUBTABs SPACE START*/);
					{
						ImGui::PushFont(c_menu::get().font);
						if (ImGui::subtab("Main", stab == 0, 285, 1))stab = 0; ImGui::SameLine();
						if (ImGui::subtab("Weapons", stab == 1, 285, 1))stab = 1; ImGui::SameLine();
						ImGui::PopFont();
					}
					ImGui::EndGroup(/*SUBTABs SPACE END*/);

					if (stab && stab_anim < 1.f)
						stab_anim += 0.05f;
					else if (!stab && stab_anim > 0)
						stab_anim -= 0.1f;
					if (stab_anim) {
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, stab_anim);
						ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 22));
						ImGui::BeginGroup(/*SUBTABs SPACE START*/);
						{
							static int subtab = 0;
							ImGui::PushFont(weapon_icons);
							//bomb y
							if (ImGui::subtab("G", hooks::rage_weapon == 0, 58, 1, 1))hooks::rage_weapon = 0; ImGui::SameLine();//pistol 
							if (ImGui::subtab("J", hooks::rage_weapon == 1, 58, 1, 1))hooks::rage_weapon = 1; ImGui::SameLine();//revolv 
							if (ImGui::subtab("A", hooks::rage_weapon == 2, 58, 1, 1))hooks::rage_weapon = 2; ImGui::SameLine();//deagle 
							if (ImGui::subtab("S", hooks::rage_weapon == 3, 58, 1, 1))hooks::rage_weapon = 3; ImGui::SameLine();//rifle  
							if (ImGui::subtab("L", hooks::rage_weapon == 4, 58, 1, 1))hooks::rage_weapon = 4; ImGui::SameLine();//smg    
							if (ImGui::subtab("d", hooks::rage_weapon == 5, 58, 1, 1))hooks::rage_weapon = 5; ImGui::SameLine();//heavy  
							if (ImGui::subtab("a", hooks::rage_weapon == 6, 58, 1, 1))hooks::rage_weapon = 6; ImGui::SameLine();//ssg
							if (ImGui::subtab("Y", hooks::rage_weapon == 7, 57, 1, 1))hooks::rage_weapon = 7; ImGui::SameLine();//auto   
							if (ImGui::subtab("Z", hooks::rage_weapon == 8, 57, 1, 1))hooks::rage_weapon = 8; ImGui::SameLine();//awp    
							ImGui::PopFont();
						}
						ImGui::EndGroup(/*SUBTABs SPACE END*/);
						ImGui::PopStyleVar();
					}
					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 54 + 20 * stab_anim));
					ImGui::BeginGroup(/*CHILDs SPACE START*/);
					{
						if (stab) {//weapons
							ImGui::MenuChild(crypt_str("Main"), ImVec2(290, 400 - 20 * stab_anim));
							{
								draw_multicombo(crypt_str("Hitboxes"), vars.ragebot.weapon[hooks::rage_weapon].hitboxes, hitboxes, ARRAYSIZE(hitboxes), preview);


								ImGui::Checkbox(crypt_str("Static point scale"), &vars.ragebot.weapon[hooks::rage_weapon].static_point_scale);
								if (vars.ragebot.weapon[hooks::rage_weapon].static_point_scale)
								{
									ImGui::SliderFloat(crypt_str("Head scale"), &vars.ragebot.weapon[hooks::rage_weapon].head_scale, 0.0f, 1.0f, vars.ragebot.weapon[hooks::rage_weapon].head_scale ? crypt_str("%.2f") : crypt_str("None"));
									ImGui::SliderFloat(crypt_str("Body scale"), &vars.ragebot.weapon[hooks::rage_weapon].body_scale, 0.0f, 1.0f, vars.ragebot.weapon[hooks::rage_weapon].body_scale ? crypt_str("%.2f") : crypt_str("None"));
								}
								ImGui::Checkbox(crypt_str("Prefer body aim"), &vars.ragebot.weapon[hooks::rage_weapon].prefer_body_aim);
								ImGui::Checkbox(crypt_str("Prefer safe point"), &vars.ragebot.weapon[hooks::rage_weapon].prefer_safe_points);

								ImGui::SetCursorPosX(8); ImGui::Text("Force safe points");
								draw_keybind(crypt_str("Force safe points"), &vars.ragebot.safe_point_key, crypt_str("##HOKEY_FORCE_SAFE_POINTS"));
								ImGui::SetCursorPosX(8); ImGui::Text("Force body aim");
								draw_keybind(crypt_str("Force body aim"), &vars.ragebot.body_aim_key, crypt_str("##HOKEY_FORCE_BODY_AIM"));
							}
							ImGui::EndMenuChild();

							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 54 + 20 * stab_anim));
							ImGui::MenuChild(crypt_str("Accuracy"), ImVec2(290, 400 - 20 * stab_anim));
							{
								ImGui::Checkbox(crypt_str("Auto-stop"), &vars.ragebot.weapon[hooks::rage_weapon].autostop);

								if (vars.ragebot.weapon[hooks::rage_weapon].autostop)
									draw_multicombo(crypt_str("Modifications"), vars.ragebot.weapon[hooks::rage_weapon].autostop_modifiers, autostop_modifiers, ARRAYSIZE(autostop_modifiers), preview);

								ImGui::Checkbox(crypt_str("Hitchance"), &vars.ragebot.weapon[hooks::rage_weapon].hitchance);

								if (vars.ragebot.weapon[hooks::rage_weapon].hitchance)
									ImGui::SliderInt(crypt_str("Hitchance amount"), &vars.ragebot.weapon[hooks::rage_weapon].hitchance_amount, 1, 100);


								ImGui::Checkbox(crypt_str("Accuracy boost"), &vars.ragebot.weapon[hooks::rage_weapon].accuracy_boost);

								if (vars.ragebot.weapon[hooks::rage_weapon].accuracy_boost)
									ImGui::SliderInt(crypt_str("Accuracy boost amount"), &vars.ragebot.weapon[hooks::rage_weapon].accuracy_boost_amount, 1, 100);

								ImGui::SliderInt(crypt_str("Visible damage"), &vars.ragebot.weapon[hooks::rage_weapon].minimum_visible_damage, 1, 120, true);
								if (vars.ragebot.autowall)
									ImGui::SliderInt(crypt_str("Non Visible damage"), &vars.ragebot.weapon[hooks::rage_weapon].minimum_damage, 1, 120, true);
								ImGui::SetCursorPosX(8); ImGui::Text("Damage override");
								draw_keybind(crypt_str("Damage override"), &vars.ragebot.damage_override_key, crypt_str("##HOTKEY__DAMAGE_OVERRIDE"));


								if (vars.ragebot.damage_override_key.key > KEY_NONE && vars.ragebot.damage_override_key.key < KEY_MAX)
									ImGui::SliderInt(crypt_str("Override damage"), &vars.ragebot.weapon[hooks::rage_weapon].minimum_override_damage, 1, 120, true);
							}
							ImGui::EndMenuChild();

						}
						else//main
						{
							ImGui::MenuChild(crypt_str("Main"), ImVec2(290, 400 - 20 * stab_anim));
							{
								ImGui::Checkbox(crypt_str("Enable"), &vars.ragebot.enable);
								ImGui::SliderInt(crypt_str("Field Of View"), &vars.ragebot.field_of_view, 1, 180, false, "%d °");

								if (vars.ragebot.enable)
									vars.legitbot.enabled = false;

								ImGui::Checkbox(crypt_str("Silent aim"), &vars.ragebot.silent_aim);

								ImGui::Checkbox(crypt_str("Auto-Wall"), &vars.ragebot.autowall);

								ImGui::Checkbox(crypt_str("Auto-Fire"), &vars.ragebot.autoshoot);

								ImGui::Checkbox(crypt_str("Auto-Scope"), &vars.ragebot.autoscope);
							}
							ImGui::EndMenuChild();

							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 54 + 20 * stab_anim));
							ImGui::MenuChild(crypt_str("Exploit"), ImVec2(290, 198 - 20 * stab_anim));
							{
								ImGui::Checkbox(crypt_str("HS"), &vars.antiaim.hide_shots);
								ImGui::SameLine();
								draw_keybind(crypt_str(""), &vars.antiaim.hide_shots_key, crypt_str("##HOTKEY_HIDESHOTS"));


								ImGui::Checkbox(crypt_str("DT"), &vars.ragebot.double_tap);
								ImGui::SameLine();
								draw_keybind(crypt_str(""), &vars.ragebot.double_tap_key, crypt_str("##HOTKEY_DOUBLETAP"));

								if (vars.ragebot.double_tap) {
									ImGui::Checkbox(crypt_str("Instant"), &vars.ragebot.dt_teleport);
								}
							}
							ImGui::EndMenuChild();

							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 58 + 198 + 20 * stab_anim));
							ImGui::MenuChild(crypt_str("Other"), ImVec2(290, 198 - 20 * stab_anim));
							{
								ImGui::Checkbox(crypt_str("Auto-Zeus"), &vars.ragebot.zeus_bot);

								ImGui::Checkbox(crypt_str("Auto-Knife"), &vars.ragebot.knife_bot);
							}
							ImGui::EndMenuChild();
						}
					}
					ImGui::EndGroup(/*CHILDs SPACE END*/);
				}break;
				case 1://legit
				{


					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 2));
					ImGui::BeginGroup(/*SUBTABs SPACE START*/);
					{
						ImGui::PushFont(weapon_icons);
						//bomb y
						if (ImGui::subtab("G", hooks::legit_weapon == 0, 58, 1, 1))hooks::legit_weapon = 0; ImGui::SameLine();//pistol 
						if (ImGui::subtab("J", hooks::legit_weapon == 1, 58, 1, 1))hooks::legit_weapon = 1; ImGui::SameLine();//revolv 
						if (ImGui::subtab("A", hooks::legit_weapon == 2, 58, 1, 1))hooks::legit_weapon = 2; ImGui::SameLine();//deagle 
						if (ImGui::subtab("S", hooks::legit_weapon == 3, 58, 1, 1))hooks::legit_weapon = 3; ImGui::SameLine();//rifle  
						if (ImGui::subtab("L", hooks::legit_weapon == 4, 58, 1, 1))hooks::legit_weapon = 4; ImGui::SameLine();//smg    
						if (ImGui::subtab("d", hooks::legit_weapon == 5, 58, 1, 1))hooks::legit_weapon = 5; ImGui::SameLine();//heavy  
						if (ImGui::subtab("a", hooks::legit_weapon == 6, 58, 1, 1))hooks::legit_weapon = 6; ImGui::SameLine();//ssg
						if (ImGui::subtab("Y", hooks::legit_weapon == 7, 57, 1, 1))hooks::legit_weapon = 7; ImGui::SameLine();//auto   
						if (ImGui::subtab("Z", hooks::legit_weapon == 8, 57, 1, 1))hooks::legit_weapon = 8; ImGui::SameLine();//awp    
						ImGui::PopFont();
					}
					ImGui::EndGroup(/*SUBTABs SPACE END*/);

					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 54));
					ImGui::BeginGroup(/*CHILDs SPACE START*/);
					{

						ImGui::MenuChild(crypt_str("Main"), ImVec2(290, 55));
						{
							ImGui::Checkbox(crypt_str("Enable"), &vars.legitbot.enabled);
							draw_keybind(crypt_str(""), &vars.legitbot.key, crypt_str("##HOTKEY_LGBT_KEY"));
							if (vars.legitbot.enabled)
								vars.ragebot.enable = false;
						}
						ImGui::EndMenuChild();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), pad.y + 108));
						ImGui::MenuChild(crypt_str("Settings"), ImVec2(290, 234));
						{
							ImGui::Checkbox(crypt_str("Friendly fire"), &vars.legitbot.friendly_fire);
							ImGui::Checkbox(crypt_str("Automatic pistols"), &vars.legitbot.autopistol);

							ImGui::Checkbox(crypt_str("Automatic scope"), &vars.legitbot.autoscope);

							if (vars.legitbot.autoscope)
								ImGui::Checkbox(crypt_str("Unscope after shot"), &vars.legitbot.unscope);

							ImGui::Checkbox(crypt_str("Snipers in zoom only"), &vars.legitbot.sniper_in_zoom_only);

							ImGui::Checkbox(crypt_str("Aim if in air"), &vars.legitbot.do_if_local_in_air);
							ImGui::Checkbox(crypt_str("Aim if flashed"), &vars.legitbot.do_if_local_flashed);
							ImGui::Checkbox(crypt_str("Aim thru smoke"), &vars.legitbot.do_if_enemy_in_smoke);

							draw_keybind(crypt_str("Automatic fire key"), &vars.legitbot.autofire_key, crypt_str("##HOTKEY_AUTOFIRE_LGBT_KEY"));
							ImGui::SliderInt(crypt_str("Automatic fire delay"), &vars.legitbot.autofire_delay, 0, 12, false, (!vars.legitbot.autofire_delay ? crypt_str("None") : (vars.legitbot.autofire_delay == 1 ? crypt_str("%d tick") : crypt_str("%d ticks"))));
						}
						ImGui::EndMenuChild();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), pad.y + 342));
						ImGui::MenuChild(crypt_str("RCS"), ImVec2(290, 112));
						{
							draw_combo(crypt_str("RCS type"), vars.legitbot.weapon[hooks::legit_weapon].rcs_type, RCSType, ARRAYSIZE(RCSType));
							ImGui::SliderFloat(crypt_str("RCS amount"), &vars.legitbot.weapon[hooks::legit_weapon].rcs, 0.f, 100.f, crypt_str("%.0f%%"), 1.f);

							if (vars.legitbot.weapon[hooks::legit_weapon].rcs > 0)
							{
								ImGui::SliderFloat(crypt_str("RCS custom FOV"), &vars.legitbot.weapon[hooks::legit_weapon].custom_rcs_fov, 0.f, 30.f, (!vars.legitbot.weapon[hooks::legit_weapon].custom_rcs_fov ? crypt_str("None") : crypt_str("%.2f"))); //-V550
								ImGui::SliderFloat(crypt_str("RCS Custom smooth"), &vars.legitbot.weapon[hooks::legit_weapon].custom_rcs_smooth, 0.f, 12.f, (!vars.legitbot.weapon[hooks::legit_weapon].custom_rcs_smooth ? crypt_str("None") : crypt_str("%.1f"))); //-V550
							}
						}
						ImGui::EndMenuChild();


						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 54));
						ImGui::MenuChild(crypt_str("Settings  "), ImVec2(290, 262));
						{
							const char* hitbox_legit[3] = { crypt_str("Closest"), crypt_str("Head"), crypt_str("Body") };
							draw_combo(crypt_str("Hitbox"), vars.legitbot.weapon[hooks::legit_weapon].priority, hitbox_legit, ARRAYSIZE(hitbox_legit));
							ImGui::SliderFloat(crypt_str("Maximum FOV amount"), &vars.legitbot.weapon[hooks::legit_weapon].fov, 0.f, 30.f, crypt_str("%.2f"));
							ImGui::Spacing();
							ImGui::SliderFloat(crypt_str("Silent FOV"), &vars.legitbot.weapon[hooks::legit_weapon].silent_fov, 0.f, 30.f, (!vars.legitbot.weapon[hooks::legit_weapon].silent_fov ? crypt_str("None") : crypt_str("%.2f"))); //-V550

							ImGui::Spacing();

							draw_combo(crypt_str("Smooth type"), vars.legitbot.weapon[hooks::legit_weapon].smooth_type, LegitSmooth, ARRAYSIZE(LegitSmooth));
							ImGui::SliderFloat(crypt_str("Smooth amount"), &vars.legitbot.weapon[hooks::legit_weapon].smooth, 1.f, 12.f, crypt_str("%.1f"));

							ImGui::Spacing();

							ImGui::SliderInt(crypt_str("Automatic wall damage"), &vars.legitbot.weapon[hooks::legit_weapon].awall_dmg, 0, 100, false, (!vars.legitbot.weapon[hooks::legit_weapon].awall_dmg ? crypt_str("None") : crypt_str("%d")));
							ImGui::SliderInt(crypt_str("Automatic fire hitchance"), &vars.legitbot.weapon[hooks::legit_weapon].autofire_hitchance, 0, 100, false, (!vars.legitbot.weapon[hooks::legit_weapon].autofire_hitchance ? crypt_str("None") : crypt_str("%d")));
							ImGui::SliderFloat(crypt_str("Target switch delay"), &vars.legitbot.weapon[hooks::legit_weapon].target_switch_delay, 0.f, 1.f);
						}
						ImGui::EndMenuChild();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 315));
						ImGui::MenuChild(crypt_str("Accuracy  "), ImVec2(290, 140));
						{
							ImGui::Checkbox(crypt_str("Automatic stop"), &vars.legitbot.weapon[hooks::legit_weapon].auto_stop);
						}
						ImGui::EndMenuChild();

					}
					ImGui::EndGroup(/*CHILDs SPACE END*/);
				}break;
				case 2://antiaim
				{
					static auto type = 0;

					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 2));
					ImGui::BeginGroup(/*SUBTABs SPACE START*/);
					{
						ImGui::PushFont(font);
						if (ImGui::subtab("Stand", type == 0, 146, 1, 1))type = 0; ImGui::SameLine();// 
						if (ImGui::subtab("Slow walk", type == 1, 146, 1, 1))type = 1; ImGui::SameLine();// 
						if (ImGui::subtab("Move", type == 2, 146, 1, 1))type = 2; ImGui::SameLine();
						if (ImGui::subtab("Air", type == 3, 146, 1, 1))type = 3; ImGui::SameLine();
						ImGui::PopFont();
					}
					ImGui::EndGroup(/*SUBTABs SPACE END*/);
					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 54));
					ImGui::BeginGroup(/*CHILDs SPACE START*/);
					{

						ImGui::MenuChild(crypt_str("Main"), ImVec2(290, 95));
						{
							ImGui::Checkbox(crypt_str("Enable"), &vars.antiaim.enable);
							draw_combo(crypt_str("Anti-aim type"), vars.antiaim.antiaim_type, antiaim_type, ARRAYSIZE(antiaim_type));
						}
						ImGui::EndMenuChild();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), pad.y + 148));
						ImGui::MenuChild(vars.antiaim.antiaim_type ? crypt_str("Not available in Legit Mode  ") : crypt_str("Yaw & Pitch"), ImVec2(290, 184));
						{
							if (!vars.antiaim.antiaim_type) {
								draw_combo(crypt_str("Pitch"), vars.antiaim.type[type].pitch, pitch, ARRAYSIZE(pitch));
								padding(0, 3);
								draw_combo(crypt_str("Yaw"), vars.antiaim.type[type].yaw, yaw, ARRAYSIZE(yaw));
								padding(0, 3);
								draw_combo(crypt_str("Base angle"), vars.antiaim.type[type].base_angle, baseangle, ARRAYSIZE(baseangle));
								padding(0, 3);

								if (vars.antiaim.type[type].yaw)
								{
									ImGui::SliderInt(vars.antiaim.type[type].yaw == 1 ? crypt_str("Jitter range") : crypt_str("Spin range"), &vars.antiaim.type[type].range, 1, 180);
								}
							}
						}
						ImGui::EndMenuChild();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), pad.y + 332));
						ImGui::MenuChild(vars.antiaim.antiaim_type ? crypt_str("Not available in Legit Mode ") : crypt_str("Direcion"), ImVec2(290, 122));
						{
							if (!vars.antiaim.antiaim_type) {
								//ImGui::Checkbox(crypt_str("Manual indicator"), &vars.antiaim.flip_indicator);
								//ImGui::SameLine();
								//ImGui::ColorEdit(crypt_str("##invc"), &vars.antiaim.flip_indicator_color, ALPHA);
								//ImGui::SetCursorPosX(8); ImGui::Text("Manual back");
								//draw_keybind(crypt_str("Manual back"), &vars.antiaim.manual_back, crypt_str("##HOTKEY_INVERT_BACK"));
								//ImGui::SetCursorPosX(8); ImGui::Text("Manual left");
								//draw_keybind(crypt_str("Manual left"), &vars.antiaim.manual_left, crypt_str("##HOTKEY_INVERT_LEFT"));
								//ImGui::SetCursorPosX(8); ImGui::Text("Manual right");
								//draw_keybind(crypt_str("Manual right"), &vars.antiaim.manual_right, crypt_str("##HOTKEY_INVERT_RIGHT"));
								ImGui::Text("In developing");

							}

						}
						ImGui::EndMenuChild();


						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 54));
						ImGui::MenuChild(crypt_str("Desync & Fakelag"), ImVec2(290, 262));
						{
							if (vars.antiaim.antiaim_type)
							{
								padding(0, 3);
								draw_combo(crypt_str("Desync"), vars.antiaim.desync, desync, ARRAYSIZE(desync));

								if (vars.antiaim.desync)
								{
									padding(0, 3);
									draw_combo(crypt_str("LBY type"), vars.antiaim.legit_lby_type, lby_type, ARRAYSIZE(lby_type));
								}

								ImGui::SetCursorPosX(8); ImGui::Text("Desync inverter");
								draw_keybind(crypt_str("Inverter"), &vars.antiaim.flip_desync, crypt_str("##HOTKEY_INVERT_DESYNC"));
							}
							else
							{
								draw_combo(crypt_str("Desync"), vars.antiaim.type[type].desync, desync, ARRAYSIZE(desync));

								if (vars.antiaim.type[type].desync)
								{

									ImGui::SetCursorPosX(8); ImGui::Text("Desync inverter");
									draw_keybind(crypt_str("Inverter"), &vars.antiaim.flip_desync, crypt_str("##HOTKEY_INVERT_DESYNC"));
									if (type == ANTIAIM_STAND)
									{
										padding(0, 3);
										draw_combo(crypt_str("LBY type"), vars.antiaim.lby_type, lby_type, ARRAYSIZE(lby_type));
									}

									if (type != ANTIAIM_STAND || !vars.antiaim.lby_type)
									{
										ImGui::SliderInt(crypt_str("Desync range"), &vars.antiaim.type[type].desync_range, 1, 60);
										ImGui::SliderInt(crypt_str("Inverted desync range"), &vars.antiaim.type[type].inverted_desync_range, 1, 60);
										ImGui::SliderInt(crypt_str("Body lean"), &vars.antiaim.type[type].body_lean, 0, 60);
										ImGui::SliderInt(crypt_str("Inverted body lean"), &vars.antiaim.type[type].inverted_body_lean, 0, 60);
									}

									if (vars.antiaim.type[type].desync == 3)
									{
										ImGui::SliderInt(crypt_str("Switch speed"), &vars.antiaim.type[type].spin_speed, 1, 15);
										ImGui::SliderInt(crypt_str("Desync switch range"), &vars.antiaim.type[type].desync_spin_range, 1, 60);
										ImGui::SliderInt(crypt_str("Inverted desync switch range"), &vars.antiaim.type[type].inverted_desync_spin_range, 1, 60);
									}
								}
							}
							ImGui::Checkbox(crypt_str("Enable fakelag"), &vars.antiaim.fakelag);
							if (vars.antiaim.fakelag)
							{
								draw_combo(crypt_str("Fake-lag type"), vars.antiaim.fakelag_type, fakelags, ARRAYSIZE(fakelags));
								ImGui::SliderInt(crypt_str("Limit"), &vars.antiaim.fakelag_amount, 1, 14);

								draw_multicombo(crypt_str("Fake-lag triggers"), vars.antiaim.fakelag_enablers, lagstrigger, ARRAYSIZE(lagstrigger), preview);

								auto enabled_fakelag_triggers = false;

								for (auto i = 0; i < ARRAYSIZE(lagstrigger); i++)
									if (vars.antiaim.fakelag_enablers[i])
										enabled_fakelag_triggers = true;

								if (enabled_fakelag_triggers)
									ImGui::SliderInt(crypt_str("Triggers limit"), &vars.antiaim.triggers_fakelag_amount, 1, 14);
							}
						}
						ImGui::EndMenuChild();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 315));
						ImGui::MenuChild(crypt_str(vars.antiaim.antiaim_type ? crypt_str("Not available in Legit Mode     ") : "Extra"), ImVec2(290, 140));
						{
							if (!vars.antiaim.antiaim_type)
							{
								ImGui::Checkbox(crypt_str("Fake-Duck"), &vars.misc.noduck);
								if (vars.misc.noduck)
									draw_keybind(crypt_str("Fake-duck"), &vars.misc.fakeduck_key, crypt_str("##FAKEDUCK__HOTKEY"));
								ImGui::SetCursorPosX(8); ImGui::Text("Slow-walk");
								draw_keybind(crypt_str("Slow-walk"), &vars.misc.slowwalk_key, crypt_str("##SLOWWALK__HOTKEY"));

								const char* legs[] =
								{
									"Avoid slide",
									"Slide",
									"Break"
								};
								draw_combo("Leg movement", vars.misc.slidewalk, legs, 3);
							}
						}
						ImGui::EndMenuChild();

					}
					ImGui::EndGroup(/*CHILDs SPACE END*/);
				}break;
				case 3://visuals(bb 3 chasa iz moei jizni)
				{
					static int category = 0;
					static int category_w = 0;
					static int vis_subtab = 0;
					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y));
					ImGui::BeginGroup(/*SUBTABs SPACE START*/);
					{
						ImGui::PushFont(c_menu::get().font);
						if (ImGui::subtab("World", stab2 == 0, 285, 1))stab2 = 0; ImGui::SameLine();
						if (ImGui::subtab("Players", stab2 == 1, 285, 1))stab2 = 1; ImGui::SameLine();
						ImGui::PopFont();
					}
					ImGui::EndGroup(/*SUBTABs SPACE END*/);

					if (stab2 && stab_anim2 < 1.f)
						stab_anim2 += 0.05f;
					else if (!stab2 && stab_anim2 > 0)
						stab_anim2 -= 0.1f;
					if (stab_anim2) {
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, stab_anim2);
						ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 22));
						ImGui::BeginGroup(/*SUBTABs SPACE START*/);
						{
							ImGui::PushFont(font);
							//bomb y
							if (ImGui::subtab("Enemy", category == 0, 168, 1, 1))category = 0; ImGui::SameLine();//pistol 
							if (ImGui::subtab("Team", category == 1, 168, 1, 1))category = 1; ImGui::SameLine();//revolv 
							if (ImGui::subtab("Local", category == 2, 168, 1, 1))category = 2; ImGui::SameLine();//deagle 
							ImGui::PopFont();
						}
						ImGui::EndGroup(/*SUBTABs SPACE END*/);
						ImGui::PopStyleVar();
					}
					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 54 + 20 * stab_anim2));
					ImGui::BeginGroup(/*CHILDs SPACE START*/);
					{
						if (stab2) {//players
							ImGui::MenuChild(crypt_str("ESP"), ImVec2(290, 400 - 20 * stab_anim2));
							{
								ImGui::Checkbox("Enable", &vars.player.enable);
								if (category == ENEMY)
								{
									ImGui::Checkbox(crypt_str("Out Of Fov Arrows"), &vars.player.arrows);
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##arrowscolor"), &vars.player.arrows_color, ALPHA);

									if (vars.player.arrows)
									{
										ImGui::SliderInt(crypt_str("Arrows distance"), &vars.player.distance, 1, 100);
										ImGui::SliderInt(crypt_str("Arrows size"), &vars.player.size, 10, 50);
									}
								}

								ImGui::Checkbox(crypt_str("Bounding box"), &vars.player.type[category].box);
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##boxc123olor"), &vars.player.type[category].box_color, ALPHA);

								ImGui::Checkbox(crypt_str("Filled box"), &vars.player.type[category].box_fill);
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##boxc123123212132131232132112321321321321321312312321323olor"), &vars.player.type[category].box_bg_color, ALPHA);

								ImGui::Checkbox(crypt_str("Name"), &vars.player.type[category].name);
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##namecolor"), &vars.player.type[category].name_color, ALPHA);

								ImGui::Checkbox(crypt_str("Health bar"), &vars.player.type[category].health);
								ImGui::Checkbox(crypt_str("Override health color"), &vars.player.type[category].custom_health_color);
								if (vars.player.type[category].custom_health_color) {
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##healthcolor"), &vars.player.type[category].health_color, ALPHA, false, true);
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##healthcolor2"), &vars.player.type[category].health_color2, ALPHA);
								}

								for (auto i = 0, j = 0; i < ARRAYSIZE(flags); i++)
								{
									if (vars.player.type[category].flags[i])
									{
										if (j)
											preview += crypt_str(", ") + (std::string)flags[i];
										else
											preview = flags[i];

										j++;
									}
								}
								draw_multicombo(crypt_str("Flags"), vars.player.type[category].flags, flags, ARRAYSIZE(flags), preview);
								draw_multicombo(crypt_str("Weapon"), vars.player.type[category].weapon, weaponplayer, ARRAYSIZE(weaponplayer), preview);
								if (vars.player.type[category].weapon[WEAPON_ICON] || vars.player.type[category].weapon[WEAPON_TEXT])
								{
									ImGui::SetCursorPosX(8);
									ImGui::Text("Weapon color", true); ImGui::SameLine(); ImGui::ColorEdit(crypt_str("##weapcolor"), &vars.player.type[category].weapon_color, ALPHA);
								}
								ImGui::Checkbox(crypt_str("Skeleton"), &vars.player.type[category].skeleton);
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##skeletoncolor"), &vars.player.type[category].skeleton_color, ALPHA);

							//	ImGui::Checkbox(crypt_str("Ammo bar"), &vars.player.type[category].ammo);
							//	ImGui::SameLine();
							//	ImGui::ColorEdit(crypt_str("##ammocolor"), &vars.player.type[category].ammobar_color, ALPHA);

								ImGui::Checkbox(crypt_str("Footsteps"), &vars.player.type[category].footsteps);
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##footstepscolor"), &vars.player.type[category].footsteps_color, ALPHA);
								if (vars.player.type[category].footsteps)
								{
									ImGui::SliderInt(crypt_str("Thickness"), &vars.player.type[category].thickness, 1, 10);
									ImGui::SliderInt(crypt_str("Radius"), &vars.player.type[category].radius, 50, 500);
								}

								if (category == ENEMY || category == TEAM)
								{
									ImGui::Checkbox(crypt_str("Snap lines"), &vars.player.type[category].snap_lines);
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##snapcolor"), &vars.player.type[category].snap_lines_color, ALPHA);

									if (category == ENEMY)
									{
										if (vars.ragebot.enable)
										{
											ImGui::Checkbox(crypt_str("Show Aimbot Multipoints"), &vars.player.show_multi_points);
											ImGui::SameLine();
											ImGui::ColorEdit(crypt_str("##showmultipointscolor"), &vars.player.show_multi_points_color, ALPHA);
										}
									}
								}
								ImGui::Checkbox("Glow", &vars.player.type[category].glow);
								ImGui::SameLine(); ImGui::ColorEdit(crypt_str("##&&Color"), &vars.player.type[category].glow_color, ALPHA);
								draw_combo(crypt_str("Glow type"), vars.player.type[category].glow_type, glowtype, ARRAYSIZE(glowtype));
							}
							ImGui::EndMenuChild();

							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 54 + 20 * stab_anim2));
							ImGui::MenuChild(crypt_str("Chams"), ImVec2(290, 400 - 20 * stab_anim2));
							{
								const char* dm_type[] =
								{
									"Contour",
									"Wide contour"
								};
								if (category == LOCAL)
								{
									draw_combo(crypt_str("Category"), vars.player.local_chams_type, local_chams_type, ARRAYSIZE(local_chams_type));
									if (!vars.player.local_chams_type) {
										ImGui::Checkbox("Enable", &vars.player.type[category].chams_visible);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##chamsvisible"), &vars.player.type[category].chams_color, ALPHA);
										if (vars.player.type[category].chams_type == 6 || vars.player.type[category].chams_type == 7) {
											ImGui::SetCursorPosX(8);
											ImGui::Text(crypt_str("Overlay color "));
											ImGui::SameLine();
											ImGui::ColorEdit(crypt_str("##Overlay color"), &vars.player.type[category].animated_material_color, ALPHA);
											ImGui::Checkbox("Wireframe overlay", &vars.player.type[category].animated_wireframe);
										}
										ImGui::Checkbox("Wireframe local", &vars.player.type[category].chams_wireframe);
										draw_combo(crypt_str("Material"), vars.player.type[category].chams_type, chamstype, ARRAYSIZE(chamstype));
										ImGui::Spacing();
										ImGui::Checkbox(crypt_str("Add glow overlay"), &vars.player.type[category].double_material);
										ImGui::SameLine(210);
										ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &vars.player.type[category].double_material_color, ALPHA);
										ImGui::Checkbox("Wireframe", &vars.player.type[category].double_wireframe);
										draw_combo(crypt_str("Type"), vars.player.type[category].double_material_type, dm_type, ARRAYSIZE(dm_type));

										ImGui::Checkbox(crypt_str("Transparency in scope"), &vars.player.transparency_in_scope);
										if (vars.player.transparency_in_scope) {
											ImGui::Checkbox("On grenade", &vars.player.on_grenade);
											ImGui::SliderFloat(crypt_str("Transparency"), &vars.player.transparency_in_scope_amount, 0.0f, 1.0f);
										}
									}
									else if (vars.player.local_chams_type == 1)
									{
										ImGui::Checkbox(crypt_str("Enable"), &vars.player.fake_chams_enable);
										ImGui::SameLine(210);
										ImGui::ColorEdit(crypt_str("##fakechamscolor"), &vars.player.fake_chams_color, ALPHA);
										if (vars.player.fake_chams_type == 6 || vars.player.fake_chams_type == 7) {
											ImGui::SetCursorPosX(8);
											ImGui::Text(crypt_str("Overlay color"));
											ImGui::SameLine();
											ImGui::ColorEdit(crypt_str("##Overlay color"), &vars.player.fake_animated_material_color, ALPHA);
											ImGui::Checkbox("Wireframe overlay", &vars.player.fake_animated_wireframe);
										}
										ImGui::Checkbox("Wireframe fake", &vars.player.fake_wireframe);
										draw_combo(crypt_str("Material##des"), vars.player.fake_chams_type, chamstype, ARRAYSIZE(chamstype));
										ImGui::Spacing();
										ImGui::Checkbox(crypt_str("Layered"), &vars.player.layered);
										ImGui::Checkbox(crypt_str("Interpolated"), &vars.player.visualize_lag);
										ImGui::Checkbox(crypt_str("Add glow overlay"), &vars.player.fake_double_material);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &vars.player.fake_double_material_color, ALPHA);
										ImGui::Checkbox("Wireframe", &vars.player.fake_double_wireframe);
										draw_combo(crypt_str("Type##enem"), vars.player.fake_chams_dm_type, dm_type, ARRAYSIZE(dm_type));

									}
									else if (vars.player.local_chams_type == 2)
									{

										ImGui::Checkbox(crypt_str("Enable"), &vars.esp.arms_chams);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##armscolor"), &vars.esp.arms_chams_color, ALPHA);
										if (vars.player.type[category].chams_type == 6 || vars.player.type[category].chams_type == 7) {
											ImGui::SetCursorPosX(8);
											ImGui::Text(crypt_str("Overlay color ")); ImGui::SameLine();
											ImGui::ColorEdit(crypt_str("##animcolormat"), &vars.player.type[category].animated_material_color, ALPHA);
										}
										draw_combo(crypt_str("Material"), vars.esp.arms_chams_type, chamstype, ARRAYSIZE(chamstype));
										ImGui::Spacing();
										ImGui::Checkbox(crypt_str("Add glow overlay"), &vars.esp.arms_double_material);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##armsdoublematerial"), &vars.esp.arms_double_material_color, ALPHA);

									//	ImGui::Checkbox("Wireframe", &vars.esp.double_wireframe_arms);

										draw_combo(crypt_str("Type##enem"), vars.esp.arms_chams_dm_type, dm_type, ARRAYSIZE(dm_type));


									}
									else if (vars.player.local_chams_type == 3)
									{
										ImGui::Checkbox(crypt_str("Enable"), &vars.esp.weapon_chams);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##weaponchamscolors"), &vars.esp.weapon_chams_color, ALPHA);
										draw_combo(crypt_str("Material"), vars.esp.weapon_chams_type, chamstype, ARRAYSIZE(chamstype));
										ImGui::Spacing();

										ImGui::Checkbox(crypt_str("Add glow overlay"), &vars.esp.weapon_double_material);
										ImGui::SameLine(210);
										ImGui::ColorEdit(crypt_str("##weapondoublematerial"), &vars.esp.weapon_double_material_color, ALPHA);

									//	ImGui::Checkbox("Wireframe", &vars.esp.double_wireframe_weapon);

										draw_combo(crypt_str("Type##enem"), vars.esp.weapon_chams_dm_type, dm_type, ARRAYSIZE(dm_type));

									}
								}
								else if (category == ENEMY) {
									const char* enemy_chams_type[] =
									{
										"Visible",
										"Hidden",
										"Backtrack",
										"Ragdoll",
										"Shot",
									};
									draw_combo(crypt_str("Category"), vars.player.enemy_chams_type, enemy_chams_type, ARRAYSIZE(enemy_chams_type));
									if (!vars.player.enemy_chams_type)
									{
										ImGui::Checkbox("Enable", &vars.player.type[category].chams_visible);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##chamsvisible"), &vars.player.type[category].chams_color, ALPHA);
										if (vars.player.type[category].chams_type == 6 || vars.player.type[category].chams_type == 7) {
											ImGui::SetCursorPosX(8);
											ImGui::Text(crypt_str("Overlay color ")); ImGui::SameLine();
											ImGui::ColorEdit(crypt_str("##Overlay color"), &vars.player.type[category].animated_material_color, ALPHA);
											ImGui::Checkbox("Wireframe overlay", &vars.player.type[category].animated_wireframe);
										}
										ImGui::Checkbox("Wireframe visible", &vars.player.type[category].chams_wireframe);
										draw_combo(crypt_str("Material"), vars.player.type[category].chams_type, chamstype, ARRAYSIZE(chamstype));
										ImGui::Spacing();
										ImGui::Checkbox(crypt_str("Add glow overlay"), &vars.player.type[category].double_material);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &vars.player.type[category].double_material_color, ALPHA);
										ImGui::Checkbox("Wireframe", &vars.player.type[category].double_wireframe);
										draw_combo(crypt_str("Type##enem"), vars.player.type[category].double_material_type, dm_type, ARRAYSIZE(dm_type));
										ImGui::Spacing();
									}
									else if (vars.player.enemy_chams_type == 1)
									{
										ImGui::Checkbox("Enable", &vars.player.type[category].chams_hidden);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##chamsvisible"), &vars.player.type[category].xqz_color, ALPHA);
										if (vars.player.type[category].xqz_chams_type == 6 || vars.player.type[category].xqz_chams_type == 7) {
											ImGui::SetCursorPosX(8);
											ImGui::Text(crypt_str("Overlay color ")); ImGui::SameLine();
											ImGui::ColorEdit(crypt_str("Overlay color"), &vars.player.type[category].xqz_animated_material_color, ALPHA);
											ImGui::Checkbox("Wireframe overlay", &vars.player.type[category].xqz_animated_wireframe);
										}
										ImGui::Checkbox("Wireframe hidden", &vars.player.type[category].xqz_chams_wireframe);
										draw_combo(crypt_str("Material"), vars.player.type[category].xqz_chams_type, chamstype, ARRAYSIZE(chamstype));
										ImGui::Spacing();
										ImGui::Checkbox(crypt_str("Add glow overlay"), &vars.player.type[category].xqz_double_material);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &vars.player.type[category].xqz_double_material_color, ALPHA);
										ImGui::Checkbox("Wireframe", &vars.player.type[category].xqz_double_wireframe);
										draw_combo(crypt_str("Type##enem"), vars.player.type[category].double_material_type, dm_type, ARRAYSIZE(dm_type));
										ImGui::Spacing();
									}
									else if (vars.player.enemy_chams_type == 2)
									{
										ImGui::Checkbox("Enable", &vars.player.backtrack_chams);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##chamsvisible"), &vars.player.backtrack_chams_color, ALPHA);
										ImGui::Checkbox("Wireframe backtrack", &vars.player.backtrack_wireframe);
										draw_combo(crypt_str("Material"), vars.player.backtrack_chams_material, chamstype, ARRAYSIZE(chamstype));

									}
									else if (vars.player.enemy_chams_type == 3)
									{
										ImGui::Checkbox(crypt_str("Enable"), &vars.player.type[category].ragdoll_chams);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##hitcolor"), &vars.player.type[category].ragdoll_chams_color, ALPHA);
										draw_combo(crypt_str("Material"), vars.player.type[category].ragdoll_chams_material, chamstype, 9);
									}
									else if (vars.player.enemy_chams_type == 4)
									{
										ImGui::Checkbox(crypt_str("Enable"), &vars.player.lag_hitbox);
										ImGui::SameLine(210);
										ImGui::ColorEdit(crypt_str("##hitcolor"), &vars.player.lag_hitbox_color, ALPHA);
										draw_combo(crypt_str("Material"), vars.player.lag_material, chamstype, 9);
									}
								}
								else if (category == TEAM)
								{
									draw_combo(crypt_str("Category"), vars.player.team_chams_type, chamsvisact, ARRAYSIZE(chamsvisact));
									if (!vars.player.team_chams_type)
									{
										ImGui::Checkbox("Enable", &vars.player.type[category].chams_visible);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##chamsvisible"), &vars.player.type[category].chams_color, ALPHA);
										if (vars.player.type[category].chams_type == 6 || vars.player.type[category].chams_type == 7) {
											ImGui::SetCursorPosX(8);
											ImGui::Text(crypt_str("Overlay color "));
											ImGui::SameLine(); ImGui::ColorEdit(crypt_str("Overlay color"), &vars.player.type[category].animated_material_color, ALPHA);
											ImGui::Checkbox("Wireframe overlay", &vars.player.type[category].animated_wireframe);
										}
										ImGui::Checkbox("Wireframe visible", &vars.player.type[category].chams_wireframe);
										draw_combo(crypt_str("Material##enem"), vars.player.type[category].chams_type, chamstype, ARRAYSIZE(chamstype));
										ImGui::Spacing();
										ImGui::Checkbox(crypt_str("Add glow overlay"), &vars.player.type[category].double_material);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &vars.player.type[category].double_material_color, ALPHA);
										ImGui::Checkbox("Wireframe", &vars.player.type[category].double_wireframe);
										draw_combo(crypt_str("Type##enem"), vars.player.type[category].double_material_type, dm_type, ARRAYSIZE(dm_type));

									}
									else if (vars.player.team_chams_type == 1)
									{
										ImGui::Checkbox("Enable", &vars.player.type[category].chams_hidden);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##chamsvisible"), &vars.player.type[category].xqz_color, ALPHA);
										if (vars.player.type[category].xqz_chams_type == 6 || vars.player.type[category].xqz_chams_type == 7) {
											ImGui::SetCursorPosX(8);
											ImGui::Text(crypt_str("Overlay color ")); ImGui::SameLine(); ImGui::ColorEdit(crypt_str("##Overlay color"), &vars.player.type[category].xqz_animated_material_color, ALPHA);
											ImGui::Checkbox("Wireframe overlays", &vars.player.type[category].xqz_animated_wireframe);
										}
										ImGui::Checkbox("Wireframe hidden##main", &vars.player.type[category].xqz_chams_wireframe);
										draw_combo(crypt_str("Material"), vars.player.type[category].xqz_chams_type, chamstype, ARRAYSIZE(chamstype));
										ImGui::Spacing();
										ImGui::Checkbox(crypt_str("Add glow overlay"), &vars.player.type[category].xqz_double_material);
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &vars.player.type[category].xqz_double_material_color, ALPHA);
										ImGui::Checkbox("Wireframe", &vars.player.type[category].xqz_double_wireframe);
										draw_combo(crypt_str("Type##enem"), vars.player.type[category].double_material_type, dm_type, ARRAYSIZE(dm_type));

									}
								}
							}
							ImGui::EndMenuChild();

						}
						else//world
						{
							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), pad.y + 54 + 20 * stab_anim2));
							ImGui::MenuChild(crypt_str("View"), ImVec2(290, 148 - 20 * stab_anim2));

							ImGui::Checkbox("Enable", &vars.player.enable);
							ImGui::Spacing();

							{
								draw_multicombo(crypt_str("Removals"), vars.esp.removals, removals, ARRAYSIZE(removals), c_menu::get().preview);
								ImGui::SliderInt(crypt_str("View FOV"), &vars.esp.fov, 0, 89);
								static bool tp = false;
								ImGui::Checkbox("Thirdperson", &tp); draw_keybind(crypt_str("##Thirdperson"), &vars.misc.thirdperson_toggle, crypt_str("##TPKEY__HOTKEY"));
								if (tp || vars.misc.thirdperson_toggle.key > KEY_NONE && vars.misc.thirdperson_toggle.key < KEY_MAX) {
									ImGui::SliderInt(crypt_str("Thirdperson distance"), &vars.misc.thirdperson_distance, 50, 300);
								}
								ImGui::Checkbox(crypt_str("Aspect-ratio"), &vars.misc.aspect_ratio);
								ImGui::SliderFloat("Amount", &vars.misc.aspect_ratio_amount, 0.25, 1.5);
							}
							ImGui::EndMenuChild();

							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), pad.y + 58 + 148 + 20 * stab_anim2));
							ImGui::MenuChild(crypt_str("Modulation"), ImVec2(290, 248 - 20 * stab_anim2));
							{
								ImGui::Checkbox(crypt_str("Night-mode"), &vars.esp.nightmode);
								if (vars.esp.nightmode)
								{
									ImGui::Text(crypt_str("Color "));
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##worldcolor"), &vars.esp.world_color, ALPHA);
								}
								draw_combo(crypt_str("Skybox"), vars.esp.skybox, skybox, ARRAYSIZE(skybox));
								if (vars.esp.skybox == 21)
								{
									static char sky_custom[64] = "\0";

									if (!vars.esp.custom_skybox.empty())
										strcpy_s(sky_custom, sizeof(sky_custom), vars.esp.custom_skybox.c_str());

									ImGui::Text(crypt_str("Enter the sky name"), true);
									ImGui::SetCursorPosX(11);
									if (ImGui::InputText(crypt_str("##customsky"), sky_custom, sizeof(sky_custom)))
										vars.esp.custom_skybox = sky_custom;
								}
								if (vars.esp.skybox > 0) {
									ImGui::Text(crypt_str("Skybox color "));
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##skyboxcolor"), &vars.esp.skybox_color, NOALPHA);
								}
								ImGui::Checkbox(crypt_str("Ambient modulation"), &vars.esp.world_modulation);
								if (vars.esp.world_modulation)
								{
									ImGui::SliderFloat(crypt_str("Bloom"), &vars.esp.bloom, 0.0f, 750.0f);
									ImGui::SliderFloat(crypt_str("Ambient"), &vars.esp.ambient, 0.0f, 1500.0f);
									ImGui::SliderFloat(crypt_str("Brightness"), &vars.esp.exposure, 0.0f, 2000.0f);
								}
								ImGui::Checkbox(crypt_str("Fog modulation"), &vars.esp.fog);
								if (vars.esp.fog)
								{
									ImGui::SliderInt(crypt_str("Distance"), &vars.esp.fog_distance, 0, 2500);
									ImGui::SliderInt(crypt_str("Density"), &vars.esp.fog_density, 0, 100);
									ImGui::Text(crypt_str("Color "));
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##fogcolor"), &vars.esp.fog_color, NOALPHA);
								}
								ImGui::Checkbox(crypt_str("Full bright"), &vars.esp.bright);
							}
							ImGui::EndMenuChild();

							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 54 + 20 * stab_anim2));
							ImGui::MenuChild(crypt_str("Grenades"), ImVec2(290, 198 - 20 * stab_anim2));
							{
								ImGui::Checkbox("Grenade path", &vars.esp.grenade_prediction); ImGui::SameLine(); ImGui::ColorEdit("##g_path_color", &vars.esp.grenade_prediction_color, ALPHA);

								ImGui::Checkbox(crypt_str("Grenade warning"), &vars.esp.grenade_proximity_warning);
								if (vars.esp.grenade_proximity_warning)
								{
									ImGui::SetCursorPosX(8);
									ImGui::Text(crypt_str("Warning color"));
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##grenade_color512342"), &vars.esp.grenade_proximity_warning_progress_color, ALPHA);

									draw_combo(crypt_str("Tracer mode"), vars.esp.grenade_proximity_tracers_mode, tracer_mode, ARRAYSIZE(tracer_mode));

									if (!vars.esp.grenade_proximity_tracers_mode == 0)
									{
									if (vars.esp.grenade_proximity_tracers_mode == 2)
										ImGui::SliderInt(crypt_str("Tracer width"), &vars.esp.proximity_tracers_width, 1, 6);

									    ImGui::SetCursorPosX(8);
										ImGui::Text(crypt_str("Tracer color "));
										
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##dhau8ca9xijda0"), &vars.esp.grenade_proximity_tracers_colors, ALPHA);
									}

									ImGui::Checkbox(crypt_str("Offscreen warning"), &vars.esp.offscreen_proximity);

								}
							

								ImGui::Checkbox("Molotov timer", &vars.esp.molotov_timer);
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##molotovtimer_color"), &vars.esp.molotov_timer_color, ALPHA);
								ImGui::Checkbox("Smoke timer", &vars.esp.smoke_timer);
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##smoketimer_color"), &vars.esp.smoke_timer_color, ALPHA);
							}
							ImGui::EndMenuChild();

							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 58 + 198 + 20 * stab_anim2));
							ImGui::MenuChild(crypt_str("Other"), ImVec2(290, 198 - 20 * stab_anim2));
							{
								ImGui::Checkbox("Damage marker", &vars.esp.damage_marker); ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##headshot"), &vars.esp.damage_marker_headdead, ALPHA, false, true);
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##bofyshot"), &vars.esp.damage_marker_body, ALPHA);
								draw_multicombo(crypt_str("Hit marker"), vars.esp.hitmarker, hitmarkers, ARRAYSIZE(hitmarkers), preview);

								ImGui::Checkbox(crypt_str("Kill effect"), &vars.esp.kill_effect);

								if (vars.esp.kill_effect)
									ImGui::SliderFloat(crypt_str("Duration"), &vars.esp.kill_effect_duration, 0.01f, 3.0f, "%.1f ms");

								ImGui::Checkbox("Penetration crosshair", &vars.esp.penetration_reticle); 

								ImGui::Checkbox(crypt_str("Client bullet impacts"), &vars.esp.client_bullet_impacts);
								ImGui::SameLine((s.x - 140) / 2 + 32 - 22);
								ImGui::ColorEdit(crypt_str("##clientbulletimpacts"), &vars.esp.client_bullet_impacts_color, ALPHA);

								ImGui::Checkbox(crypt_str("Server bullet impacts"), &vars.esp.server_bullet_impacts);
								ImGui::SameLine((s.x - 140) / 2 + 32 - 22);
								ImGui::ColorEdit(crypt_str("##serverbulletimpacts"), &vars.esp.server_bullet_impacts_color, ALPHA);

								ImGui::Checkbox(crypt_str("Local bullet tracers"), &vars.esp.bullet_tracer);
								ImGui::SameLine((s.x - 140) / 2 + 32 - 22);
								ImGui::ColorEdit(crypt_str("##bulltracecolor"), &vars.esp.bullet_tracer_color, ALPHA);

								ImGui::Checkbox(crypt_str("Enemy bullet tracers"), &vars.esp.enemy_bullet_tracer);
								ImGui::SameLine((s.x - 140) / 2 + 32 - 22);
								ImGui::ColorEdit(crypt_str("##enemybulltracecolor"), &vars.esp.enemy_bullet_tracer_color, ALPHA);
								const char* beamtype[] =
								{
									"Beam",
									"Line",

								};
								draw_combo(crypt_str("Tracer type"), vars.esp.bullet_tracer_type, beamtype, ARRAYSIZE(beamtype));
							}
							ImGui::EndMenuChild();
						}
					}
					ImGui::EndGroup(/*CHILDs SPACE END*/);
				}break;
				case 4://skins
				{
					
					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 34));
					ImGui::MenuChild("Changer", ImVec2(295 + 290, 420));
					{
						// we need to count our items in 1 line
						auto same_line_counter = 0;
						ImGui::SetCursorPosX(8);
						// if we didnt choose any weapon
						if (current_profile == -1)
						{
							for (auto i = 0; i < vars.skins.skinChanger.size(); i++)
							{
								// do we need update our preview for some reasons?
								if (!all_skins[i])
								{
									vars.skins.skinChanger.at(i).update();
									all_skins[i] = get_skin_preview(get_wep(i, (i == 34 || i == 35) ? vars.skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), vars.skins.skinChanger.at(i).skin_name, device); //-V810
								}
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(51 / 255.f, 51 / 255.f, 51 / 255.f, 1.f));
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
								ImGui::BeginGroup();
								// we licked on weapon
								if (ImGui::ImageButton(all_skins[i], ImVec2(100, 76)))
								{
									current_profile = i;
								}
								ImGui::Text(GetWeaponName(i));
								ImGui::EndGroup();
								ImGui::PopStyleColor(3);

								if (same_line_counter < 4) {
									ImGui::SameLine();
									same_line_counter++;
								}
								else {
									padding(8, 2);
									same_line_counter = 0;
								}
							}
						}
						else
						{
							// update skin preview bool
							static bool need_update[36];

							// we pressed crypt_str("Save & Close") button
							static bool leave;
							// update if we have nullptr texture or if we push force update
							if (!all_skins[current_profile] || need_update[current_profile])
							{
								all_skins[current_profile] = get_skin_preview(get_wep(current_profile, (current_profile == 34 || current_profile == 35) ? vars.skins.skinChanger.at(current_profile).definition_override_vector_index : -1, current_profile == 34).c_str(), vars.skins.skinChanger.at(current_profile).skin_name, device); //-V810
								need_update[current_profile] = false;
							}

							// get settings for selected weapon
							auto& selected_entry = vars.skins.skinChanger[current_profile];
							selected_entry.itemIdIndex = current_profile;
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(51 / 255.f, 51 / 255.f, 51 / 255.f, 1.f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
							ImGui::BeginGroup();
							ImGui::PushItemWidth(290 );
							// search input later
							static char search_skins[64] = "\0";
							static auto item_index = selected_entry.paint_kit_vector_index;

							if (current_profile == 34)
							{
								ImGui::Text(crypt_str("Knife"));
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * c_menu::get().dpi_scale);
								if (ImGui::Combo(crypt_str("##Knife_combo"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
									{
										*out_text = game_data::knife_names[idx].name;
										return true;
									}, nullptr, IM_ARRAYSIZE(game_data::knife_names)))
									need_update[current_profile] = true; // push force update
							}
							else if (current_profile == 35)
							{
								ImGui::Text(crypt_str("Gloves"));
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * c_menu::get().dpi_scale);
								if (ImGui::Combo(crypt_str("##Glove_combo"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
									{
										*out_text = game_data::glove_names[idx].name;
										return true;
									}, nullptr, IM_ARRAYSIZE(game_data::glove_names)))
								{
									item_index = 0; // set new generated paintkits element to 0;
									need_update[current_profile] = true; // push force update
								}
							}
							else
								selected_entry.definition_override_vector_index = 0;

							if (current_profile != 35)
							{
								ImGui::Text(crypt_str("Search"));
								ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

								if (ImGui::InputText(crypt_str("##search"), search_skins, sizeof(search_skins)))
									item_index = -1;

								ImGui::PopStyleVar();
							}

							auto main_kits = current_profile == 35 ? SkinChanger::gloveKits : SkinChanger::skinKits;
							auto display_index = 0;

							SkinChanger::displayKits = main_kits;

							// we dont need custom gloves
							if (current_profile == 35)
							{
								for (auto i = 0; i < main_kits.size(); i++)
								{
									auto main_name = main_kits.at(i).name;

									for (auto i = 0; i < main_name.size(); i++)
										if (iswalpha((main_name.at(i))))
											main_name.at(i) = towlower(main_name.at(i));

									char search_name[64];

									if (!strcmp(game_data::glove_names[selected_entry.definition_override_vector_index].name, crypt_str("Hydra")))
										strcpy_s(search_name, sizeof(search_name), crypt_str("Bloodhound"));
									else
										strcpy_s(search_name, sizeof(search_name), game_data::glove_names[selected_entry.definition_override_vector_index].name);

									for (auto i = 0; i < sizeof(search_name); i++)
										if (iswalpha(search_name[i]))
											search_name[i] = towlower(search_name[i]);

									if (main_name.find(search_name) != std::string::npos)
									{
										SkinChanger::displayKits.at(display_index) = main_kits.at(i);
										display_index++;
									}
								}

								SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
							}
							else
							{
								if (strcmp(search_skins, crypt_str(""))) //-V526
								{
									for (auto i = 0; i < main_kits.size(); i++)
									{
										auto main_name = main_kits.at(i).name;

										for (auto i = 0; i < main_name.size(); i++)
											if (iswalpha(main_name.at(i)))
												main_name.at(i) = towlower(main_name.at(i));

										char search_name[64];
										strcpy_s(search_name, sizeof(search_name), search_skins);

										for (auto i = 0; i < sizeof(search_name); i++)
											if (iswalpha(search_name[i]))
												search_name[i] = towlower(search_name[i]);

										if (main_name.find(search_name) != std::string::npos)
										{
											SkinChanger::displayKits.at(display_index) = main_kits.at(i);
											display_index++;
										}
									}

									SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
								}
								else
									item_index = selected_entry.paint_kit_vector_index;
							}

							ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
							if (!SkinChanger::displayKits.empty())
							{
								if (ImGui::ListBox(crypt_str("##PAINTKITS"), &item_index, [](void* data, int idx, const char** out_text) //-V107
									{
										while (SkinChanger::displayKits.at(idx).name.find(crypt_str("С‘")) != std::string::npos) //-V807
											SkinChanger::displayKits.at(idx).name.replace(SkinChanger::displayKits.at(idx).name.find(crypt_str("С‘")), 2, crypt_str("Рµ"));

										*out_text = SkinChanger::displayKits.at(idx).name.c_str();
										return true;
									}, nullptr, SkinChanger::displayKits.size(), SkinChanger::displayKits.size() > 9 ? 9 : SkinChanger::displayKits.size()) || !all_skins[current_profile])
								{
									SkinChanger::scheduleHudUpdate();
									need_update[current_profile] = true;

									auto i = 0;

									while (i < main_kits.size())
									{
										if (main_kits.at(i).id == SkinChanger::displayKits.at(item_index).id)
										{
											selected_entry.paint_kit_vector_index = i;
											break;
										}

										i++;
									}

								}
							}
							ImGui::PopStyleVar();
							ImGui::PushItemWidth(290 - ImGui::CalcTextSize("Seed").x - ImGui::GetStyle().ItemInnerSpacing.x);
							if (ImGui::InputInt(crypt_str("Seed"), &selected_entry.seed, 1, 100))
								SkinChanger::scheduleHudUpdate();
							ImGui::PopItemWidth();

							ImGui::PushItemWidth(290 - ImGui::CalcTextSize("StatTrak").x - ImGui::GetStyle().ItemInnerSpacing.x);
							if (ImGui::InputInt(crypt_str("StatTrak"), &selected_entry.stat_trak, 1, 15))
								SkinChanger::scheduleHudUpdate();
							ImGui::PopItemWidth();
							if (current_profile != 1)
							{
								if (!vars.skins.custom_name_tag[current_profile].empty())
									strcpy_s(selected_entry.custom_name, sizeof(selected_entry.custom_name), vars.skins.custom_name_tag[current_profile].c_str());
								ImGui::SetCursorPosX(8);
								ImGui::Text(crypt_str("Name Tag"));
								ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
								ImGui::SetCursorPosX(8);
								if (ImGui::InputText(crypt_str("##nametag"), selected_entry.custom_name, sizeof(selected_entry.custom_name)))
								{
									vars.skins.custom_name_tag[current_profile] = selected_entry.custom_name;
									SkinChanger::scheduleHudUpdate();
								}

								ImGui::PopStyleVar();
							}

							ImGui::PopItemWidth();
							ImGui::EndGroup();

							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 305, 5));

							ImGui::BeginChild("##seett", ImVec2(280, 400));
							ImGui::PushItemWidth(240);
							if (ImGui::SliderFloat(crypt_str("Wear"), &selected_entry.wear, 0.0f, 1.0f))
								drugs = true;
							else if (drugs)
							{
								SkinChanger::scheduleHudUpdate();
								drugs = false;
							}

							ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6 * c_menu::get().dpi_scale);
							ImGui::SetCursorPosX(8);
							ImGui::Text(crypt_str("Quality"));
							ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * c_menu::get().dpi_scale);
							if (ImGui::Combo(crypt_str("##Quality_combo"), &selected_entry.entity_quality_vector_index, [](void* data, int idx, const char** out_text)
								{
									*out_text = game_data::quality_names[idx].name;
									return true;
								}, nullptr, IM_ARRAYSIZE(game_data::quality_names)))
								SkinChanger::scheduleHudUpdate();

								ImGui::PopItemWidth();
								ImGui::Checkbox(crypt_str("Force rare animations"), &vars.skins.rare_animations);
								ImGui::SetCursorPosX(8);
								if (ImGui::ImageButton(all_skins[current_profile], ImVec2(256, 214)))
								{
									// maybe i will do smth later where, who knows :/
								}
								ImGui::SetCursorPosX(8);
								if (ImGui::CustomButton(crypt_str("Close"), crypt_str("##CLOSE__SKING"), ImVec2(264, 20)))
								{
									next_id = -1;
									leave = true;
								}
								//

								ImGui::EndChild();
								ImGui::PopStyleColor(3);
								// update element
								selected_entry.update();

								// we need to reset profile in the end to prevent render images with massive's index == -1
								if (leave)
								{
									ImGui::SetScrollY(0);
									current_profile = next_id;
									leave = false;
								}
						}
					}
					ImGui::EndMenuChild();
				}break;
				case 5://misc
				{
					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 34));
					ImGui::BeginGroup(/*CHILDs SPACE START*/);
					{
						ImGui::MenuChild("Movement", ImVec2(290, 220));
						{
							ImGui::Checkbox(crypt_str("Automatic jump"), &vars.misc.bunnyhop);
							draw_combo(crypt_str("Automatic strafes"), vars.misc.airstrafe, strafes, ARRAYSIZE(strafes));
							ImGui::Checkbox(crypt_str("Crouch in air"), &vars.misc.crouch_in_air);
							ImGui::Checkbox(crypt_str("Fast stop"), &vars.misc.fast_stop);
							ImGui::SetCursorPosX(8); ImGui::Text(crypt_str("Auto peek"));
							draw_keybind(crypt_str("Auto peek"), &vars.misc.automatic_peek, crypt_str("##AUTOPEEK__HOTKEY"), true); ImGui::SameLine();
							ImGui::ColorEdit("##autopeeek_color", &vars.misc.automatic_peek_color, ALPHA);
							ImGui::SetCursorPosX(8); ImGui::Text(crypt_str("Edge jump"));
							draw_keybind(crypt_str("Edge jump"), &vars.misc.edge_jump, crypt_str("##EDGEJUMP__HOTKEY"));
						}
						ImGui::EndMenuChild();
						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), pad.y + 38 + 220));
						ImGui::MenuChild("Logging", ImVec2(290, 196));
						{
							draw_multicombo(crypt_str("Events to log"), vars.misc.events_to_log, events, ARRAYSIZE(events), c_menu::get().preview);
							draw_multicombo(crypt_str("Console output"), vars.misc.log_output, events_output, ARRAYSIZE(events_output), c_menu::get().preview);
							//ImGui::SetCursorPosX(8); ImGui::Text("Color"); ImGui::SameLine(); ImGui::ColorEdit(crypt_str("Color"), &vars.misc.log_color, ALPHA);
							draw_combo("Hit sound", vars.esp.hitsound, sounds, 7);
						}
						ImGui::EndMenuChild();

						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 34));
						ImGui::MenuChild("Gameplay", ImVec2(290, 190));
						{
							ImGui::Checkbox(crypt_str("Preserve killfeed"), &vars.esp.preserve_killfeed);
							ImGui::Checkbox("Unlock hidden cvars", &vars.misc.unlock_hiddencvars);
							ImGui::Checkbox("Block server advertsments", &vars.misc.block_adv);
							ImGui::Checkbox("Unlock inventory access", &vars.misc.inventory_access);
							ImGui::Checkbox("Accept matchmaking", &vars.misc.auto_accept);
							ImGui::Checkbox("Extended backtrack", &vars.misc.extended_backtrack);
							ImGui::Checkbox("Cheat tag", &vars.misc.clantag_spammer);
							ImGui::Checkbox("Ragdoll gravity", &vars.misc.ragdolls);
						}
						ImGui::EndMenuChild();

						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 38 + 190));
						ImGui::MenuChild("Other", ImVec2(290, 226));
						{
							ImGui::Checkbox(crypt_str("Buybot"), &vars.misc.buybot_enable);
							if (vars.misc.buybot_enable)
							{
								draw_combo(crypt_str("Main gun"), vars.misc.buybot1, mainwep, ARRAYSIZE(mainwep));
								draw_combo(crypt_str("Pistol"), vars.misc.buybot2, secwep, ARRAYSIZE(secwep));
								draw_multicombo(crypt_str("Attachments"), vars.misc.buybot3, grenades, ARRAYSIZE(grenades), c_menu::get().preview);
							}

						//	ImGui::Checkbox(crypt_str("Info Bar"), &vars.misc.gaybar);
						}
						ImGui::EndMenuChild();
					}
					ImGui::EndGroup(/*CHILDs SPACE END*/);
				}break;
				case 6://cefege pricupi eblan
				{
					{

						cfg_manager->config_files();
						files = cfg_manager->files;

						for (auto& current : files)
							if (current.size() > 2)
								current.erase(current.size() - 3, 3);

					}
					ImGui::SetCursorPos(ImVec2(pad.x + 48, pad.y + 34));
					ImGui::BeginGroup(/*CHILDs SPACE START*/);
					{
						ImGui::MenuChild("Configs", ImVec2(290, 420));
						{
							ImGui::SetCursorPos(ImVec2(0, 0));
							ImGui::BeginChild("##configs", ImVec2(290, 300), false, ImGuiWindowFlags_NoBackground);
							{
								draw_config_button("Add new config", "adddadasdasdsadsadassaasd", false, false, false, true);
								for (int i = 0; i < files.size(); i++)
								{
									bool load, save = false;
									draw_config_button(files.at(i).c_str(), std::string(files.at(i) + "id_ajdajdadmadasd").c_str(), load, save, i);
								}
							}
							ImGui::EndChild();
						}
						ImGui::EndMenuChild();

						ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 295, pad.y + 34));
						ImGui::MenuChild("Settings", ImVec2(290, 420));
						{
							ImGui::SetCursorPosX(8); ImGui::Text("Menu color");
							ImGui::SameLine();
							ImGui::ColorEdit(crypt_str("##menu_coloredit"), &vars.misc.menu_color, ALPHA);

							ImGui::Checkbox(crypt_str("Watermark"), &vars.menu.watermark);

							ImGui::Spacing();

							ImGui::Checkbox(crypt_str("Keybinds"), &vars.misc.keybinds);
							ImGui::SameLine();
							ImGui::ColorEdit(crypt_str("##keybinds_color"), &vars.misc.keybinds_color, ALPHA);

							ImGui::Checkbox(crypt_str("Spectators"), &vars.misc.spectators);
							ImGui::SameLine();
							ImGui::ColorEdit(crypt_str("##spectator_color"), &vars.misc.spectator_color, ALPHA);
						}
						ImGui::EndMenuChild();
					}
					ImGui::EndGroup(/*CHILDs SPACE END*/);
				}break;
			//	case 7://luvashki po skidke
			//	{
			//		
			//
			//	}break;
				}
				
				ImGui::PopStyleVar();
			}
			ImGui::PopFont();
		}
		ImGui::EndGroup(/*MAIN SPACE END*/);
	}
	ImGui::End();
	ImGui::PopStyleVar();
}