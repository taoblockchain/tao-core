// (c) 2016 TAO

#include "init.h"
#include "main.h"
#include "dbwrapper.h"

using namespace std;


class CIntelliTxDB : public CDBWrapper {
    string sType;

public:
    ~CIntelliTxDB();
    
    CIntelliTxDB(size_t nCacheSize, bool fMemory, bool fWipe, std::string& t) : CDBWrapper(GetDataDir() / t, nCacheSize, fMemory, fWipe) 
    {
        sType = t;
    }
    bool ReconstructIntelliTxIndex(CBlockIndex *pindexRescan);
};
