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

SmallShell::SmallShell() : m_oldpwd(""), m_prompt(DEFAULT_PROMPT)
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

void ChangeDirCommand::_changeDirAndUpdateOldPwd(const char *newdir)
{
    char *pwd = (char *)malloc(COMMAND_ARGS_MAX_LENGTH + 1);
    getcwd(pwd, sizeof(pwd));

    int result = chdir(newdir);
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
    SmallShell &smash = SmallShell::getInstance();

    if (m_args.size() > 2)
    {
        _printErrorToScreen("cd", "too many arguments");
        return;
    }
    else if (m_args.size() > 1)
    {
        if (m_args[1] == "-")
        {
            if (string(*m_plastPwd).empty())
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
            _changeDirAndUpdateOldPwd(m_args[1].c_str());
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
        _printErrorToScreen("kill", "job-id" + m_args[2] + "does not exist");
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
        int job_id = stoi(m_args[2]);
        if (job_id < 1)
        {
            _printErrorToScreen("fg", INVALID_ARGUMNETS);
            return;
        }

        job = m_jobs->getJobById(job_id);
        if (job == nullptr)
        {
            _printErrorToScreen("fg", "job-id" + m_args[2] + "does not exist");
            return;
        }
    }
    else if (m_args.size() > 2)
    {
        _printErrorToScreen("fg", INVALID_ARGUMNETS);
    }
    else
    {
        job = m_jobs->getLastStoppedJob(nullptr);
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
}