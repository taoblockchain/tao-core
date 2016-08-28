
#include "net.h"
#include "alphanodeconfig.h"
#include "util.h"
#include <base58.h>

CAlphanodeConfig alphanodeConfig;

void CAlphanodeConfig::add(std::string alias, std::string ip, std::string privKey, std::string txHash, std::string outputIndex) {
    CAlphanodeEntry cme(alias, ip, privKey, txHash, outputIndex);
    entries.push_back(cme);
}

bool CAlphanodeConfig::read(boost::filesystem::path path) {
    boost::filesystem::ifstream streamConfig(GetAlphanodeConfigFile());
    if (!streamConfig.good()) {
        return true; // No alphanode.conf file is OK
    }

    for(std::string line; std::getline(streamConfig, line); )
    {
        if(line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string alias, ip, privKey, txHash, outputIndex;
        iss.str(line);
        iss.clear();
        if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
            LogPrintf("Could not parse alphanode.conf line: %s\n", line.c_str());
            streamConfig.close();
            return false;
        }

        add(alias, ip, privKey, txHash, outputIndex);
    }

    streamConfig.close();
    return true;
}
