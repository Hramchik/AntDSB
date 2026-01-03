//
// Created by sanek on 05/12/2025.
//

#ifndef ANTDSB_BUILTINCOMMANDS_H
#define ANTDSB_BUILTINCOMMANDS_H

class BuiltInCommands {
public:
    static void RegisterAll();
    static void SetCluster(dpp::cluster* cluster);
};

#endif //ANTDSB_BUILTINCOMMANDS_H