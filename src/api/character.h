#ifndef _character_h
#define _character_h

#include "endpoint.h"

struct CharacterData : AbstractEndpoint {
    CharacterData(std::string tmpName)
        : AbstractEndpoint("https://api.guildwars2.com/v2/characters/", 10)
        , name(tmpName)
    {
    }

    virtual void show() override;
    virtual void fetch() override;

    void drawOverviewInfo();

    std::string name;
    std::string race;
    std::string gender;
    std::string profession;
    time_t age;
    std::string createdStr;
    int deaths;
};
json_helper(CharacterData);

#endif //_character_h