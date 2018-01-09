#include "Chunk.h"
#include "pch.h"

using namespace std;

Chunk::Chunk(int32_t inBuffSize, uint8_t val):
m_inBuffer(inBuffSize, val), m_outBuffer(0)
{
    Init();
}

Chunk::~Chunk()
{    //dtor
}

void Chunk::Init(uint8_t val){
    //m_decResults.decodedBytes = 0;
    m_chunkID = 0;
    m_frameID = 0;
    m_outHash = 0;
    m_rendered = false;
    m_inBuffer.assign(m_inBuffer.size(), val);
}

void Chunk::SetInBuffer(uint8_t* pBuffer, int32_t size){
    m_inBuffer.assign(pBuffer, pBuffer + size);
}

vector<uint8_t>& Chunk::GetInBuffer(){
    return m_inBuffer;
}

void Chunk::FreeInBuffer(){
    m_inBuffer.clear();
}

void Chunk::SetOutBuffer(uint8_t* pBuffer, int32_t size){
    m_outBuffer.assign(pBuffer, pBuffer + size);
}

vector<uint8_t>& Chunk::GetOutBuffer(){
    return m_outBuffer;
}

void Chunk::FreeOutBuffer(){
    m_outBuffer.clear();
}

uint32_t Chunk::CalcHashsum(uint8_t* pBuffer, int32_t bufSize){
    uint32_t hash = bufSize;
    for(uint32_t i = 0; i < bufSize; i++){
        hash = hash + (hash << 2);
        hash += pBuffer[i];
    }
    return hash;
}

Chunk::Chunk(Chunk&& ch):
m_inBuffer(0), m_outBuffer(0)
{// move constructor
    m_chunkID = ch.m_chunkID;
    m_frameID = ch.m_frameID;
    m_inHash = ch.m_inHash;
    m_outHash = ch.m_outHash;
    m_rendered = ch.m_rendered;

    m_inBuffer.swap(ch.m_inBuffer);
    m_outBuffer.swap(ch.m_outBuffer);
    //LOG("Inside Chunk() move cstr\n");
}

Chunk& Chunk::operator=(Chunk&& ch){// move assignment
    m_chunkID = ch.m_chunkID;
    m_frameID = ch.m_frameID;
    m_inHash = ch.m_inHash;
    m_outHash = ch.m_outHash;
    m_rendered = ch.m_rendered;

    m_inBuffer.swap(ch.m_inBuffer);
    m_outBuffer.swap(ch.m_outBuffer);
    //LOG("Inside Chunk =operator\n");
}
