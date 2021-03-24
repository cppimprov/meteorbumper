
#include "bump_game.hpp"
#include "bump_game_app.hpp"
#include "bump_log.hpp"

#include <SDL.h>
#include <cstdlib>

int main(int , char* [])
{
	using namespace bump;

	{
		auto app = game::app();
		game::run_state({ game::do_start }, app);
	}

	log_info("done!");

	return EXIT_SUCCESS;
}

// todo next:

	// particle field:
		// color (random, persistent w/ particle)
			// base color, variation in hsv
			// how to pick a nice random color?
			// use different random value for size and color!
		// [stretch: movement - slow, brownian motion style?]
		// [stretch: lighting?]

	// we want an asteroid... slowly tumbling through space... we wanted some stuff in the background too... and a space station

	// start screen:
		// create asteroid swarm
			// one asteroid in center of screen
			// camera rotates slowly around that asteroid
			// other asteroids "follow" the first one?
			// (space dust shows movement)
	
// design:

	// camera:
		// top-down, center on player
		// [stretch: if player moving quickly, position camera ahead slightly to increase visibility]
	
	// player:
		// model
			// visually more similar to a "helicopter" (no rotors) - stubby wings
			// add lasers!

		// controls
			// rotate model to face mouse -> directly set position, rather than using torque?
			// a, d strafe, w, s control speed, 
			// [stretch: space to break hard?]
			// mouse 1 to shoot?
		
		// shields / armor system

	// weapons:
		// lasers
		// missiles
		// [stretch: others...]

	// physics:
		// add cuboid shape collision
		// add ray shape and collision
		// add cylinder shape and collision
		// add arbitrary mesh collision (for space station)
		// [stretch: better broadphase - quadtree / octree?]

	// level:
		// space station
			// agglomerate of space station pieces.
			// also docked ships.
		
		// model reusable space-station parts in blender
		// export as one model for now!

		// export physics collision shape as one model? (needs arbitrary mesh collision)

	// asteroids:
		// better random-color generation.
		// can we randomly deform a sphere for each asteroid? (needs arbitrary mesh collision)
		// take damage from weapons and split into smaller asteroids / debris
		// damage player / space-station
	
	// effects:
		// laser / weapon effects
		// smoke trails
		// engine trails (trails / particles)
		// explosions
		// space dust / speed effect

	// rendering:
		// lighting
		// [stretch: background (distant planet / sun?, other space stations)]
		// [stretch: reflection effects off some materials? (glass / shiny metal)]
	
	// ui:
		// player status
		// weapons status (cooldown effects)
		// minimap
		// space-station health
		// score

	// sound:
		// weapon effects
		// ui effects
		// explosion sounds
		// hit sounds
		// background music
	
	// intro screen
		// nice text shader
		// background music
		// asteroid(s) travelling through space
