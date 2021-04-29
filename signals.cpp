#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num)
{
    cout << "smash: got Ctrl-Z" << endl;
    SmallShell &smash = SmallShell::getInstance();
    if (smash.m_currForegroundProcess != -1)
    {
        int result = 0;
        DO_SYS(result, kill(smash.m_currForegroundProcess, SIGSTOP));

        cout << "smash: process " << smash.m_currForegroundProcess << " was stopped" << endl;

        JobsList::JobEntry *job_entry = smash.m_jobs.getJobByPid(smash.m_currForegroundProcess);
        time_t curTime = 0;
        DO_SYS(curTime, time(nullptr));

        if (job_entry == nullptr)
        {
            smash.m_jobs.addJob(smash.m_currForegroundCommand, smash.m_currForegroundProcess, curTime, true);
        }
        else
        {
            job_entry->setStopped(true);
            job_entry->setFg(false);
            job_entry->setTime(curTime);
        }

        smash.m_currForegroundProcess = -1;
        smash.m_currForegroundCommand = "";
    }
    return;
}

void ctrlCHandler(int sig_num)
{
    cout << "smash: got Ctrl-C" << endl;
    SmallShell &smash = SmallShell::getInstance();
    if (smash.m_currForegroundProcess != -1)
    {
        int ret;
        DO_SYS(ret, kill(smash.m_currForegroundProcess, SIGKILL));

        cout << "smash: process " << smash.m_currForegroundProcess << " was killed" << endl;
        smash.m_currForegroundProcess = -1;
        smash.m_currForegroundCommand = "";
    }
}

void alarmHandler(int sig_num)
{
    // TODO: Add your implementation
}
