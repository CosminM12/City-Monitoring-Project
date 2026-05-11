# AI Usage Documentation

**Tool Used:** Google Gemini

## Phase 1

### Task 1: Generating `parse_condition`

**Prompt Given:**
> I am writing a C program that filters binary records of the type: *provided Record_t structure*. Create a function `int parse_conditions(const char *input, char *field, char *value)` that will take a string formatted exactly as : `"field:operator:value"` (ex: "severity:>=:2"), with the supported operators: `==, !=, <, <=, >, >=` and fields: `severity, category, inspector and timestamp`, and split it into three separate strings. Return 1 on success, 0 otherwise.

**What was generated:**
The AI provided a function that uses the strchr() function to find the positions of the delimiters(:). It calculates the string lengths and uses pointer arithmetic to get the strings and strncpy to cpy them to the corresponding parameter.

**What I changed and why:**
There is no case for changes, everything has been correctly implemented for now.

### Task 2: Generating `match_condition`

**Prompt Given:**
> Create the function `int match_condition(Report_t *r, const char *field, const char *op, const char *value)` that will take a record of the structure: *provided Record_t structure*, and the 3 parameters: `field, op, value` and will check the condition parameter:op:value for the corresponding field of the record. The supported operators are: `==, !=, <, <=, >, >=` and fields: `severity, category, inspector and timestamp`. Return 1 on success or 0 otherwise, careful with the comparison between strings and integers.

**What was generated:**
The AI generated a function that contains an if-else tree checking by the field first using the strcmp function. For each separate field it checks the different operators possible and then returns the result of that corresponding inequality/equality. It correctly put only == and != where no inequality is possible (for strings). The problem: there is no case for the timestamp field, although I asked for it.

**What I changed and why:**
I had to add another else-if branch to check for the timestamp inequalities and implement the possible cases using another prompt: "Please also include the possible cases and operators for the 'timestamp' field", after which it gave the final answer correct.

---

## Phase 2

### Task 3: Understanding `fork()` and `exec()` for Directory Deletion

**Prompt Given:**
> I need to delete a directory and its contents using the external `rm -rf` command in C. I must use `fork()` and the `exec*()` family of functions. How do I safely do this and wait for the deletion to finish?

**What was generated:**
The AI provided a code structure utilizing `pid_t pid = fork()`. It explained that inside the `if (pid == 0)` block (the child process), I should use `execlp("rm", "rm", "-rf", district_name, NULL)`. In the `else` block (the parent process), it demonstrated using `wait(NULL)` to pause the main program until the `rm` command completed.

**What I changed and why:**
I implemented this exact architecture inside my `remove_district` function. Before the `fork()`, I also added an `unlink()` call to ensure the active reports symbolic link associated with that district was deleted concurrently.

### Task 4: Signal Handling with `sigaction` and `kill`

**Prompt Given:**
> How do I use `sigaction` (not `signal`) to catch `SIGINT` and `SIGUSR1` in a C program, and how do I send a `SIGUSR1` from a different C program if I know its PID?

**What was generated:**
The AI showed how to populate the `struct sigaction` and use `sigemptyset` to clear the mask before calling `sigaction(SIGINT, &sa, NULL)`. It highlighted a critical security aspect: `printf()` is not async-signal-safe, and I should use `write(STDOUT_FILENO, ...)` inside signal handlers instead. For the sending program, it showed the `kill(pid, SIGUSR1)` function.

**What I changed and why:**
I used the `write` function in `monitor_reports.c` handlers to ensure safe execution. In `city_manager.c`, I added a check to open `.monitor_pid`, read the PID using `atoi`, and used the `kill()` command to send `SIGUSR1` after appending data to `reports.dat`.