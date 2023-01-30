#include "Variables.hpp"
#include "Vector/Vector2Int.hpp"
#include "Textgen.hpp"

namespace BirbType
{
	void Variables::ConstructParticleSystem(TimeStep* ts)
	{
		/* Create the reference particle */
		particle_color = Colors::Nord::SnowStorm::nord4;
		reference_particle.color = particle_color;
		reference_particle.gravity = 50.0f;
		reference_particle.particle_speed = 10.0f;
		reference_particle.life = 5.0f;
		reference_particle.size = 1.0f;

		/* Construct the particle source */
		caret_particle_source = Particles::ParticleSource(ts);
		caret_particle_source.reference_particle = reference_particle;
		caret_particle_source.pattern = &particle_pattern;
		caret_particle_source.speed = 0.0f;
	}

	void Variables::ResetCaret()
	{
		caret_rect.x = typing_entity.rect.x;
		caret_rect.y = typing_entity.rect.y;
		caret_rect.h = target_text_entity[0].rect.h;
	}

	void Variables::ResetTextToType()
	{
		for (int i = 0; i < line_count; ++i)
		{
			text_to_type[i] = BirbType::GenerateLine(max_line_length);
			target_text_entity[i].SetText(text_to_type[i]);
		}
	}

	void Variables::CenterTextEntities(Vector2Int window_dimensions)
	{
		typing_entity.rect.x = window_dimensions.x / 2.0f - target_text_entity[0].rect.w / 2.0f;
		typing_entity.rect.y = window_dimensions.y / 2.0f - (line_count / 2.0f) * 32;

		wpm_text.rect.x = window_dimensions.x / 2.0f - wpm_text.rect.w / 2.0f;
		wpm_text.rect.y = typing_entity.rect.y - wpm_text.rect.h - 8;

		for (int i = 0; i < line_count; ++i)
		{
			target_text_entity[i].rect.x = window_dimensions.x / 2.0f - target_text_entity[i].rect.w / 2.0f;
			target_text_entity[i].rect.y = typing_entity.rect.y + i * (target_text_entity[i].rect.h + 4);
		}
	}

	int Variables::wpm() const
	{
		return (total_chars_typed / 5.0f) / wpm_timer.ElapsedMinutes();
	}
}
