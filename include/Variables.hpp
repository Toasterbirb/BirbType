#pragma once
#include <string>
#include <vector>
#include "Entities/Text.hpp"
#include "ParticlePatterns/RandomizedCircle.hpp"
#include "ParticleSource.hpp"
#include "Rect.hpp"
#include "Scene.hpp"
#include "Timer.hpp"

using namespace Birb;

namespace BirbType
{
	struct Variables
	{
		static constexpr int line_count = 3;

		std::string text_to_type[line_count];
		std::string text_typed = "";
		static inline bool text_changed = false;
		static inline bool typing_error = false;

		void ConstructParticleSystem(TimeStep* ts);
		Particles::ParticleSource caret_particle_source;
		Particles::Patterns::RandomizedCircle particle_pattern;
		Particles::Particle reference_particle;
		Color particle_color;

		Rect caret_rect;
		void ResetCaret();

		/* Entities */
		Entity::Text wpm_text;
		Entity::Text typing_entity;
		Entity::Text target_text_entity[line_count];
		void ResetTextToType();
		void CenterTextEntities(Vector2Int window_dimensions);

		/* Colors */
		Timer color_lerp_timer;
		bool changing_background_color = false;
		Color white_text_color = Colors::Nord::SnowStorm::nord6;

		int cur_background_color = 0;
		Color background_color_list[10] = {
			Colors::Nord::PolarNight::nord0,
			Colors::Nord::PolarNight::nord1,
			Colors::Nord::Frost::nord7,
			Colors::Nord::Frost::nord8,
			Colors::Nord::Frost::nord9,
			Colors::Nord::Frost::nord10,
			//Colors::Nord::Aurora::nord11,
			Colors::Nord::Aurora::nord12,
			Colors::Nord::Aurora::nord13,
			Colors::Nord::Aurora::nord14,
			Colors::Nord::Aurora::nord15
		};

		/* Fonts */
		Font* mononoki_large;

		/* Misc. stuff */
		Scene game_scene;
		Rect background_color;
		static constexpr int max_line_length = 60;
		Timer screenshake_timer;
		int backspace_combo = 0;

		/* Words per minute calculations */
		static inline int total_chars_typed = 0;
		Timer wpm_timer;
		Timer wpm_lerp_timer;
		static inline int wpm_lerp_start = 0;
		static inline int wpm_lerp_target = 0;
		int wpm() const;
	};
}
