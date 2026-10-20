/*
 * This file is part of mod-craft-speed, MIT licence (see LICENSE).
 */

/*
 * mod-craft-speed script loader.
 *
 * The function name is imposed by the CMake generation: Add<directory>Scripts,
 * with the dashes of the directory name turned into underscores.
 * mod-craft-speed -> Addmod_craft_speedScripts
 */

void AddSC_craft_speed();

void Addmod_craft_speedScripts()
{
    AddSC_craft_speed();
}
