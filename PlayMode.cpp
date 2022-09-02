#include "PlayMode.hpp"

// for the GL_ERRORS() macro:
#include "gl_errors.hpp"

// for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>
#include <iterator>
#include <random>
#include <set>
#include <unordered_map>
#include "data_path.hpp"

PlayMode::PlayMode() {
  read_png_file(data_path("../assets.png"));
  load_png_tu_ppu();
  for (uint32_t i = 0; i < PPU466::BackgroundWidth * PPU466::BackgroundHeight;
       i++) {
    ppu.background[i] = 255;
  }
  player_at.x = PPU466::ScreenWidth / 2 - 8;
  player_at.y = PPU466::ScreenHeight / 2 - 16;

  // Todo: set background to some star tile
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	constexpr float PlayerSpeed = 150.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	if (up.pressed) player_at.y += PlayerSpeed * elapsed;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---	

	//rocket sprite:
	ppu.sprites[0].x = int8_t(player_at.x);
	ppu.sprites[0].y = int8_t(player_at.y);
	ppu.sprites[0].index = 48;
	ppu.sprites[0].attributes = tile_idx_to_palette_idx[48];

	ppu.sprites[1].x = int8_t(player_at.x) + 8;
	ppu.sprites[1].y = int8_t(player_at.y);
	ppu.sprites[1].index = 49;
	ppu.sprites[1].attributes = tile_idx_to_palette_idx[49];

	ppu.sprites[2].x = int8_t(player_at.x);
	ppu.sprites[2].y = int8_t(player_at.y) + 8;
	ppu.sprites[2].index = 32;
	ppu.sprites[2].attributes = tile_idx_to_palette_idx[32];

	ppu.sprites[3].x = int8_t(player_at.x) + 8;
	ppu.sprites[3].y = int8_t(player_at.y) + 8;
	ppu.sprites[3].index = 33;
	ppu.sprites[3].attributes = tile_idx_to_palette_idx[33];

	ppu.sprites[4].x = int8_t(player_at.x);
	ppu.sprites[4].y = int8_t(player_at.y) + 16;
	ppu.sprites[4].index = 16;
	ppu.sprites[4].attributes = tile_idx_to_palette_idx[16];

	ppu.sprites[5].x = int8_t(player_at.x) + 8;
	ppu.sprites[5].y = int8_t(player_at.y) + 16;
	ppu.sprites[5].index = 17;
	ppu.sprites[5].attributes = tile_idx_to_palette_idx[17];

	ppu.sprites[6].x = int8_t(player_at.x);
	ppu.sprites[6].y = int8_t(player_at.y) + 24;
	ppu.sprites[6].index = 0;
	ppu.sprites[6].attributes = tile_idx_to_palette_idx[0];

	ppu.sprites[7].x = int8_t(player_at.x) + 8;
	ppu.sprites[7].y = int8_t(player_at.y) + 24;
	ppu.sprites[7].index = 1;
	ppu.sprites[7].attributes = tile_idx_to_palette_idx[1];


	//some other misc sprites:
	for (uint32_t i = 8; i < 63; ++i) {
		ppu.sprites[i].y = 240;
	}

	/*
	//some other misc sprites:
	for (uint32_t i = 1; i < 63; ++i) {
		float amt = (i + 2.0f * background_fade) / 62.0f;
		ppu.sprites[i].x = int8_t(0.5f * PPU466::ScreenWidth + std::cos( 2.0f * M_PI * amt * 5.0f + 0.01f * player_at.x) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].y = int8_t(0.5f * PPU466::ScreenHeight + std::sin( 2.0f * M_PI * amt * 3.0f + 0.01f * player_at.y) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].index = 32;
		ppu.sprites[i].attributes = 6;
		if (i % 2) ppu.sprites[i].attributes |= 0x80; //'behind' bit
	}*/

	//--- actually draw ---
	ppu.draw(drawable_size);
}

void PlayMode::load_png_tu_ppu() {
  std::vector<std::vector<glm::u8vec4>> palette_table;
  std::array<PPU466::Tile, 16 * 16> tile_table;

  for (uint32_t i = 0; i < 16; i++) {
    for (uint32_t j = 0; j < 16; j++) {
      std::vector<glm::u8vec4> current_palette;

      for (uint32_t y = i * 8; y < i * 8 + 8; y++) {
        png_bytep row = row_pointers[y];
        for (int x = j * 8; x < j * 8 + 8; x++) {
          png_bytep px = &(row[x * 4]);

          glm::u8vec4 color(px[0], px[1], px[2], px[3]);
          if(std::find(current_palette.begin(), current_palette.end(), color) == current_palette.end()){
						current_palette.push_back(color);
					}
        }
      }

      if (current_palette.size() > 4) {
        throw std::runtime_error("Tile has more than 4 colors");
      }

      auto lt = [](glm::u8vec4 a, glm::u8vec4 b) {
        return (a[0] | (a[1] << 8) | (a[2] << 16) | (a[3] << 24)) < (b[0] |
               (b[1] << 8) | (b[2] << 16) | (b[3] << 24));
      };

      sort(current_palette.begin(), current_palette.end(), lt);

      uint32_t palette_idx = -1;
      for (uint32_t k = 0; k < palette_table.size(); k++) {
        if (std::includes(palette_table[k].begin(), palette_table[k].end(),
                          current_palette.begin(), current_palette.end(), lt)) {
          palette_idx = k;
        } else if (std::includes(current_palette.begin(), current_palette.end(),
                                 palette_table[k].begin(),
                                 palette_table[k].end(), lt)) {
          palette_table[k] = current_palette;
          palette_idx = k;
        }
      }
      if (palette_idx == -1) {
        palette_idx = palette_table.size();
        palette_table.push_back(current_palette);
      }

      if (palette_table.size() > 8) {
        throw std::runtime_error("Need more than 8 palettes");
      }

			tile_idx_to_palette_idx[i * 16 + j] = palette_idx;
    }
  }

  for (uint32_t i = 0; i < 16; i++) {
    for (uint32_t j = 0; j < 16; j++) {
      std::vector<glm::u8vec4> current_palette =
          palette_table[tile_idx_to_palette_idx[i * 16 + j]];
      PPU466::Tile current_tile;
			current_tile.bit0.fill(0);
			current_tile.bit1.fill(0);

      for (uint32_t y = i * 8; y < i * 8 + 8; y++) {
        png_bytep row = row_pointers[y];
        for (uint32_t x = j * 8; x < j * 8 + 8; x++) {
					
          png_bytep px = &(row[x * 4]);

          glm::u8vec4 color(px[0], px[1], px[2], px[3]);

          auto it =
              std::find(current_palette.begin(), current_palette.end(), color);
          assert(it != current_palette.end());
          uint32_t color_idx = it - current_palette.begin();

					current_tile.bit0[7 - y + i * 8] |= (color_idx & 1) << (x - j * 8);
          current_tile.bit1[7 - y + i * 8] |= (color_idx >> 1) << (x - j * 8);
          
        }
      }
      tile_table[i * 16 + j] = current_tile;
    }
  }

  ppu.tile_table = tile_table;
  for (uint32_t i = 0; i < palette_table.size(); i++) {
    PPU466::Palette palette;
    for (uint32_t j = 0; j < palette_table[i].size(); j++) {
      palette[j] = palette_table[i][j];
    }
    ppu.palette_table[i] = palette;
  }

  // Todo: free row_pointers here
}

void PlayMode::read_png_file(std::string filename) {
  // http://www.libpng.org/pub/png/book/chapter13.html
  // https://gist.github.com/jeroen/10eb17a9fb0e5799b772
  FILE *fp = fopen(filename.c_str(), "rb");

  png_structp png =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) abort();

  png_infop info = png_create_info_struct(png);
  if (!info) abort();

  if (setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  png_read_info(png, info);

  width = png_get_image_width(png, info);
  height = png_get_image_height(png, info);
  color_type = png_get_color_type(png, info);
  bit_depth = png_get_bit_depth(png, info);

  // Read any color_type into 8bit depth, RGBA format.
  // See http://www.libpng.org/pub/png/libpng-manual.txt

  if (bit_depth == 16) png_set_strip_16(png);

  if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  for (int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png, info));
  }

  png_read_image(png, row_pointers);

  fclose(fp);
}
