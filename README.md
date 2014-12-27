Minimal CRYENGINE
=================

Home of the Minimal GameDLL for CRYENGINE, aimed to give developers a good base for new projects.

# Usage

The Minimal project root directory should be merged with a CRYENGINE installation, most commonly [Engine as a Service](http://store.steampowered.com/app/220980/). 

The solution file can be found in Code/Solutions/CRYENGINE_Minimal.sln, but will not compile without the code package obtained from the 'Engine as a Service' package, or licensee builds.

## Game directory setup
The sample can be run out of any game directory, but it is recommended to use the nearly empty GameMinimal directory included with this project.

To do so, follow the steps below:

1. Copy GameMinimal/, as well as the CryGameMinimal library generated by the compiler to your engine installation
2. Open system.cfg and change the value of 'sys_game_folder' to "GameMinimal".
3. Run the game using the Editor, or the Launcher. (Note how it's a stripped down game project, no special game logic is included)

# Compatible Engine Version(s)
The project currently requires CRYENGINE 3.6.14, but will likely compile just fine with other 3.6.* versions.
