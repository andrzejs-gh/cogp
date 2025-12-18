# cogp

A small UNIX utility for changing owner, group, and permissions in a single command.

## Usage

```bash
cogp <owner> <group> <permissions> <path1> <path2> ...
```

### Examples

```bash
cogp user group rwxr--r-- /path1 /path2 /path3
cogp user group 744 /path1 /path2 /path3
```

> At least one path is required.

## Permissions

Permissions can be expressed either as:

- Symbolic string: `rwxrwxrwx`
- Numeric (octal) mode: `000-777`

Use `/` to leave owner, group, or permissions unchanged.

## More Examples

```bash
cogp user / / /some/path        # changes only owner
cogp user / r--r----- /some/path # changes only owner and permissions
cogp / / 700 /some/path          # changes only permissions
```

