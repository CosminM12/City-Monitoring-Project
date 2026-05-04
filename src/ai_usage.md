# AI Usage Documentation

**Tool Used:** Google Gemini

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
