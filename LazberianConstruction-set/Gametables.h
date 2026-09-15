#pragma once
#include <string>
#include <vector>


inline const std::vector<std::string> ItemEffects = {
    "--", "Female lock", "x2", "x3", "x4", "--", "--", "--", "--", "--", "Ignore defense", "--",
    "Ignore shields", "Ignore armor class defense", "Ignore horse defense", "--", "Miracle effect", "Renewal HP",
    "2x cripple", "Use numerical durability", "Can't counter (lance effect)", "Can't move", "--", "--", "Chance def UP", "Defend adjutant",
    "Gigas Kight lock", "Nosferatu effect", "Damage EXP", "General lock", "Infantry type only", "--", "Lock to user", "--",
    "Unbreakable", "Quest item", "--", "--", "--", "??", "Unobtainable dagger", "Unobtainable arrow",
    "Unobtainable mace", "money", "--", "Hide", "Overwatch", "Parry", "--", "--", "--", "--",
    "Vitria effect", "??", "--", "??", "Can't kill", "--", "Add level to damage", "Scorpio buff effect (?)",
    "0 range throwable", "--", "--", "Goes to broken", "Horse", "Famed horse", "1+ move", "--", "--", "Paladin lock",
    "Star icon", "--", "Assassin lock", "Aiantos effect", "--", "--", "--", "--", "--", "Blackrider lock",
    "Apostle lock", "Horse lover", "Pascanion backfire", "Holy vantage", "Can't attack flier", "--",
    "Restore uses by chapter", "--", "--", "--", "Material type", "--", "--", "--", "--", "--"
};

inline const std::vector<std::string> ItemEffectRates = {
    "Half damage", "--", "Devil reversal", "--", "+ steal rate", "Thunder damage", "Fire damage", "wind damage",
    "Swordbreaker", "--", "Insta kill horses", "Insta break shields", "--", "Poison effect", "Sleep chance", "--", "Negate dark attacks", "Negate criticals",
    "Reflect damage chance", "Up growths", "--", "Cripple rate", "--", "Dark damage", "Holy damage", "To broken weapon?",
    "Fire defense", "--", "Thunder defense", "--", "Disarm rate", "Injure rate", "--", "Holy defense"
};

inline const std::vector<std::string> Durability = { "S", "A", "B", "C", "D", "E", "F" };


inline const std::vector<std::string> Skills = {
    "Locktouch", "Canto", "Overwatch", "Watchful", "Triple-Shot", "Counter", "Shieldfaire", "Mercy",
    "Adept", "Vantage", "Vengeful", "Spearbane", "Provoke", "Safezone", "Guard", "Astra",
    "Arrowbane", "Commander", "Paragon", "Fortune", "Obfuscate", "Steal", "Silence", "Hateful", "Battlecry",
    "Blessing", "Sunder", "Charisma", "Acrobat", "Evasion", "--", "--", "Axeguard",
    "Deadeye", "Huntsman", "Aim", "Swimmer", "Resistor", "Search", "One-Two", "Deathmatch",
    "Horseswap", "Throw", "Parry", "Prepared", "Robust", "Vulnerable", "Focuschant", "Ourbond",
    "Horselover", "Robbery", "Despoil", "Knockaway", "Iaido", "Pulverize", "Mug+", "Miracle",
    "Armsthrift", "Hide", "Hurry", "Lance", "Blade", "--", "--", "Cavalry", "Celerity",
    "Swordbane", "Axebane", "Limited", "Convert", "Expert", "Magicbane", "Mug", "Versatile",
    "Slowstart", "Pitchfork", "Pursuit", "Doubleshot", "Commander+", "Return", "Camouflage", "Scotopic",
    "Supporter", "Climber", "Windsweep", "Desperation", "Flourish", "Slowstart+", "Robust+",
    "Imbue", "Critical", "Protector", "Debility", "Maim", "--", "--"
};

inline const std::vector<std::string> WeaponCategories = {
    "Knife", "Sword", "Spear/Lance", "Axe", "Bow", "Crossbow",
    "Fire", "Thunder", "Wind", "Holy", "Dark", "S.Shield", "M.Shield", "L.Shield"
};

inline const std::vector<std::string> GrowthProficiencyCategories = {
    "Knife", "Sword", "Spear/Lance", "Axe", "Bow", "Crossbow",
    "Fire", "Thunder", "Wind", "Holy", "Dark", "Shield"
};

inline const std::vector<std::string> MountStatus = { "Unmounted", "Mounted", "Flying" };

inline const std::vector<std::string> UnitTypes = {
    "Cavalry", "Infantry", "Armor", "Thief", "Lt-Infantry", "Mage", "Priest", "Flier"
};

inline const std::vector<std::string> MovementTypes = {
    "Cavalry", "Lt-Cavalry", "Flier", "Knight", "Armor", "Infantry", "Thief", "Civilian",
    "Special", "Snow", "War-Priest", "Lt-Infantry", "Priest", "--"
};