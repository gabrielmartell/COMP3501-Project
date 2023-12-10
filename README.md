# COMP3501 Final Project
Name: Ewan McCarthy, Gabe Martell, Nicholas Faubert, Uzonne Alexander
Student #: 101189646, 101191857, 101192850, 101233844
Due: Dec 8-17th 2023 at 11:59pm EST.

This folder contains our final project which follows our initial projectplan.

	Game Elements:
	- Collection of ingredients (mushrooms, bees, rusted nails) from Grandpa's Cabin.
	- "Hungry Man" enemy AI with chasing and patrolling modes.
	- Hiding mechanics using bushes and cabin.
	- Inventory system to track collected ingredients.
	- HUD displaying images of collected ingredients.
	- Heightfield terrain with collision detection.
	- Illuminated game objects with textured materials.
	- Screen space special effects on player death.
	- Two particle systems for bees and objective markers.
	- Hierarchical game object for Hungry Man.
	- Player-centric camera controls.
	- Skybox using scaled game maps.
	- Multiple running phases with UI.

	Technical Implementations:
	- Use of GLFW, GLEW, and SOIL libraries.
	- Inclusion of glfwNative.h as a file in the library.
  	- Implementation of a objective marker inside the cabin.
	- Implementation of a red-tinted screen effect on player death.
	- Implementation of a sin function timer for a waving screen effect.
	- Implementation of a randomized patrol algorithm for Hungry Man.
	- Implementation of a gravity concept.

	Contributions:
	- Ewan McCarthy: Created the plane, expanded it, and performed code cleanup.
	- Gabe Martell: Created obj files, implemented the random generation algorithm, handled player and Hungry Man collision, set up props on the map, and created HUD artwork.
  	- Uzonna Alexander: Created the skybox, game screens, on-screen icons, death animation, main patrol algorithm, and introduced gravity.
	- Nicholas Faubert: Implemented the function of game score, collectibles, heightmap calculations, and cleaned up a lot of code.

Changed:

	Map Design:
	- Deviation from the original map plan for improved gameplay.
	- Adjustments to the hill height and slope.
Concepts:

	Gameplay Concepts:
	- Collection and management of ingredients.
	- Enemy AI with distinct behaviors (chasing, patrolling).
	- Hiding mechanics using environmental elements (bushes, cabin).
 	- Inventory system with limitations on ingredient collection.

	Technical Concepts:
	- Use of heightmaps for terrain generation.
	- Illuminated game objects with textured materials and lighting effects.
	- Screen space special effects for player death.
	- Implementation of particle systems for bees and objective markers.
	- Hierarchical game object structure for the Hungry Man.
	- Player-centric camera controls for improved user experience.
	- Skybox creation using scaled game maps.

Post-mortem Reflections:
	- Emphasis on proper lighting and map setup.
	- Acknowledgment of successful implementation of heightmap and Blender-made objects.

//!!/ We've marked most of the comments with, //!/ so that it can easily be found by Intructors/TAs with ctrl-f :).
