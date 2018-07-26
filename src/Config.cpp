#include <Config.h>

void printEncCfg(Config cfg){
    string inFileName = cfg.m_ifName.size() ? cfg.m_ifName : string("<stdin>");
    string outFileName = cfg.m_ofName.size() ? cfg.m_ofName : string("<stdout>");
    string keyFileName = cfg.m_keyFileName.size() ? cfg.m_keyFileName : string("");

    LOG("Encoder settings: ");
    LOG("-i %s ", inFileName.c_str());
    LOG("-f %dx%d ", cfg.m_frameWidth, cfg.m_frameHeight);
    LOG("-o %s ", outFileName.c_str());
    LOG("-w %d ", cfg.m_nWorkingThreads);
    LOG("-p %d ", cfg.m_framesPerThread);

    if(cfg.m_stegModeOn){
        LOG("--stg ");
        LOG("th=%d ", cfg.m_stegThreshold);
        string s = keyFileName.size() ? keyFileName : inFileName + string(".stg");
        LOG("kf=\"%s\" ", s.c_str());
        LOG("up=\"%c\" ", cfg.m_unitPattern);
        if(cfg.m_stegGen != -1){
            LOG("up=\"%d\" ", cfg.m_stegGen);
        }
    }else if(cfg.m_cipheringOn){
        LOG("-c %s ", keyFileName.c_str());
    }

    //encoder specific options
    LOG("-a %d ", cfg.m_alignment);
    LOG("-s %d ", cfg.m_qrScale);
    LOG("-e %d ", cfg.m_eccLevel);
    LOG("-r %d ", cfg.m_frameRepeats);
    LOG("-t %d ", cfg.m_nTrailingFrames);
    LOG("\nQR code Version: %d\n", cfg.m_qrVersion);
}

void printDecCfg(Config cfg){
    string inFileName = cfg.m_ifName.size() ? cfg.m_ifName : string("<stdin>");
    string outFileName = cfg.m_ofName.size() ? cfg.m_ofName : string("<stdout>");
    string keyFileName = cfg.m_keyFileName.size() ? cfg.m_keyFileName : string("");

    LOG("Decoder settings: ");
    LOG("-i %s ", inFileName.c_str());
    LOG("-f %dx%d ", cfg.m_frameWidth, cfg.m_frameHeight);
    LOG("-o %s ", outFileName.c_str());
    LOG("-w %d ", cfg.m_nWorkingThreads);
    LOG("-p %d ", cfg.m_framesPerThread);

    if(cfg.m_stegModeOn){
        LOG("--stg ");
        LOG("kf=\"%s\" ", keyFileName.c_str());
        LOG("up=\"%c\" ", cfg.m_unitPattern);
    }else if(cfg.m_cipheringOn){
        LOG("-c %s ", keyFileName.c_str());
    }

    //decoder specific options
    switch(cfg.m_decMode)
    {
    case MODE_QUICK:
        LOG("-m %s ", "quick");
        break;
    case MODE_MIXED:
        LOG("-m %s ", "mixed");
        break;
    case MODE_SLOW:
        LOG("-m %s ", "slow");
        break;
    }
    //LOG("-m %d ", cfg.m_decMode);
    if(cfg.m_skipDupFrames){
        LOG("-k ");
    }
    LOG("\n");
}
