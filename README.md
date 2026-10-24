# mod-craft-speed

**WORK IN PROGRESS**

Crafting speed by profession for AzerothCore 3.3.5a: the cast time of every craft of a profession is multiplied by a value of its own.

## Key features

- One multiplier per profession (the crafting ones, Smelting, Cooking, First Aid), or a default for all.
- Crafts only: gathering, disenchanting, prospecting, milling and class spells keep their cast time.
- Faster, instant or slower, with an optional floor; applied again by `.reload config`.
- Nothing on the client side, no database change.

## Dependencies

| Dependency | For |
|---|---|
| [AzerothCore](https://github.com/azerothcore/azerothcore-wotlk) (WotLK 3.3.5a) | the server |

## How to install

1. Download `installer.exe` from the [WoW-mods-installer releases](https://github.com/Adrestie/WoW-mods-installer/releases).
2. Stop the worldserver.
3. Run `installer.exe`, give it this folder and the folders it asks for, click **Check**, then **Install**.
4. Rebuild the worldserver.

## How to update

Same steps with the new version's folder, choosing **Update** instead of **Install**. Your `mod-craft-speed.conf` settings are kept. Rebuild the worldserver.

## How to uninstall

Same steps, choosing **Remove**. Rebuild the worldserver.

## Credits

- Based on [mod-craftspeed](https://github.com/Day36512/mod-craftspeed) by Day36512, which applies one value to every craft.

## Configuration

`mod-craft-speed.conf` sets:

- the module on or off;
- the default multiplier, and one multiplier per profession;
- the shortest cast time a craft may get.
