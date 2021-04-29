#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
//#include <iomanip>
//#include <stdio.h>
//#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <list>

#define DO_SYS(ret, cmd)                  \
    do                                    \
    {                                     \
        /* safely invoke a system call */ \
        (ret) = (cmd);                    \
        if ((ret) == -1)                  \
        {                                 \
            _smashPError(#cmd);           \
            return;                       \
        }                                 \
    } while (0)

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define DEFAULT_PROMPT "smash> "

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define MAX_JOBS 100
const std::string WHITESPACE = " \n\r\t\f\v";

static void _printErrorToScreen(std::string command, std::string error_msg)
{
    std::cerr << "smash error: " << command << ": " << error_msg << std::endl;
}

static void _smashPError(std::string syscall)
{
    std::string msg = "smash error: " + syscall.substr(0, syscall.find('(')) + " failed";
    perror(msg.c_str());
}

class Command
{
protected:
    const char *m_cmd_line;

public:
    Command(const char *cmd_line) : m_cmd_line(cmd_line) {}
    virtual ~Command() {}
    virtual void execute() = 0;
    std::string getString() { return m_cmd_line; }
    static void runProcessInForeground(pid_t pid, std::string command);
};

class BuiltInCommand : public Command
{
protected:
    std::vector<std::string> m_args;

public:
    BuiltInCommand(const char *cmd_line);
    virtual ~BuiltInCommand() {}
};

class JobsList;
class ExternalCommand : public Command
{
    JobsList *m_jobs;

public:
    ExternalCommand(const char *cmd_line, JobsList *jobs) : Command(cmd_line), m_jobs(jobs)
    {
    }
    virtual ~ExternalCommand() {}
    void execute() override;
};

class PipeCommand : public Command
{
    std::string m_cmd1;
    std::string m_cmd2;
    bool m_isErr;

public:
    PipeCommand(const char *cmd_line) : Command(cmd_line) {}
    virtual ~PipeCommand() {}
    void execute() override;
    void prepare();
};

class RedirectionCommand : public Command
{

    std::string m_cmd;
    std::string m_outPutFile;
    bool m_isAppend;

public:
    explicit RedirectionCommand(const char *cmd_line)
        : Command(cmd_line), m_cmd(""), m_outPutFile(""), m_isAppend(false) { prepare(); }
    virtual ~RedirectionCommand() {}

    void execute() override;
    void prepare();
    //void cleanup() override {}
};

class ChangeDirCommand : public BuiltInCommand
{
private:
    void _changeDirAndUpdateOldPwd(const std::string newdir);
    std::string *m_plastPwd;

public:
    ChangeDirCommand(const char *cmd_line, std::string *plastPwd) : BuiltInCommand(cmd_line), m_plastPwd(plastPwd) {}
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand
{
public:
    GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
    {
    }
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
    ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
    {
    }
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class JobsList
{
public:
    class JobEntry
    {
    private:
        std::string m_cmd;
        pid_t m_pid;
        time_t m_start;
        int m_jobId;
        bool m_isStopped;
        bool m_isFg;

    public:
        JobEntry(std::string cmd, pid_t pid, time_t start, int jobId, bool isStopped = false, bool isFg = false)
            : m_cmd(cmd), m_pid(pid), m_start(start), m_jobId(jobId), m_isStopped(isStopped), m_isFg(isFg) {}
        std::string getCommand() { return m_cmd; }
        pid_t getPid() { return m_pid; }
        time_t getStartTime() { return m_start; }
        void setTime(time_t time) { m_start = time; }
        bool isStopped() { return m_isStopped; }
        void setStopped(bool flag) { m_isStopped = flag; }
        int getJobId() { return m_jobId; }
        bool isFg() { return m_isFg; }
        void setFg(bool flag) { m_isFg = flag; }
    };

    JobsList();
    ~JobsList();
    void addJob(std::string cmd, pid_t pid, time_t time, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry *getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry *getLastJob(int *lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    JobEntry *getJobByPid(int pid);

private:
    std::vector<JobEntry *> m_jobEntries;
    int m_maxJobId;
};

class JobsCommand : public BuiltInCommand
{
private:
    JobsList *m_jobs;

public:
    JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), m_jobs(jobs) {}
    virtual ~JobsCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand
{
private:
    JobsList *m_jobs;

public:
    KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), m_jobs(jobs) {}
    virtual ~KillCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
    JobsList *m_jobs;

public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), m_jobs(jobs) {}
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand
{
    JobsList *m_jobs;

public:
    BackgroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), m_jobs(jobs) {}
    virtual ~BackgroundCommand() {}
    void execute() override;
};

class CatCommand : public BuiltInCommand
{
public:
    CatCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~CatCommand() {}
    void execute() override;
};

class QuitCommand : public BuiltInCommand
{
    JobsList *m_jobs;

public:
    QuitCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), m_jobs(jobs) {}
    virtual ~QuitCommand() {}
    void execute() override;
};

class ChangePromptCommand : public BuiltInCommand
{
public:
    ChangePromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
    {
    }
    virtual ~ChangePromptCommand() {}
    void execute() override;
};

struct AlarmData
{
    std::string command;
    time_t start_time;
    int duration;
    pid_t pid;
    bool operator==(const AlarmData &data)
    {
        return (command == data.command) && (start_time == data.start_time) &&
               (duration == data.duration) && (pid == data.pid);
    }
};

class SmallShell
{
private:
    std::string m_oldpwd;
    std::string m_prompt;
    SmallShell();

public:
    pid_t m_currForegroundProcess;
    std::string m_currForegroundCommand;
    JobsList m_jobs;
    std::list<AlarmData> m_alarm;

    Command *CreateCommand(const char *cmd_line);
    SmallShell(SmallShell const &) = delete;     // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator

    void SetPrompt(const std::string &prompt);
    const std::string &GetPrompt();

    void setAlarm();

    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    void cleanup(Command *cmd)
    {
        delete cmd;
        exit(0);
    }

    ~SmallShell();
    void executeCommand(const char *cmd_line);
};

#endif //SMASH_COMMAND_H_