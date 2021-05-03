#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num)
{
    cout << "smash: got ctrl-Z" << endl;
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
    cout << "smash: got ctrl-C" << endl;
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
    cout << "smash: got an alarm" << endl;
    SmallShell &smash = SmallShell::getInstance();
    time_t curTime;
    bool isKillable = false;
    ;
    DO_SYS(curTime, time(nullptr));

    list<AlarmData> to_remove;

    for (AlarmData data : smash.m_alarm)
    {
        int diff = difftime(data.start_time + data.duration, curTime);
        if (diff <= 0)
        {
            if (data.pid == smash.m_currForegroundProcess)
            {
                smash.m_currForegroundProcess = -1;
                smash.m_currForegroundCommand = "";
                isKillable = true;
            }
            else if (SmallShell::getInstance().m_jobs.getJobByPid(data.pid) != nullptr)
            {
                isKillable = true;
            }

            if (isKillable)
            {
                cout << "smash: " << data.command << " timed out!" << endl;
                int ret;
                DO_SYS(ret, kill(data.pid, SIGKILL));
            }
            to_remove.push_back(data);
        }
    }
    for (AlarmData data : to_remove)
    {
        smash.m_alarm.remove(data);
    }
    SmallShell::getInstance().setAlarm();
}
