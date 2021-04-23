#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define DEFAULT_PROMPT "smash> "
#define MAX_JOBS 100
const std::string WHITESPACE = " \n\r\t\f\v";

class Command
{
    const char *m_cmd_line;
    // TODO: Add your data members
public:
    Command(const char *cmd_line) : m_cmd_line(cmd_line) {}
    virtual ~Command() {}
    virtual void execute() = 0;
    std::string getCommand() { return m_cmd_line; }
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command
{
protected:
    std::vector<std::string> m_args;

public:
    BuiltInCommand(const char *cmd_line);
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command
{
public:
    ExternalCommand(const char *cmd_line) : Command(cmd_line)
    {
    }
    virtual ~ExternalCommand() {}
    void execute() override {}
};

class PipeCommand : public Command
{
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command
{
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand
{
private:
    void _changeDirAndUpdateOldPwd(const std::string newdir);
    std::string *m_plastPwd;

public:
    // TODO: Add your data members public:
    ChangeDirCommand(const char *cmd_line, std::string *plastPwd):
                    BuiltInCommand(cmd_line), m_plastPwd(plastPwd){}
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
        Command *m_cmd;
        pid_t m_pid;
        time_t m_start;
        int m_jobId;
        bool m_isStopped;

    public:
        JobEntry(Command *cmd, pid_t pid, time_t start, int jobId, bool isStopped = false) :
                m_cmd(cmd), m_pid(pid), m_start(start), m_jobId(jobId), m_isStopped(isStopped) {}
        std::string getCommand() { return m_cmd->getCommand(); }
        pid_t getPid() { return m_pid; }
        time_t getStartTime() { return m_start; }
        bool isStopped() { return m_isStopped; }
        void setStopped(bool flag) { m_isStopped = flag; }
        int getJobId() { return m_jobId; }
    };

    // TODO: Add your data members
    JobsList();
    ~JobsList();
    void addJob(Command *cmd, pid_t pid, time_t time, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry *getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry *getLastJob(int *lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
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
    // TODO: Add your data members
    JobsList *m_jobs;

public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), m_jobs(jobs) {}
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand
{
    // TODO: Add your data members
    JobsList *m_jobs;

public:
    BackgroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), m_jobs(jobs) {}
    virtual ~BackgroundCommand() {}
    void execute() override;
};

class CatCommand : public BuiltInCommand
{
public:
    CatCommand(const char *cmd_line);
    virtual ~CatCommand() {}
    void execute() override;
};

class QuitCommand : public BuiltInCommand
{
    JobsList *m_jobs;
public:
    QuitCommand(const char *cmd_line, JobsList *jobs) :
                BuiltInCommand(cmd_line), m_jobs(jobs) {}
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

class SmallShell
{
private:
    // TODO: Add your data members
    std::string m_oldpwd;
    std::string m_prompt;
    JobsList m_jobs;
    SmallShell();

public:
    Command *CreateCommand(const char *cmd_line);
    SmallShell(SmallShell const &) = delete;     // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator

    void SetPrompt(const std::string &prompt);
    const std::string &GetPrompt();

    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
