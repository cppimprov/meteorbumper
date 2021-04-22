
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

	// remove strafing / manouvering (not really necessary)
		// reduce drag

	// player shield visual:
		// draw shield (iso sphere?)
		// make collision circle larger with shield
		// shield down / recharge particle effects

	// player armor visual:
		// when hit, yellow particle effect?
		// fewer, less speedy particles
	
	// lighting!
		// g-buffers
		// directional lights, point lights,
		// proper materials
	
	// effects:
		// weapon hits (rock chunks / dust, laser "dissolve" type effect)
		// asteroid explode / split effects
		// lights for:
			// (sun) lasers, engine boost, powerups, player searchlight
		// *subtle* bloom effect?

	// player lasers:
		// add player velocity to lasers?
		// make lasers slightly faster?

	// player controls:
		// fix input:
			// when both direction keys are pressed, release one -> 
			// ship should move in the direction of the one still pressed


	// asteroid shapes:

		// load icosphere, normalize points to make sure they're on a sphere
		// use vec3 simplex noise to get offset value.
		// offset points along normal.

		// save mesh on cpu.
		// create render data and render.
		// i guess we can use 4d noise if we use an id as the other coordinate???
		// then we only need to keep a single mesh on the cpu, and use the noise function for calculations!!!

		// so what to do?
			// easiest thing to do is to make models and export them. we can make sure they're convex.
				// we need the full 3d collision, right?
				// (or add convex hull when loading)
			// how do we then restrict to 2d collision?

			// start with an isosphere mesh, move some vertices in / out based on noise texture?
		
	// start screen:
		// create asteroid swarm
			// one asteroid in center of screen
			// camera rotates slowly around that asteroid
			// other asteroids move with the first one
			// (space dust shows movement)

		// space station... moves away at the same speed as the main asteroid (!) so it never gets closer
	
// todo sometime:
	
	// particle field:
		// make more shader variables uniforms for better control (e.g. point size)
		// [stretch: movement - slow, brownian motion style?]
		// [stretch: lighting?]

	// player controls:
		// does it make sense?
		// make the player rotate over time instead of instantly (damped harmonic oscillation, e.g. https://gamedev.stackexchange.com/a/109828/)
		// [stretch: zoom out when player is moving fast]

// design:

	// camera:
		// top-down, center on player
		// [stretch: if player moving quickly, position camera ahead slightly to increase visibility]
	
	// player:

		// model
			// visually more similar to a "helicopter" (no rotors) - stubby wings?
			// add lasers!

		// controls
			// rotate model to face mouse -> directly set position, rather than using torque?
			// a, d strafe, w, s control speed,
			// mouse click to shoot
		
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
