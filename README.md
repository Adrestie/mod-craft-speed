# mod-craft-speed

Crafting speed by profession for AzerothCore 3.3.5a: the cast time of every
craft of a profession is multiplied by a value of its own, set in the
configuration. Based on
[mod-craftspeed](https://github.com/Day36512/mod-craftspeed) by Day36512,
which applies one value to every craft.

## How it behaves

- **One value per profession**: Alchemy, Blacksmithing, Enchanting,
  Engineering, Inscription, Jewelcrafting, Leatherworking, Tailoring,
  Smelting (the crafting part of Mining), Cooking and First Aid. A profession
  left at -1 takes `CraftSpeed.Default`.
- **Crafts only**: the spells a profession teaches that create or enchant an
  item. Gathering, disenchanting, prospecting, milling and the class spells
  that create items (conjured food, healthstones) keep their cast time.
- **Faster or slower**: 0.5 crafts twice as fast, 0 instantly, 2 twice as
  slowly. `CraftSpeed.MinCastTime` sets a floor.
- **Reloadable**: `.reload config` applies new values without a restart.
- **Tooltips** still show the normal cast time, read by the client from its
  own files; the cast bar shows the real one.

## How it works

The core has no hook on a spell's cast time, so at startup the module gives
each crafting spell its own copy of its `SpellCastTimes.dbc` entry (several
spells share one) and writes the new cast time there. A crafting spell is
found through `SkillLineAbility`: a spell of a profession's skill line, with
a cast time, that creates or enchants an item.

## Requirements

| | |
|---|---|
| [AzerothCore](https://github.com/azerothcore/azerothcore-wotlk) | 3.3.5a, built with the module in `modules/` |

No database change, nothing on the client side.

## Installing

Stop the world server, then run `installer.exe`, the WoW-mods installer
([WoW-mods-installer releases](https://github.com/Adrestie/WoW-mods-installer/releases)),
and give it this folder. It copies the module into `modules/` and the
configuration into `configs/modules/`. Then rebuild the core.

By hand: copy the module into `modules/`, copy
`conf/mod-craft-speed.conf.dist` to `configs/modules/mod-craft-speed.conf`,
rebuild the core.

## Configuration

`mod-craft-speed.conf`:

| Setting | Default | |
|---|---|---|
| `CraftSpeed.Enable` | 1 | the module on or off |
| `CraftSpeed.Default` | 0.25 | multiplier of the professions left at -1 |
| `CraftSpeed.<Profession>` | -1 | multiplier of one profession, -1 for the default |
| `CraftSpeed.MinCastTime` | 0 | shortest cast time, in milliseconds |

## Licence

MIT, like mod-craftspeed.
