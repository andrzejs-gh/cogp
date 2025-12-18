# cogp

A small UNIX utility for changing owner / group / permissions in a single command.

Usage:

cogp <owner name> <group name> <permissions> <path1> <path2> ... 

Example:
        cogp user group rwxr--r-- /path1 /path2 /path3 
        cogp user group 744 /path1 /path2 /path3 

At least one path is required.

Permissions can be expressed either as:
        - Symbolic string: "rwxrwxrwx"
        - Numeric (octal) mode: 000–777 

Use "/" to leave owner, group, or permissions unchanged.

Examples:
        1: cogp user / / /some/path 
                (changes only owner) 
        2: cogp user / r--r----- /some/path 
                (changes only owner and permissions) 
        3: cogp / / 700 /some/path 
                (changes only permissions) 

