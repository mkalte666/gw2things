#ifndef GW2CHARACTER_H
#define GW2CHARACTER_H

#include "gw2common.h"

namespace GW2  {

/**
 * @brief The Character class
 */
class Character
{
public:
    /**
     * Callback for the Character class api functions
     */
    using Callback = std::function<void(const Character&)>;

    /**
     * @brief Character
     * @param data Construct a Character from this
     */
    Character(const QJsonObject& data);
};

}; // namespace GW2

#endif // GW2CHARACTER_H
