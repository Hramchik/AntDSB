//
// Created by sanek on 08/12/2025.
//

#ifndef ANTDSB_DISCORDCLUSTER_H
#define ANTDSB_DISCORDCLUSTER_H

#include <dpp/dpp.h>

class DiscordCluster {
public:
    static void SetCluster(dpp::cluster* c) {
        instance().cluster = c;
    }

    static dpp::cluster* GetCluster() {
        return instance().cluster;
    }

private:
    dpp::cluster* cluster = nullptr;

    static DiscordCluster& instance() {
        static DiscordCluster inst;
        return inst;
    }
};

#endif //ANTDSB_DISCORDCLUSTER_H