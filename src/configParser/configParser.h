//
// Created by sanek on 30/11/2025.
//

#ifndef ANTDSB_CONFIGPARSER_H
#define ANTDSB_CONFIGPARSER_H

#include <fstream>

class configParser {
    public:
        static void configInit();
        static bool configExists(const std::string& name);
        static std::string configRead(const std::string& name);
};


#endif //ANTDSB_CONFIGPARSER_H