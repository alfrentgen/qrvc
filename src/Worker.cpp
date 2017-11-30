#include "Worker.h"
#include "Decode.h"

using namespace std;

Worker::Worker() :
    m_uniqThreadMtx(), m_syncMtx(), m_isStopped(true), m_job(NULL), m_jobState(DONE)
{
    //ctor
}

Worker::~Worker()
{
    //dtor

}

int32_t Worker::Start(){
    //make sure no other thread could call Start() until current thread has finished
    lock_guard<mutex> lck(m_uniqThreadMtx);
    {
        unique_lock<mutex> syncLck(m_syncMtx);
        if(m_isStopped){
            m_isStopped = false;
        }else{
            return FAIL;
        }
    }

    m_jobState = DONE;
    m_thread = thread(&Worker::Work, this);
    m_isStopped = true;
    return OK;
}

void Worker::Work(){
    unique_lock<mutex> syncLck(m_syncMtx);

    while(!m_isStopped && m_job != NULL){
        m_jobState = DOING;
        m_job->Do();
        m_jobState = DONE;
        m_cv.wait(syncLck);
    }
    m_isStopped = true;
}

void Worker::Stop(){
    lock_guard<mutex> syncLck(m_syncMtx);
    m_isStopped = true;
    m_cv.notify_all();
}

void Worker::SetJob(Job* j){
    lock_guard<mutex> syncLck(m_syncMtx);
    m_job = j;
    m_cv.notify_all();
}

void DispatchingAlgorithm(){

}
