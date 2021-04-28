
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

	// gbuffers:
		// diffuse rgb, object type tag
		// normal xyz,
		// depth (float as vec3), 
		
	// lighting!
		// g-buffers
			// add gbuffers to start screen as well...
			// blit info, then copy to screen (temp).
			// get lighting pass working and blit to screen.
		// directional lights, point lights, spot lights
		// proper materials!
	
	// player shield visual:
		// draw shield as semi-transparent (second pass!)
		// shield down / recharge particle effects

	// effects:
		// experiment with different particle sizes?
		// asteroid explode / split effects (rock chunks?)
		// lights for:
			// (sun) lasers, engine boost, powerups, player searchlight
		// *subtle* bloom effect?
		// make player shield particles not collide with player when shield is up, and collide with player when shield is down?

	// misc:
		// player collision with powerup slows down player :(

	// player controls:
		// remove strafing / thrusters (-> classic asteroids controls)
		// reduce drag on player ship
		// fix key presses:
			// when both direction keys are pressed, release one -> 
			// ship should move in the direction of the one still pressed

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
