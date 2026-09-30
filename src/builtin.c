#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "jobs.h"
#include "builtin.h"
#include "job_control.h"

/* =========================================================
   BUILTIN: cd
   ========================================================= */

static int builtin_cd(command_t *cmd)
{
    const char *directory;

    if (cmd->argc == 1)
    {
        directory = getenv("HOME");

        if (directory == NULL)
        {
            fprintf(stderr, "cd: HOME not set\n");
            return -1;
        }
    }
    else if (cmd->argc == 2)
    {
        directory = cmd->argv[1];
    }
    else
    {
        fprintf(stderr, "cd: too many arguments\n");
        return -1;
    }

    if (chdir(directory) != 0)
    {
        perror("cd");
        return -1;
    }

    return 0;
}


/* =========================================================
   BUILTIN: pwd
   ========================================================= */

static int builtin_pwd(command_t *cmd)
{
    char current_directory[4096];

    if (cmd->argc > 1)
    {
        fprintf(stderr, "pwd: too many arguments\n");
        return -1;
    }

    if (getcwd(current_directory,
               sizeof(current_directory)) == NULL)
    {
        perror("pwd");
        return -1;
    }

    printf("%s\n", current_directory);

    return 0;
}


/* =========================================================
   BUILTIN: echo
   ========================================================= */

static int builtin_echo(command_t *cmd)
{
    for (int i = 1; i < cmd->argc; i++)
    {
        printf("%s", cmd->argv[i]);

        if (i < cmd->argc - 1)
        {
            printf(" ");
        }
    }

    printf("\n");

    return 0;
}


/* =========================================================
   BUILTIN: exit
   ========================================================= */

static int builtin_exit(command_t *cmd)
{
    if (cmd->argc > 1)
    {
        fprintf(stderr, "exit: too many arguments\n");
        return -1;
    }

    /*
     * Return 1 so the main shell loop
     * knows that it should terminate.
     */
    return 1;
}


/* =========================================================
   CHECK WHETHER COMMAND IS A BUILTIN
   ========================================================= */

int is_builtin(const command_t *cmd)
{
    if (cmd == NULL || cmd->argc == 0)
    {
        return 0;
    }

    if (strcmp(cmd->argv[0], "cd") == 0)
        return 1;

    if (strcmp(cmd->argv[0], "pwd") == 0)
        return 1;

    if (strcmp(cmd->argv[0], "echo") == 0)
        return 1;

    if (strcmp(cmd->argv[0], "exit") == 0)
        return 1;

    if (strcmp(cmd->argv[0], "jobs") == 0)
        return 1;

    if (strcmp(cmd->argv[0], "fg") == 0)
        return 1;

    if (strcmp(cmd->argv[0], "bg") == 0)
        return 1;

    return 0;
}


/* =========================================================
   EXECUTE BUILTIN
   ========================================================= */

int execute_builtin(command_t *cmd)
{
    if (cmd == NULL || cmd->argc == 0)
    {
        return -1;
    }

    if (strcmp(cmd->argv[0], "cd") == 0)
    {
        return builtin_cd(cmd);
    }

    if (strcmp(cmd->argv[0], "pwd") == 0)
    {
        return builtin_pwd(cmd);
    }

    if (strcmp(cmd->argv[0], "echo") == 0)
    {
        return builtin_echo(cmd);
    }

    if (strcmp(cmd->argv[0], "exit") == 0)
    {
        return builtin_exit(cmd);
    }

    if (strcmp(cmd->argv[0], "jobs") == 0)
    {
        return builtin_jobs(cmd);
    }

    if (strcmp(cmd->argv[0], "fg") == 0)
    {
        return builtin_fg(cmd);
    }

    if (strcmp(cmd->argv[0], "bg") == 0)
    {
        return builtin_bg(cmd);
    }

    return -1;
}


/* =========================================================
   BUILTIN: jobs
   ========================================================= */

int builtin_jobs(command_t *cmd)
{
    (void)cmd;

    jobs_print();

    return 0;
}


/* =========================================================
   CONVERT JOB ARGUMENT
   Supports:
       fg 1
       fg %1
       bg 1
       bg %1
   ========================================================= */

static int get_job_id(command_t *cmd)
{
    const char *argument;

    if (cmd == NULL || cmd->argc < 2)
    {
        return -1;
    }

    argument = cmd->argv[1];

    if (argument[0] == '%')
    {
        argument++;
    }

    if (*argument == '\0')
    {
        return -1;
    }

    return atoi(argument);
}


/* =========================================================
   BUILTIN: fg
   ========================================================= */

int builtin_fg(command_t *cmd)
{
    int job_id;
    job_t *job;
    int status;

    if (cmd->argc < 2)
    {
        fprintf(stderr, "fg: usage: fg <job number>\n");
        return 1;
    }

    job_id = get_job_id(cmd);

    if (job_id <= 0)
    {
        fprintf(stderr, "fg: invalid job number\n");
        return 1;
    }

    job = job_find(job_id);

    if (job == NULL)
    {
        fprintf(stderr,
                "fg: no such job: %d\n",
                job_id);
        return 1;
    }

    /*
     * Give terminal to the job's process group.
     */
    give_terminal_to(job->pgid);

    /*
     * Continue stopped process group.
     */
    if (job->state == JOB_STOPPED)
    {
        if (kill(-job->pgid, SIGCONT) == -1)
        {
            perror("fg: SIGCONT");

            take_terminal_back();

            return 1;
        }

        job_continue(job->pgid);
    }

    /*
     * Wait for the process group.
     */
    while (1)
    {
        pid_t result;

        result = waitpid(
            -job->pgid,
            &status,
            WUNTRACED
        );

        if (result < 0)
        {
            /*
             * If there are no more children,
             * the job is finished.
             */
            break;
        }

        if (WIFSTOPPED(status))
        {
            job_stop(job->pgid);
            break;
        }

        if (WIFEXITED(status) ||
            WIFSIGNALED(status))
        {
            job_done(job->pgid);
            break;
        }
    }

    /*
     * Give terminal back to ShellForge.
     */
    take_terminal_back();

    /*
     * Remove completed job.
     */
    if (job->state == JOB_DONE)
    {
        job_remove(job_id);
    }

    return 0;
}


/* =========================================================
   BUILTIN: bg
   ========================================================= */

int builtin_bg(command_t *cmd)
{
    int job_id;
    job_t *job;

    if (cmd->argc < 2)
    {
        fprintf(stderr,
                "bg: usage: bg <job number>\n");
        return 1;
    }

    job_id = get_job_id(cmd);

    if (job_id <= 0)
    {
        fprintf(stderr,
                "bg: invalid job number\n");
        return 1;
    }

    job = job_find(job_id);

    if (job == NULL)
    {
        fprintf(stderr,
                "bg: job not found: %d\n",
                job_id);
        return 1;
    }

    if (kill(-job->pgid, SIGCONT) == -1)
    {
        perror("bg: kill");
        return 1;
    }

    job_continue(job->pgid);

    printf("[%d] %s &\n",
           job->job_id,
           job->command);

    return 0;
}
