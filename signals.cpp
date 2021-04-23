#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;




void ctrlZHandler(int sig_num)
{
  cout << "smash: got Ctrl-Z" << endl;
  SmallShell& smash = SmallShell::getInstance();
  if (smash.m_currForegroundProcess != -1)
  {
    int result = kill(smash.m_currForegroundProcess, SIGSTOP);
    if (result == -1)
    {
      _smashPError("kill");
    }
    else
    {
      cout << "smash: process " << smash.m_currForegroundProcess << " was stopped" << endl;
      smash.m_jobs.addJob(smash.m_currForegroundCommand, smash.m_currForegroundProcess, time(nullptr), true);
      smash.m_currForegroundProcess = -1;
      smash.m_currForegroundCommand = nullptr;
    }
  }
}

void ctrlCHandler(int sig_num)
{
  cout << "smash: got Ctrl-C" << endl;
  SmallShell& smash = SmallShell::getInstance();
  if (smash.m_currForegroundProcess != -1)
  {
    int result = kill(smash.m_currForegroundProcess, SIGKILL);
    if (result == -1)
    {
      _smashPError("kill");
    }
    else
    {
      cout << "smash: process " << smash.m_currForegroundProcess << " was killed" << endl;
      smash.m_currForegroundProcess = -1;
      smash.m_currForegroundCommand = nullptr;
    }
  }
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
