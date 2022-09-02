#include <png.h>

#include <deque>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Mode.hpp"
#include "PPU466.hpp"
#include <list>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//player position:
	glm::vec2 player_at = glm::vec2(0.0f);

	//projectiles
	typedef struct {
		uint32_t tile_idx;
		glm::vec2 pos;
		uint8_t dir; // {up,right,down,left} -> {0,1,2,3}
	} Projectile;

	std::list<Projectile> projectiles;

	float total_elapsed = 0.0;
	bool has_lost = false;

	//----- drawing handled by PPU466 -----

	PPU466 ppu;

	// reading the PNG file and loading it to the PPU memory

	int width, height;
  png_byte color_type;
  png_byte bit_depth;
  png_bytep *row_pointers;

  void read_png_file(std::string filename);
  void load_png_tu_ppu();

	// computed once

	std::array<uint32_t, 16*16> tile_idx_to_palette_idx;
};
