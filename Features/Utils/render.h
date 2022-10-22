#pragma once

#include "../includes.hpp"
#include "singleton.h"
#include "../Others/imguiconnect.h"
#include <mutex>

class renderer : public singleton< renderer >
{
public:
	// Initialize
	void create_objects(IDirect3DDevice9* pDevice);
	void invalidate_objects();
	void scene_add();
	void scene_render();
	void gui_init(IDirect3DDevice9* pDevice);

	// Functions
	float text(ImFont* font, float x, float y, Color color, bool center_x, bool center_y, bool outline, bool shadow, const char* text, ...);
	void line(float x1, float y1, float x2, float y2, Color color, float thickness = 1.f);
	void ring3d(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float progress = 1.f, bool fill_prog = false);
	void sided_arc(float x, float y, float radius, float scale, Color col, float thickness);
	void line_gradient(float x1, float y1, float x2, float y2, Color color1, Color color2, float thickness = 1.f);
	void rect(float x, float y, float w, float h, Color color, float rounding = 0.f);
	void rect_filled(float x, float y, float w, float h, Color color, float rounding = 0.f, ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_None);
	void rect_filled_gradient(float x, float y, float w, float h, Color first, Color second, int type);
	void circle(float x, float y, float radius, int points, Color color, float thickness = 1.f);
	void circle_filled(float x, float y, float radius, int points, Color color);
	void triangle(Vector2D first, Vector2D second, Vector2D third, Color color, float thickness = 1.f);
	void filled_triangle(Vector2D first, Vector2D second, Vector2D third, Color color);
	void image(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a = ImVec2(0, 0), const ImVec2& uv_b = ImVec2(1, 1), ImU32 color = 0xFFFFFFFF);
	void arc(float x, float y, float radius, float min_angle, float max_angle, Color color, float thickness = 1.f);
	void Draw3DRainbowCircle(const Vector& origin, float radius);
	void Draw3DCircle(const Vector& origin, float radius, Color color);
	void Draw3DFilledCircle(const Vector& origin, float radius, Color color);
	void dual_circle(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device);

	bool d3d_init = false;
	Vector2D m_screen_size;


	ImFont* logs = nullptr;
	ImFont* name = nullptr;
	ImFont* esp = nullptr;
	ImFont* world = nullptr;
	ImFont* damage = nullptr;
	ImFont* indicators = nullptr;
	ImFont* grenades = nullptr;
	ImFont* grenades_warning = nullptr;
	ImFont* weapon_icons = nullptr;
	ImFont* screen_indicators = nullptr;

private:
	std::mutex m_mutex;

	// drawlist
	std::shared_ptr<ImDrawList> m_draw_list;
	std::shared_ptr<ImDrawList> m_data_draw_list;
	std::shared_ptr<ImDrawList> m_replace_draw_list;

	// u32
	ImU32 GetU32(Color _color)
	{
		return ((_color[3] & 0xff) << 24) + ((_color[2] & 0xff) << 16) + ((_color[1] & 0xff) << 8)
			+ (_color[0] & 0xff);
	}
};