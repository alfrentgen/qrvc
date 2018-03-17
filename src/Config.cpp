#include <Config.h>

void printEncCfg(Config cfg){
    string inFileName = cfg.m_ifName.size() ? cfg.m_ifName : string("<stdin>");
    string outFileName = cfg.m_ofName.size() ? cfg.m_ofName : string("<stdout>");
    string keyFileName = cfg.m_keyFileName.size() ? cfg.m_keyFileName : string("");

    LOG("Encoder settings: ");
    LOG("-i %s ", inFileName.c_str());
    LOG("-f %dx%d ", cfg.m_frameWidth, cfg.m_frameHeight);
    LOG("-a %d ", cfg.m_alignment);
    LOG("-o %s ", outFileName.c_str());
    LOG("-s %d ", cfg.m_qrScale);
    LOG("-e %d ", cfg.m_eccLevel);
    LOG("-t %d ", cfg.m_nTrailingFrames);
    LOG("-r %d ", cfg.m_frameRepeats);
    LOG("-w %d ", cfg.m_nWorkingThreads);
    LOG("-p %d ", cfg.m_framesPerThread);
    if(cfg.m_cipheringOn){
        LOG("-c %s ", keyFileName.c_str());
    }
    LOG("\nQR Version: %d ", cfg.m_qrVersion);
    LOG("\n");
}

void printDecCfg(Config cfg){
    string inFileName = cfg.m_ifName.size() ? cfg.m_ifName : string("<stdin>");
    string outFileName = cfg.m_ofName.size() ? cfg.m_ofName : string("<stdout>");
    string keyFileName = cfg.m_keyFileName.size() ? cfg.m_keyFileName : string("");

    LOG("Encoder settings: ");
    LOG("-i %s ", inFileName.c_str());
    LOG("-f %dx%d ", cfg.m_frameWidth, cfg.m_frameHeight);
    LOG("-o %s ", outFileName.c_str());
    LOG("-w %d ", cfg.m_nWorkingThreads);
    LOG("-p %d ", cfg.m_framesPerThread);
    LOG("-m %d ", cfg.m_decMode);
    if(cfg.m_skipDupFrames){
        LOG("-k ");
    }
    if(cfg.m_cipheringOn){
        LOG("-c %s ", keyFileName.c_str());
    }
    LOG("\n");
}
