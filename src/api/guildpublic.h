#ifndef _guild_public_h
#define _guild_public_h

#include "endpoint.h"

struct GuildPublicData : public AbstractEndpoint {
    GuildPublicData(std::string idForFetch = "")
        : AbstractEndpoint("https://api.guildwars2.com/v2/guild/", 60)
        , id(idForFetch)
    {
    }
    void fetch() override;
    void show() override;

    std::string id;
    std::string name = "???";
    std::string motd = "???";
};
json_helper(GuildPublicData);

#endif //_guild_public_h