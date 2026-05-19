#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

// Global flag to control the main loop
volatile sig_atomic_t keep_running = 1;

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    // write() is async-signal-safe, unlike printf()
    char msg[] = "\n[Monitor] Received SIGINT. Shutting down...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    keep_running = 0;
}

// Signal handler for SIGUSR1 (New report notification)
void handle_sigusr1(int sig) {
    char msg[] = "[Monitor] Alert: A new report has been added!\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

int main() {
    //Check if another monitor is already running
    int fd = open(".monitor_pid", O_RDONLY);
    if (fd >= 0) {
        char pid_buff[32] = {0};
        read(fd, pid_buff, sizeof(pid_buff) - 1);
        close(fd);

        char err_msg[128];
        snprintf(err_msg, sizeof(err_msg), "ERROR: Another monitor is already running with PID: %s!\n", pid_buff);
        write(STDOUT_FILENO, err_msg, strlen(err_msg));
        return 1;
    }

    //Setup signal handlers
    struct sigaction sa_int, sa_usr1;

    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    sigaction(SIGUSR1, &sa_usr1, NULL);

    // 3. Create .monitor_pid
    fd = open(".monitor_pid", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Failed to create .monitor_pid");
        return 1;
    }

    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
    write(fd, pid_str, strlen(pid_str));
    close(fd);

    char start_msg[128];
    snprintf(start_msg, sizeof(start_msg), "START: Monitor initialized successfully with PID %d\n", getpid());
    write(STDOUT_FILENO, start_msg, strlen(start_msg));

    // 3. Wait in a loop until SIGINT is received
    while (keep_running) {
        pause(); // Suspends execution until any signal is caught
    }

    // 4. Cleanup on exit
    unlink(".monitor_pid");
    printf("[Monitor] Cleaned up .monitor_pid. Exiting.\n");

    return 0;
}