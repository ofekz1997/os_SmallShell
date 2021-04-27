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
    m_jobs.removeFinishedJobs();
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    Command *cmd = nullptr;
    if (cmd_s.find(">") != std::string::npos)
    {
        cmd = new RedirectionCommand(cmd_line);
    }
    else if (firstWord.compare("pwd") == 0)
    {
        cmd = new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0)
    {
        cmd = new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("chprompt") == 0)
    {
        cmd = new ChangePromptCommand(cmd_line);
    }
    else if (firstWord.compare("cd") == 0)
    {
        cmd = new ChangeDirCommand(cmd_line, &m_oldpwd);
    }
    else if (firstWord.compare("jobs") == 0)
    {
        cmd = new JobsCommand(cmd_line, &m_jobs);
    }
    else if (firstWord.compare("kill") == 0)
    {
        cmd = new KillCommand(cmd_line, &m_jobs);
    }
    else if (firstWord.compare("fg") == 0)
    {
        cmd = new ForegroundCommand(cmd_line, &m_jobs);
    }
    else if (firstWord.compare("bg") == 0)
    {
        cmd = new BackgroundCommand(cmd_line, &m_jobs);
    }
    else if (firstWord.compare("quit") == 0)
    {
        cmd = new QuitCommand(cmd_line, &m_jobs);
    }
    else if (firstWord.compare("cat") == 0)
    {
        cmd = new CatCommand(cmd_line);
    }
    else
    {
        cmd = new ExternalCommand(cmd_line, &m_jobs);
    }

    return cmd;
}

void SmallShell::executeCommand(const char *cmd_line)
{
    Command *cmd = CreateCommand(cmd_line);
    cmd->execute();
    delete cmd;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line), m_args()
{
    char **temp = (char **)malloc(sizeof(char *) * COMMAND_MAX_ARGS);
    for (int i = 0; i < COMMAND_MAX_ARGS; i++)
    {
        temp[i] = nullptr;
    }
    char *tempCmd = (char *)malloc(sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
    memcpy(tempCmd, cmd_line, sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
    _removeBackgroundSign(tempCmd);
    _parseCommandLine(tempCmd, temp);
    free(tempCmd);
    for (int i = 0; i < COMMAND_MAX_ARGS && temp[i] != nullptr; i++)
    {
        m_args.push_back(string(temp[i]));
        free(temp[i]);
    }
    free(temp);
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
        smash.SetPrompt(m_args[1] + "> ");
    }
}

void ShowPidCommand::execute()
{
    pid_t pid = getpid();
    if (pid == -1)
    {
        _smashPError("getpid");
    }
    else
    {
        std::cout << "smash pid is " << pid << std::endl;
    }
}

void GetCurrDirCommand::execute()
{
    char *pwd = (char *)malloc(COMMAND_ARGS_MAX_LENGTH + 1);
    pwd = getcwd(pwd, COMMAND_ARGS_MAX_LENGTH + 1);
    if (pwd == nullptr)
    {
        _smashPError("getcwd");
    }
    else
    {
        std::cout << pwd << std::endl;
    }
    free(pwd);
}

void ChangeDirCommand::_changeDirAndUpdateOldPwd(const std::string newdir)
{
    char *pwd = (char *)malloc(COMMAND_ARGS_MAX_LENGTH + 1);
    getcwd(pwd, COMMAND_ARGS_MAX_LENGTH + 1);
    if (pwd == nullptr)
    {
        _smashPError("getcwd");
        return;
    }

    int result = chdir(newdir.c_str());
    if (result == -1)
    {
        _smashPError("chdir");
    }
    else
    {
        *m_plastPwd = pwd;
    }
    free(pwd);
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

    cout << job->getCommand() << " : " << job->getPid() << endl;
    if (kill(job->getPid(), SIGCONT) == -1)
    {
        _smashPError("kill");
        return;
    }

    std::string command = job->getCommand();
    pid_t pid = job->getPid();
    m_jobs->removeJobById(job->getJobId());
    Command::runProcessInForeground(pid, command);
}

void BackgroundCommand::execute()
{
    FUNC_ENTRY()
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

    cout << job->getCommand() << " : " << job->getPid() << endl;
    if (kill(job->getPid(), SIGCONT) == -1)
    {
        _smashPError("kill");
        return;
    }

    job->setStopped(false);
    FUNC_EXIT()

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
        while (wait(NULL) != -1);
    }
    SmallShell::getInstance().cleanup(this);

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
void JobsList::addJob(std::string cmd, pid_t pid, time_t time, bool isStopped)
{
    removeFinishedJobs();
    JobEntry *job = nullptr;
    if (static_cast<int>((m_jobEntries.size() - 1)) == m_maxJobId)
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
        time_t curTime = time(nullptr);
        if (curTime == -1)
        {
            _smashPError("time");
            return;
        }
        str += to_string(int(difftime(curTime, job->getStartTime()))) + " secs";
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
        int result = kill(job->getPid(), SIGKILL);
        if (result == -1)
        {
            _smashPError("kill");
        }
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
        int stat = 0;
        pid_t ret = waitpid(job->getPid(), &stat, WNOHANG);
        if (ret > 0) //finished
        {
            if (WIFEXITED(stat))
            {
                if (i == m_maxJobId)
                {
                    maxFinished = true;
                }
                //waitpid(job->getPid(), nullptr, 0);
                delete m_jobEntries[i];
                m_jobEntries[i] = nullptr;
            }
        }
        else if (ret < 0)
        {
            _smashPError("waitpid");
            return;
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
    if (jobId == m_maxJobId)
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
JobsList::JobEntry *JobsList::getLastJob(int *lastJobId)
{
    if (lastJobId != nullptr)
    {
        *lastJobId = m_maxJobId;
    }

    return getJobById(m_maxJobId);
}
JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId)
{
    for (int i = m_maxJobId; i > 0; --i)
    {
        if ((m_jobEntries[i] != nullptr) && m_jobEntries[i]->isStopped())
        {
            if (jobId != nullptr)
            {
                *jobId = i;
            }
            return m_jobEntries[i];
        }
    }
    return nullptr;
}
void ExternalCommand::execute()
{
    cout << m_cmd_line << endl;
    pid_t pid = fork();

    if (pid == 0)
    {
        setpgrp();

        char *const cmd = (char *)malloc(sizeof(char) * sizeof(m_cmd_line));
        memcpy(cmd, m_cmd_line, sizeof(char) * sizeof(m_cmd_line));
        _removeBackgroundSign(cmd);

        char bash[] = {"bash"};
        char c[] = {"-c"};
        char *const args[] = {bash, c, cmd, NULL};

        execv("/bin/bash", args);
        _smashPError("execv");
        return;
    }
    else if (pid > 0)
    {
        if (_isBackgroundComamnd(m_cmd_line))
        {
            m_jobs->addJob(this->getString(), pid, time(nullptr));
        }
        else
        {
            Command::runProcessInForeground(pid, this->getString());
        }
    }
    else
    {
        _smashPError("fork");
    }
}
void CatCommand::execute()
{
    if (m_args.size() == 1)
    {
        _printErrorToScreen("cat", "not enough arguments");
        return;
    }
    for (size_t i = 1; i < m_args.size(); i++)
    {
        int fd = open(m_args[i].c_str(), O_RDONLY);
        if (fd == -1)
        {
            _smashPError("open");
            continue;
        }
        char ch = 0;
        ssize_t res = 0;
        while (true)
        {
            res = read(fd, &ch, 1);
            if (res == -1)
            {
                _smashPError("read");
                break;
            }
            else if (res == 0)
            {
                break;
            }
            else
            {
                if (write(STDOUT, &ch, 1) == -1)
                {
                    _smashPError("write");
                    break;
                }
            }
        }
        close(fd);
    }
}

void Command::runProcessInForeground(pid_t pid, std::string command)
{
    SmallShell &smash = SmallShell::getInstance();
    smash.m_currForegroundProcess = pid;
    smash.m_currForegroundCommand = command;
    int stat = 0;
    while (true)
    {
        pid_t ret = waitpid(pid, &stat, WNOHANG | WUNTRACED);
        if (ret > 0) //finished
        {
            if (WIFEXITED(stat) || WIFSTOPPED(stat) || WIFSIGNALED(stat)) // end
            {
                break;
            }
        }
        else if (ret < 0)
        {
            _smashPError("waitpid");
            break;
        }
    }

    smash.m_currForegroundProcess = -1;
    smash.m_currForegroundCommand = "";
}

void RedirectionCommand::execute() 
{
    
    int temp_stdout_fd = dup(STDOUT);
    if (temp_stdout_fd == -1)
    {
        _smashPError("dup");
        return;
    }
    if (close(STDOUT) == -1)
    {
        _smashPError("close");
        return;
    }

    int flags = O_RDWR | O_CREAT;
    if (m_isAppend)
    {
        flags = flags | O_APPEND;
    }
    else
    {
        flags = flags | O_TRUNC;
    }

    int fd_out = open(m_outPutFile.c_str(), flags, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH | S_IWOTH);

    if (fd_out == -1)
    {
        _smashPError("open");
        return;
    }

    SmallShell::getInstance().executeCommand(m_cmd.c_str());

    if (close(STDOUT) == -1)
    {
        _smashPError("close");
        return;
    }
    if (dup(temp_stdout_fd) == -1)
    {
        _smashPError("dup");
        return;
    }
    if (close(temp_stdout_fd) == -1)
    {
        _smashPError("close");
        return;
    }
}

void RedirectionCommand::prepare()
{
    std::string s(m_cmd_line);
    std::string delimiter = ">";
    std::string file;

    size_t pos = 0;
    pos = s.find_first_of(delimiter);
    m_cmd = s.substr(0, pos);
    ++pos;

    if (s[pos] == '>')
    {
        ++pos;
        m_isAppend = true;
    }
    else
    {
        m_isAppend = false;
    }

    file = s.substr(pos, s.size());
    m_outPutFile = _trim(file);
}

void PipeCommand::prepare()
{
    std::string s(m_cmd_line);
    std::string delimiter = "|";
    std::string cmd1;
    std::string cmd2;

    size_t pos = 0;
    pos = s.find_first_of(delimiter);
    cmd1 = s.substr(0, pos);
    ++pos;

    if (s[pos] == '&')
    {
        ++pos;
        m_isErr = true;
    }
    else
    {
        m_isErr = false;
    }

    cmd2 = s.substr(pos, s.size());

    SmallShell &smash = SmallShell::getInstance();

    char *tempCmd1 = (char *)malloc(sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
    memcpy(tempCmd1, cmd1.c_str(), sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
    _removeBackgroundSign(tempCmd1);

    char *tempCmd2 = (char *)malloc(sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
    memcpy(tempCmd2, cmd2.c_str(), sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
    _removeBackgroundSign(tempCmd2);

    m_cmd1 = smash.CreateCommand(tempCmd1);
    m_cmd2 = smash.CreateCommand(tempCmd2);
}

void PipeCommand::execute()
{
    int my_pipe[2];
    pipe(my_pipe);

    if (fork() == 0)
    { // cmd1
        close(my_pipe[0]);
        close(STDOUT);
        dup(my_pipe[1]);
        close(my_pipe[1]);
        m_cmd1->execute();
    }
    else
    { // smash
        if (fork() == 0)
        { // cmd2
            close(my_pipe[1]);
            close(STDIN);
            dup(my_pipe[0]);
            close(my_pipe[0]);
            m_cmd2->execute();
        }
    }

    //TODO: get pids and figure out how to wait for them
}