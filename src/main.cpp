#include "Birb2D.hpp"
#include "Textgen.hpp"
#include "Variables.hpp"
#include <iostream>
#include <string.h>
#include <vector>

using namespace Birb;

/* Function declarations */
static void start(Game& game);
static void input(Game& game);
static void update(Game& game);
static void render(Game& game);
static void post_render();
static void cleanup();

bool arg_parse(char* arg, std::vector<std::string> arg_options)
{
	for (size_t i = 0; i < arg_options.size(); ++i)
		if (!strcmp(arg, arg_options[i].c_str()))
			return true;

	return false;
}

BirbType::Variables* v;

void OnWindowResize(Window& window)
{
	v->CenterTextEntities(window.dimensions);
	v->ResetCaret();

	v->background_color.w = window.dimensions.x + 1600;
	v->background_color.h = window.dimensions.y;
}

int main(int argc, char** argv)
{
	Game::WindowOpts window_options;
	window_options.title 				= "BirbType";
	window_options.window_dimensions 	= { 1280, 720 };
	window_options.refresh_rate 		= 240;
	window_options.resizable 			= true;

	/* Launch options */
	if (argc == 2)
	{
		/* Help page */
		if (arg_parse(argv[1], { "-h", "--h", "-help", "--help", "help" }))
		{
			std::cout << "BirbType help page :3\n"\
				<< " -w\tforce windowed mode\n"\
				<< " -h\tshow this small help page and quit\n";
			return 0;
		}

		/* Windowed mode */
		if (arg_parse(argv[1], { "-w", "--window", "--windowed", "window", "windowed" }))
			window_options.resizable = false;
	}


	Game game_loop(window_options, start, input, update, render);

	/* Optional extra functions */
	game_loop.post_render = post_render;
	game_loop.cleanup = cleanup;

	/* Start the game loop */
	game_loop.Start();

	return 0;
}

/* start() is called before the game loop starts.
 * Useful for doing stuff that will only run once before
 * the game starts */
void start(Game& game)
{
	//Splash splash(*game.window);
	//splash.Run();

	v = new BirbType::Variables;

	v->background_color.color = Colors::Nord::PolarNight::nord0;
	v->background_color.x = -800;
	v->background_color.y;
	v->background_color.w = game.window->dimensions.x + 1600;
	v->background_color.h = game.window->dimensions.y;
	v->game_scene.AddObject(v->background_color);

	/* Load fonts */
	v->mononoki_large = new Font("mononoki.ttf", 24);

	/* Load entities */
	v->wpm_text.Construct("WPM: -", Vector2Int(0, 0), v->mononoki_large, v->white_text_color);
	v->game_scene.AddObject(v->wpm_text);

	v->typing_entity.Construct("", Vector2Int(64, 64), v->mononoki_large, v->white_text_color);
	v->typing_entity.renderingPriority = 2;
	v->typing_entity.name = "Typing entity";
	v->game_scene.AddObject(v->typing_entity);

	for (int i = 0; i < v->line_count; ++i)
	{
		v->target_text_entity[i].Construct(v->text_to_type[i], Vector2Int(64, 64 + 32 * i), v->mononoki_large, Colors::Nord::PolarNight::nord3);
		v->target_text_entity[i].renderingPriority = 1;
		v->game_scene.AddObject(v->target_text_entity[i]);
	}

	/* Generate some text to type */
	v->ResetTextToType();


	v->CenterTextEntities(game.window->dimensions);

	v->caret_rect = Rect(v->typing_entity.rect.x, v->typing_entity.rect.y, 1, v->target_text_entity[0].rect.h);
	v->caret_rect.renderingPriority = 10;
	v->caret_rect.color = v->white_text_color;
	v->game_scene.AddObject(v->caret_rect);

	/* Set game window resizing callback function */
	game.window->OnWindowResize = OnWindowResize;

	/* Construct the caret particle source */
	v->ConstructParticleSystem(game.time_step());
	v->caret_particle_source.renderingPriority = 5;
	v->game_scene.AddObject(v->caret_particle_source);
}

/* input() is called at the beginning of the frame
 * before update(). Behind the curtains it does input
 * polling etc. and then passes the SDL_Event into
 * this function */
void input(Game& game)
{
	switch (game.event->type)
	{
		case SDL_TEXTINPUT:
			v->text_typed += game.window->event.text.text;
			v->backspace_combo = 0;
			v->text_changed = true;

			/* Start the wpm timer when the user starts typing for
			 * the first time after either a reset or game start */
			if (!v->wpm_timer.running)
				v->wpm_timer.Start();
			break;

		/* Change the background color by clicking anywhere */
		case SDL_MOUSEBUTTONDOWN:
			if (game.event->button.button == 1)
				v->cur_background_color++;
			else if (game.event->button.button == 3)
				v->cur_background_color--;

			if (v->cur_background_color < 0)
				v->cur_background_color = 9;
			else if (v->cur_background_color > 9)
				v->cur_background_color = 0;

			v->changing_background_color = true;
			v->color_lerp_timer.Start();
			break;
	}

	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (game.window->isKeyDown() && state[SDL_SCANCODE_F8])
		Diagnostics::Debugging::Overlays::ResourceMonitor = !Diagnostics::Debugging::Overlays::ResourceMonitor;

	if (game.window->isKeyDown())
	{
		/* Handle backspace */
		if (game.event->key.keysym.scancode == SDL_SCANCODE_BACKSPACE && v->text_typed.size() > 0)
		{
			Global::RenderVars::CameraPosition.x += Global::random.RandomFloat(2, 4 + v->backspace_combo / 4.0f);
			v->backspace_combo++;
			v->screenshake_timer.Start();

			/* If control is held, go back by one word */
			if (state[SDL_SCANCODE_LCTRL])
			{
				/* Find the previous whitespace. If there are none, reset the typing_text */
				bool whitespace_found = false;
				for (int i = v->text_typed.size() - 2; i > 0; --i)
				{
					if (v->text_typed[i] == ' ')
					{
						v->text_typed.erase(i + 1, v->text_typed.size() - (i + 1));
						whitespace_found = true;
						break;
					}
				}

				if (!whitespace_found)
					v->text_typed.clear();

				v->text_changed = true;
			}
			else
			{
				v->text_typed.pop_back();
				v->text_changed = true;
			}
		}

		/* Clear all typed text in bash shell style if Ctrl+u is hit */
		if (state[SDL_SCANCODE_LCTRL] && state[SDL_SCANCODE_U])
		{
			v->text_typed.clear();
			v->text_changed = true;
		}

		/* Reset WPM and the words */
		if (state[SDL_SCANCODE_R] && state[SDL_SCANCODE_LCTRL])
		{
			v->wpm_lerp_start = 0;
			v->total_chars_typed = 0;
			v->wpm_timer.Stop();
			v->wpm_lerp_timer.Stop();
			v->wpm_text.SetText("WPM: -");

			v->text_typed.clear();
			v->ResetTextToType();
			v->text_changed = true;
		}

		/* Zoom in or out with + and - */
		if (state[SDL_SCANCODE_PAGEUP] || state[SDL_SCANCODE_PAGEDOWN])
		{
			if (state[SDL_SCANCODE_PAGEUP])
				v->mononoki_large->SetSize(v->mononoki_large->GetSize() + 2);
			else if (state[SDL_SCANCODE_PAGEDOWN] && v->mononoki_large->GetSize() > 6)
				v->mononoki_large->SetSize(v->mononoki_large->GetSize() - 2);

			v->text_changed = true;
			v->typing_entity.ReloadSprite();
			v->wpm_text.ReloadSprite();
			for (int i = 0; i < v->line_count; ++i)
				v->target_text_entity[i].ReloadSprite();

			v->ResetCaret();
			v->CenterTextEntities(game.window->dimensions);
		}
	}
}

/* update() is called after input has been handled and
 * before the frame gets rendered. Its useful for any game
 * logic that needs to be updated before rendering */
void update(Game& game)
{
	if (v->text_changed)
	{
		v->text_changed = false;
		v->typing_entity.SetText(v->text_typed);

		/* Move the caret */
		v->caret_rect.x = v->typing_entity.rect.x + v->typing_entity.rect.w;
		v->caret_particle_source.spawn_area.x = v->caret_rect.x;
		v->caret_particle_source.spawn_area.y = v->caret_rect.y + v->caret_rect.h;
		v->caret_particle_source.Emit(true);

	   	/* Turn the text red if there's an error */
		if (v->text_to_type[0].substr(0, v->text_typed.size()) != v->text_typed && !v->typing_error)
		{
			v->caret_particle_source.reference_particle.color = Colors::Nord::Aurora::nord11;
			v->typing_entity.SetTextColor(Colors::Nord::Aurora::nord11);
			v->typing_error = true;
		}
		else if (v->typing_error && v->text_to_type[0].substr(0, v->text_typed.size()) == v->text_typed)
		{
			v->caret_particle_source.reference_particle.color = v->particle_color;
			v->typing_entity.SetTextColor(v->white_text_color);
			v->typing_error = false;
		}
		else if (v->text_to_type[0] == v->text_typed) /* The line was typed correctly */
		{
			/* Calculate the wpm */
			v->total_chars_typed += v->text_to_type[0].size();
			v->wpm_lerp_target = v->wpm();
			v->wpm_lerp_timer.Start();

			/* Move all of the lines up by one and generate a new up-coming line */
			for (int i = 0; i < v->line_count - 1; ++i)
			{
				v->text_to_type[i] = v->text_to_type[i + 1];
			}

			v->text_to_type[v->line_count - 1] = BirbType::GenerateLine(v->max_line_length);

			/* Update all target text entities */
			for (int i = 0; i < v->line_count; ++i)
			{
				v->target_text_entity[i].SetText(v->text_to_type[i]);
			}

			/* Reset the typing text */
			v->text_typed = "";
			v->typing_entity.SetText("");
			v->CenterTextEntities(game.window->dimensions);
			v->ResetCaret();
		}
	}
	else
	{
		v->caret_particle_source.Emit(false);
	}

	/* Handle screenshake interpolation */
	if (Global::RenderVars::CameraPosition != Vector2(0, 0))
		Global::RenderVars::CameraPosition = Math::Lerp(Global::RenderVars::CameraPosition, Vector2(0, 0), v->screenshake_timer.ElapsedSeconds());

	/* Handle background color lerping */
	if (v->changing_background_color)
	{
		v->background_color.color = Math::Lerp(v->background_color.color, v->background_color_list[v->cur_background_color], v->color_lerp_timer.ElapsedSeconds());

		if (v->color_lerp_timer.ElapsedSeconds() >= 1.0f)
		{
			v->background_color.color = v->background_color_list[v->cur_background_color];
			v->changing_background_color = false;
		}
	}

	/* Handle wpm text lerping */
	if (v->wpm_lerp_timer.running && v->wpm_lerp_timer.ElapsedSeconds() < 0.99f)
	{
		v->wpm_text.SetText("WPM: " + std::to_string(
					Math::Clamp(Math::Lerp(v->wpm_lerp_start, v->wpm_lerp_target, v->wpm_lerp_timer.ElapsedSeconds()), v->wpm_lerp_start, v->wpm_lerp_target)
				));
	}
	else if (v->wpm_lerp_timer.running)
	{
		v->wpm_text.SetText("WPM: " + std::to_string(v->wpm_lerp_target));
		v->wpm_lerp_start = v->wpm_lerp_target;
		v->wpm_lerp_timer.Stop();
	}
}

/* render() is called after update() has been finished.
 * Before it gets called, the window will be cleared and
 * after the function has finished running, the rendered frame
 * will be presented */
void render(Game& game)
{
	v->game_scene.Render();
}

/* post_render() will be called after rendering has finished
 * and the timestep stalling has started. On non-windows systems
 * this function call will be done on a separate thread, so you
 * could use it to do some extra preparations for the next frame
 * while the main thread is sitting around doing nothing
 * and waiting to maintain the correct frame rate */
void post_render()
{

}

/* cleanup() gets called after the game loop has finished running
 * and the application is getting closed. This is useful for doing any
 * cleanup that is necessary, like freeing heap allocations etc. */
void cleanup()
{
	delete v->mononoki_large;
	delete v;
}
