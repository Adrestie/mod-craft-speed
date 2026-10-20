/*
 * This file is part of mod-craft-speed, MIT licence (see LICENSE).
 *
 * Based on mod-craftspeed by Day36512 (https://github.com/Day36512/mod-craftspeed),
 * which applies one multiplier to every crafting spell. This module applies one
 * multiplier per profession and can be reloaded without a restart.
 */

/*
 * The core has no hook on a spell's cast time, so the module changes the cast
 * time entry of the crafting spells, once, after the spells are loaded.
 *
 * Several spells share a SpellCastTimes.dbc entry, so each crafting spell gets
 * its own copy of its entry and points to it; the original entries are never
 * written. The copies live as long as the server: on `.reload config` only
 * their CastTime is written again, from the original value, so a spell being
 * cast at that moment never reads a freed entry.
 *
 * A crafting spell is a spell of a profession's skill line (SkillLineAbility)
 * that creates or enchants an item and has a cast time. Class spells that
 * create items (conjured food, healthstones) belong to no profession and are
 * left alone, as are gathering, disenchanting, prospecting and milling.
 *
 * The client still shows the cast time of its own Spell.dbc in tooltips; the
 * cast bar follows the server.
 */

#include "Config.h"
#include "DBCStores.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    struct Profession
    {
        uint32 skill;
        char const* key; // CraftSpeed.<key>
    };

    // Mining stands for smelting: gathering is an "open lock" spell, not a craft.
    constexpr Profession Professions[] =
    {
        { SKILL_ALCHEMY,        "Alchemy"        },
        { SKILL_BLACKSMITHING,  "Blacksmithing"  },
        { SKILL_ENCHANTING,     "Enchanting"     },
        { SKILL_ENGINEERING,    "Engineering"    },
        { SKILL_INSCRIPTION,    "Inscription"    },
        { SKILL_JEWELCRAFTING,  "Jewelcrafting"  },
        { SKILL_LEATHERWORKING, "Leatherworking" },
        { SKILL_TAILORING,      "Tailoring"      },
        { SKILL_MINING,         "Smelting"       },
        { SKILL_COOKING,        "Cooking"        },
        { SKILL_FIRST_AID,      "FirstAid"       },
    };

    struct CraftSpell
    {
        SpellCastTimesEntry* copy;
        int32 originalCastTime;
        uint32 skill;
    };

    struct CraftSpeedState
    {
        bool enable = true;
        float defaultMultiplier = 0.25f;
        uint32 minCastTime = 0;
        std::unordered_map<uint32 /*skill*/, float> multipliers;

        bool spellsReady = false;
        std::vector<std::unique_ptr<SpellCastTimesEntry>> copies;
        std::vector<CraftSpell> spells;
    };

    CraftSpeedState& State()
    {
        static CraftSpeedState state;
        return state;
    }

    float ReadMultiplier(std::string const& key, float fallback)
    {
        float value = sConfigMgr->GetOption<float>(key, -1.0f, false);
        if (value < 0.0f)
            return fallback;
        return std::min(value, 10.0f);
    }

    void LoadConfig()
    {
        CraftSpeedState& state = State();
        state.enable = sConfigMgr->GetOption<bool>("CraftSpeed.Enable", true);
        state.defaultMultiplier = ReadMultiplier("CraftSpeed.Default", 0.25f);
        state.minCastTime = sConfigMgr->GetOption<uint32>("CraftSpeed.MinCastTime", 0);

        state.multipliers.clear();
        for (Profession const& profession : Professions)
            state.multipliers[profession.skill] = ReadMultiplier(std::string("CraftSpeed.") + profession.key, state.defaultMultiplier);
    }

    bool CreatesOrEnchantsItem(SpellInfo const* spellInfo)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            switch (spellInfo->Effects[i].Effect)
            {
                case SPELL_EFFECT_CREATE_ITEM:
                case SPELL_EFFECT_CREATE_ITEM_2:
                case SPELL_EFFECT_TRADE_SKILL:
                case SPELL_EFFECT_ENCHANT_ITEM:
                case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
                case SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC:
                    return true;
                default:
                    break;
            }
        }
        return false;
    }

    // The profession the spell is taught by, or 0.
    uint32 ProfessionOf(uint32 spellId)
    {
        SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);
        for (auto itr = bounds.first; itr != bounds.second; ++itr)
            for (Profession const& profession : Professions)
                if (itr->second->SkillLine == profession.skill)
                    return profession.skill;
        return 0;
    }

    // Once, after the spells are loaded: every crafting spell gets its own entry.
    void CollectSpells()
    {
        CraftSpeedState& state = State();
        for (uint32 spellId = 1; spellId < sSpellMgr->GetSpellInfoStoreSize(); ++spellId)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo || !spellInfo->CastTimeEntry || spellInfo->CastTimeEntry->CastTime <= 0)
                continue;

            if (!CreatesOrEnchantsItem(spellInfo))
                continue;

            uint32 skill = ProfessionOf(spellId);
            if (!skill)
                continue;

            state.copies.push_back(std::make_unique<SpellCastTimesEntry>(*spellInfo->CastTimeEntry));
            SpellCastTimesEntry* copy = state.copies.back().get();
            state.spells.push_back({ copy, copy->CastTime, skill });
            const_cast<SpellInfo*>(spellInfo)->CastTimeEntry = copy;
        }
        state.spellsReady = true;
    }

    void Apply()
    {
        CraftSpeedState& state = State();
        if (!state.spellsReady)
            return;

        uint32 changed = 0;
        for (CraftSpell const& spell : state.spells)
        {
            int32 castTime = spell.originalCastTime;
            if (state.enable)
            {
                float multiplier = state.multipliers[spell.skill];
                castTime = int32(float(spell.originalCastTime) * multiplier);
                if (castTime < int32(state.minCastTime))
                    castTime = std::min(int32(state.minCastTime), spell.originalCastTime);
            }

            if (castTime != spell.originalCastTime)
                ++changed;
            spell.copy->CastTime = castTime;
        }

        LOG_INFO("module", "mod-craft-speed: {} crafting spells, {} with a changed cast time{}",
            state.spells.size(), changed, state.enable ? "" : " (module disabled)");
    }
}

class CraftSpeedWorldScript : public WorldScript
{
public:
    CraftSpeedWorldScript() : WorldScript("CraftSpeedWorldScript", { WORLDHOOK_ON_AFTER_CONFIG_LOAD, WORLDHOOK_ON_STARTUP }) { }

    void OnAfterConfigLoad(bool reload) override
    {
        LoadConfig();
        if (reload)
            Apply();
    }

    void OnStartup() override
    {
        CollectSpells();
        Apply();
    }
};

void AddSC_craft_speed()
{
    new CraftSpeedWorldScript();
}
