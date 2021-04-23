#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
using namespace std;

#if 0
#define FUNC_ENTRY() \
    cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
    cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define INVALID_ARGUMNETS "invalid arguments"

void _printErrorToScreen(string command, string error_msg)
{
    cerr << "smash error: " << command << ": " << error_msg << endl;
}

void _smashPError(string syscall)
{
    string msg = "smash error: " + syscall + " failed";
    perror(msg.c_str());
}

string _ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args)
{
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;)
    {
        args[i] = (char *)malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos)
    {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&')
    {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() : m_oldpwd(""), m_prompt(DEFAULT_PROMPT), m_jobs()
{
    // TODO: add your implementation
}
void SmallShell::SetPrompt(const std::string &prompt)
{
    m_prompt = prompt;
}
const std::string &SmallShell::GetPrompt()
{
    return m_prompt;
}

SmallShell::~SmallShell()
{
    // TODO: add your implementation
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line)
{
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("pwd") == 0)
    {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0)
    {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("chprompt") == 0)
    {
        return new ChangePromptCommand(cmd_line);
    }
    else if (firstWord.compare("cd") == 0)
    {
        return new ChangeDirCommand(cmd_line, &m_oldpwd);
    }
    else if (firstWord.compare("jobs") == 0)
    {
        return new JobsCommand(cmd_line, &m_jobs);
    }
    else
    {
        return new ExternalCommand(cmd_line);
    }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line)
{
    char **temp = (char **)malloc(sizeof(char *) * COMMAND_MAX_ARGS);
    for (int i = 0; i < COMMAND_MAX_ARGS; i++)
    {
        temp[i] = nullptr;
    }

    _parseCommandLine(cmd_line, temp);

    for (int i = 0; i < COMMAND_MAX_ARGS && temp[i] != nullptr; i++)
    {
        m_args[i] = string(temp[i]);
    }

    if (m_args[0][m_args[0].length() - 2] == '&')
    {
    }
}

void ChangePromptCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    string str = string(m_args[1]);
    if (str.empty() || str.compare("") == 0 || str.compare("&") == 0)
    {
        smash.SetPrompt(DEFAULT_PROMPT);
    }
    else
    {
        smash.SetPrompt(m_args[1]);
    }
}

void ShowPidCommand::execute()
{
    std::cout << "smash pid is" << getpid();
}

void GetCurrDirCommand::execute()
{
    char *pwd = (char *)malloc(COMMAND_ARGS_MAX_LENGTH + 1);
    getcwd(pwd, sizeof(pwd));
    printf("%s", pwd);
}

void ChangeDirCommand::_changeDirAndUpdateOldPwd(const std::string newdir)
{
    char *pwd = (char *)malloc(COMMAND_ARGS_MAX_LENGTH + 1);
    getcwd(pwd, sizeof(pwd));

    int result = chdir(newdir.c_str());
    if (result == -1)
    {
        _smashPError("chdir");
    }
    else
    {
        *m_plastPwd = pwd;
    }
}

void ChangeDirCommand::execute()
{
    if (m_args.size() > 2)
    {
        _printErrorToScreen("cd", "too many arguments");
        return;
    }
    else if (m_args.size() > 1)
    {
        if (m_args[1] == "-")
        {
            if ((*m_plastPwd).empty())
            {
                _printErrorToScreen("cd", "OLDPWD not set");
                return;
            }
            else
            {
                _changeDirAndUpdateOldPwd(*m_plastPwd);
            }
        }
        else
        {
            _changeDirAndUpdateOldPwd(m_args[1]);
        }
    }
}

void JobsCommand::execute()
{
    m_jobs->printJobsList();
}

void KillCommand::execute()
{
    if (m_args.size() != 3 || m_args[1][0] != '-')
    {
        _printErrorToScreen("kill", INVALID_ARGUMNETS);
        return;
    }

    int job_id = stoi(m_args[2]);
    int sig = -stoi(m_args[1]);

    if ((sig < 1) || (sig > 64) || job_id < 1)
    {
        _printErrorToScreen("kill", INVALID_ARGUMNETS);
        return;
    }

    JobsList::JobEntry *job = m_jobs->getJobById(job_id);
    if (job == nullptr)
    {
        _printErrorToScreen("kill", "job-id " + m_args[2] + " does not exist");
        return;
    }

    if (kill(job->getPid(), sig) == -1)
    {
        _smashPError("kill");
    }
}

void ForegroundCommand::execute()
{
    JobsList::JobEntry *job;

    if (m_args.size() == 2)
    {
        int job_id = stoi(m_args[1]);
        if (job_id < 1)
        {
            _printErrorToScreen("fg", INVALID_ARGUMNETS);
            return;
        }

        job = m_jobs->getJobById(job_id);
        if (job == nullptr)
        {
            _printErrorToScreen("fg", "job-id " + m_args[1] + " does not exist");
            return;
        }
    }
    else if (m_args.size() > 2)
    {
        _printErrorToScreen("fg", INVALID_ARGUMNETS);
    }
    else
    {
        job = m_jobs->getLastJob(nullptr);
        if (job == nullptr)
        {
            _printErrorToScreen("fg", "jobs list is empty");
            return;
        }
    }

    cout << job->getCommand() << " : " << job->getPid();
    if (kill(job->getPid(), SIGCONT) == -1)
    {
        _smashPError("kill");
        return;
    }

    waitpid(job->getPid(), nullptr, 0);
    m_jobs->removeJobById(job->getJobId());
}

void BackgroundCommand::execute()
{
    JobsList::JobEntry *job;

    if (m_args.size() == 2)
    {
        int job_id = stoi(m_args[1]);
        if (job_id < 1)
        {
            _printErrorToScreen("bg", INVALID_ARGUMNETS);
            return;
        }

        job = m_jobs->getJobById(job_id);
        if (job == nullptr)
        {
            _printErrorToScreen("bg", "job-id " + m_args[1] + " does not exist");
            return;
        }
        else if (!job->isStopped())
        {
            _printErrorToScreen("bg", "job-id " + m_args[1] + " is already running in the background");
            return;
        }
    }
    else if (m_args.size() > 2)
    {
        _printErrorToScreen("bg", INVALID_ARGUMNETS);
    }
    else
    {
        job = m_jobs->getLastStoppedJob(nullptr);
        if (job == nullptr)
        {
            _printErrorToScreen("bg", "there is no stopped jobs to resume");
            return;
        }
    }

    cout << job->getCommand() << " : " << job->getPid();
    if (kill(job->getPid(), SIGCONT) == -1)
    {
        _smashPError("kill");
        return;
    }

    job->setStopped(false);

    //TODO: check if we need o add & to the end of the command
}

void QuitCommand::execute()
{
    if (m_args.size() > 1)
    {
        if (m_args[1] == "kill")
        {
            m_jobs->killAllJobs();
        }
    }
    else
    {
        while (wait(NULL) != -1)
            ;
    }

    exit(1);

    //TDO: vcheck how to exit smash
}

JobsList::JobsList() : m_jobEntries(MAX_JOBS), m_maxJobId(0)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        m_jobEntries[i] = nullptr;
    }
}
JobsList::~JobsList()
{
    for (int i = 1; i < m_maxJobId; i++)
    {
        delete m_jobEntries[i];
    }
}
void JobsList::addJob(Command *cmd, pid_t pid, time_t time, bool isStopped)
{
    removeFinishedJobs();
    JobEntry *job = nullptr;
    if (static_cast<int>((m_jobEntries.size() - 1))== m_maxJobId)
    {
        m_jobEntries.resize(m_maxJobId * 2);

        for (int i = m_maxJobId + 1; i < m_maxJobId * 2; i++)
        {
            m_jobEntries[i] = nullptr;
        }
    }
    int id = ++m_maxJobId;
    job = new JobEntry(cmd, pid, time, id, isStopped);
    m_jobEntries[id] = job;
}
void JobsList::printJobsList()
{
    removeFinishedJobs();
    for (int i = 1; i < static_cast<int>(m_jobEntries.size()); i++)
    {
        JobEntry *job = m_jobEntries[i];

        if (job == nullptr)
        {
            continue;
        }
        string str = "[" + to_string(i) + "]" + " " + job->getCommand() + " : " + to_string(job->getPid()) + " ";
        str += to_string(difftime(time(nullptr), job->getStartTime())) + " secs";
        if (job->isStopped())
        {
            str += " (stopped)";
        }
        std::cout << str << std::endl;
    }
}
void JobsList::killAllJobs()
{
    int num = 0;
    for (int i = 0; i < static_cast<int>(m_jobEntries.size()); i++)
    {
        if (m_jobEntries[i] != nullptr)
        {
            num++;
        }
    }
    std::cout << "smash: sending SIGKILL signal to " << num << "jobs:" << std::endl;
    for (JobEntry *job : m_jobEntries)
    {
        if (job == nullptr)
        {
            continue;
        }
        std::cout << to_string(job->getPid()) << ": " << job->getCommand() << std::endl;
        kill(job->getPid(), SIGKILL);
    }
}
void JobsList::removeFinishedJobs()
{
    bool maxFinished = false;
    for (int i = 0; i < static_cast<int>(m_jobEntries.size()); i++)
    {
        JobEntry *job = m_jobEntries[i];
        if (job == nullptr)
        {
            continue;
        }
        pid_t status = waitpid(job->getPid(), nullptr, WNOHANG);
        if (status > 0) //finished
        {
            if (i == m_maxJobId)
            {
                maxFinished = true;
            }
            waitpid(job->getPid(), nullptr, 0);
            m_jobEntries[i] = nullptr;
        }
    }
    if (maxFinished)
    {
        for (int i = m_maxJobId; i > 0; --i)
        {
            if (m_jobEntries[i] != nullptr)
            {
                m_maxJobId = i;
                return;
            }
        }
        m_maxJobId = 0;
    }
}
JobsList::JobEntry *JobsList::getJobById(int jobId)
{
    if ((jobId < 1) || (jobId > m_maxJobId))
    {
        return nullptr;
    }
    else
    {
        return m_jobEntries[jobId];
    }
}
void JobsList::removeJobById(int jobId)
{
    delete getJobById(jobId);
    m_jobEntries[jobId] = nullptr;
}
JobsList::JobEntry *JobsList::getLastJob(int *lastJobId)
{
    *lastJobId = m_maxJobId;
    return getJobById(m_maxJobId);
}
JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId)
{
    for (int i = m_maxJobId; i > 0; --i)
    {
        if ((m_jobEntries[i] != nullptr) && m_jobEntries[i]->isStopped())
        {
            return m_jobEntries[i];
        }
    }
    return nullptr;
}