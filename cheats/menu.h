#pragma once
#include "../includes.hpp"
#include "../steam/steam_api.h"

class c_menu : public singleton<c_menu> {
public:
	void spectators();
	void waterwark();
	int pressed_keys = 0;
	enum state
	{
		hold,
		toggle
	};
	
	std::string get_state(key_bind_mode mode)
	{
		if (mode == hold)
			return "hold";
		else if (mode == toggle)
			return "toggle";

	}
	void add_key(const char* name, bool main, key_bind key, int spacing, float alpha, bool condition = true, bool damage = false)
	{
		float animka = 1.f;
		auto p = ImGui::GetWindowPos() + ImGui::GetCursorPos() * animka;

		if (!condition || animka < 0)
			return;

		
		if (!main || animka < 0)
			return;

		if (key.mode == hold)
			ImGui::GetWindowDrawList()->AddCircle(ImVec2(p.x + 5, p.y + ImGui::CalcTextSize(name).y / 2), 3, ImColor(vars.misc.keybinds_color.r(), vars.misc.keybinds_color.g(), vars.misc.keybinds_color.b(), 255));
		else if (key.mode == toggle)
			ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(p.x + 5, p.y + ImGui::CalcTextSize(name).y / 2), 3, ImColor(vars.misc.keybinds_color.r(), vars.misc.keybinds_color.g(), vars.misc.keybinds_color.b(), 255));

		spacing -= ImGui::CalcTextSize(damage ? std::to_string(vars.ragebot.weapon[csgo.globals.current_weapon].minimum_override_damage).c_str() : get_state(key.mode).c_str()).x;
		ImGui::SetCursorPos(ImVec2(key.mode == -1 ? 5 : 15, ImGui::GetCursorPosY() * animka));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1));
		ImGui::Text(name, false); ImGui::SameLine(spacing - 5);
		ImGui::Text(damage ? std::to_string(vars.ragebot.weapon[csgo.globals.current_weapon].minimum_override_damage).c_str() : get_state(key.mode).c_str(), false);
		ImGui::PopStyleColor(1);
		
			pressed_keys + 1;
		
	}
	LPDIRECT3DTEXTURE9 steam_image(CSteamID SteamId);
	void draw( bool is_open );
	void menu_setup(ImGuiStyle &style);
	void keys();
	float dpi_scale = 1.f;
	ImFont* smallest_pixel;
	ImFont* esp_name;
	ImFont* esp_weapon;
	LPDIRECT3DTEXTURE9 ct_a = nullptr;
	LPDIRECT3DTEXTURE9 tt_a = nullptr;
	ImFont* name;
	ImFont* isis;
	ImFont* icon_font;
	ImFont* weapon_icons;
	ImFont* weapon_icons2;
	ImFont* font;
	ImFont* interfaces;
	ImFont* timersz;
	float public_alpha;
	IDirect3DDevice9* device;
	float color_buffer[4] = { 1.f, 1.f, 1.f, 1.f };
	std::string preview = crypt_str("None");
private:
	struct {
		ImVec2 WindowPadding;
		float  WindowRounding;
		ImVec2 WindowMinSize;
		float  ChildRounding;
		float  PopupRounding;
		ImVec2 FramePadding;
		float  FrameRounding;
		ImVec2 ItemSpacing;
		ImVec2 ItemInnerSpacing;
		ImVec2 TouchExtraPadding;
		float  IndentSpacing;
		float  ColumnsMinSpacing;
		float  ScrollbarSize;
		float  ScrollbarRounding;
		float  GrabMinSize;
		float  GrabRounding;
		float  TabRounding;
		float  TabMinWidthForUnselectedCloseButton;
		ImVec2 DisplayWindowPadding;
		ImVec2 DisplaySafeAreaPadding;
		float  MouseCursorScale;
	} styles;

	bool update_dpi = false;
	bool update_scripts = false;
	void dpi_resize(float scale_factor, ImGuiStyle &style);

	int active_tab_index;
	ImGuiStyle style;
	int width = 850, height = 560;
	float child_height;

	float preview_alpha = 1.f;

	int active_tab;

	int rage_section;
	int legit_section;
	int visuals_section;
	int players_section;
	int misc_section;
	int settings_section;

	// we need to use 'int child' to seperate content in 2 childs
	void draw_ragebot(int child);
	void draw_tabs_ragebot();

	void draw_legit(int child);
	 
	void draw_visuals(int child);
	void draw_tabs_visuals();
	int current_profile = -1;

	void draw_players(int child);
	void draw_tabs_players();

	void draw_misc(int child);
	void draw_tabs_misc();

	void draw_settings(int child);

	void draw_lua(int child);
	void draw_radar(int child);
	void draw_player_list(int child);

	
};
