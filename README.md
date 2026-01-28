# cogp

A UNIX utility for changing owner, group, and permissions in a single command.

## Usage

```bash
cogp [-r | --recursive] <owner name> <group name> <permissions> <path1> [<path2> ... ]
cogp [-r | --recursive] <owner name> <group name> <permissions> --list <path_to_list>
cogp [-r | --recursive] <owner name> <group name> <permissions> # read paths from stdin
```

### Examples

```bash
cogp user group rwxr--r-- <path1> [<path2> ... ] 
cogp user group 744 <path1> [<path2> ... ] 
cogp user group 700 --list <path_to_list> 
cogp -r user group 0600 <path1> [<path2> ... ] 
cogp --recursive user group 644 <path1> [<path2> ... ] 
some_program | cogp user group 644 
```

> Paths provided via a path list file or stdin must be null-terminated.

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
cogp user / / [...]
```
> changes only owner
```bash
cogp user / r--r----- [...]
```
> changes only owner and permissions
```bash
cogp / / 700 [...]
```
> changes only permissions
```bash
cogp -r user / / [...] 
```
> changes only owner and applies recursively to all subpaths 

## Installation and uninstallation

Installation and uninstallation are handled by Python scripts in /scripts. 

To install **cogp**, launch INSTALL.desktop.
Installation will build and install `cogp` binary in `~/.local/bin`. 

To uninstall, launch UNINSTALL.desktop which will remove `cogp` binary from `~/.local/bin` (if it exists).

## Build requirements

- **CMake ≥ 3.16**
- **C++17-compatible compiler**

If any dependencies are missing, the installation script notifies the user and provides instructions on how to install them.
