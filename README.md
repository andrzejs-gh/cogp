# cogp

A UNIX utility for changing owner, group, and permissions in a single command.

## Usage

```bash
cogp [-r | --recursive] <owner name> <group name> <permissions> <path1> [<path2> ... ]
```

### Examples

```bash
cogp user group rwxr--r-- <path1> [<path2> ... ] 
cogp user group 744 <path1> [<path2> ... ] 
cogp -r user group 0600 <path1> [<path2> ... ] 
cogp --recursive user group 644 <path1> [<path2> ... ] 
```

> At least one path is required.

Use "-r" or "--recursive" as the first argument to apply recursively.
Symlinks below top-level paths are ignored.
If none of the top-level paths is a directory, the flag has no effect.

Permissions can be expressed either as:

- Symbolic string: "rwxrwxrwx", special bits "s" "S" "t" "T" are allowed
- Numeric (octal) mode: 000–777 
- Numeric (octal) mode: 0000–7777 

Use `/` to leave owner, group, or permissions unchanged.

### More Examples

```bash
cogp user / / <path1> [<path2> ... ]
```
> changes only owner
```bash
cogp user / r--r----- <path1> [<path2> ... ] 
```
> changes only owner and permissions
```bash
cogp / / 700 <path1> [<path2> ... ]
```
> changes only permissions
```bash
cogp -r user / / <path1> [<path2> ... ] 
```
> changes only owner and applies recursively to all subpaths 


