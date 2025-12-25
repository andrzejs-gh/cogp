#include <iostream>
#include <string>
#include <pwd.h>		
#include <grp.h>        
#include <sys/stat.h> 
#include <dirent.h>
#include <unistd.h>     
#include <cstring>
#include <cerrno>
#include <vector>

const uid_t invalid_owner_uid = (uid_t)-2;
const gid_t invalid_group_gid = (gid_t)-2;
const uid_t dont_change_owner_uid = (uid_t)-1;
const gid_t dont_change_group_gid = (gid_t)-1;
const mode_t invalid_permission_mode  = (mode_t)-2;
const mode_t dont_change_permissions = (mode_t)-1;

uid_t get_uid(const std::string& username) 
{
    struct passwd* pw = getpwnam(username.c_str()); // look up user name
    if (!pw) // if nullptr - not found
		return invalid_owner_uid;
    return pw->pw_uid; // return UID
}

gid_t get_gid(const std::string& groupname) 
{
    struct group* gr = getgrnam(groupname.c_str()); // look up group name
    if (!gr) // if nullptr - not found
		return invalid_group_gid;
    return gr->gr_gid; // return GID
}

mode_t get_permission_mode(const std::string& permissions_arg)
{
	// validate the input:
	int arg_length = permissions_arg.size();
	if (permissions_arg == "/")
		return dont_change_permissions;
	else if (arg_length != 3 && arg_length != 4 && arg_length != 9)
		return invalid_permission_mode;
	
	int owner_bit = 0;
	int group_bit = 0;
	int others_bit = 0; 
	int special_bit = 0;
	
	if (arg_length == 3)
	{
		for (char c : permissions_arg) 
		{
			if (c < '0' || c > '7')
				return invalid_permission_mode;
		}
		owner_bit = permissions_arg[0] - '0';
		group_bit = permissions_arg[1] - '0';
		others_bit = permissions_arg[2] - '0';
		
	}
	else if (arg_length == 4)
	{
		for (char c : permissions_arg) 
		{
			if (c < '0' || c > '7')
				return invalid_permission_mode;
		}
		special_bit = permissions_arg[0] - '0';
		owner_bit = permissions_arg[1] - '0';
		group_bit = permissions_arg[2] - '0';
		others_bit = permissions_arg[3] - '0';
	}
	else // (arg_length == 9)
	{	
		// owner bit
		if (permissions_arg[0] == 'r')
			owner_bit += 4;
		else if (permissions_arg[0] != '-')
			return invalid_permission_mode;
		if (permissions_arg[1] == 'w')
			owner_bit += 2;
		else if (permissions_arg[1] != '-')
			return invalid_permission_mode;
		if (permissions_arg[2] == 'x')
			owner_bit += 1;
		else if (permissions_arg[2] == 's')
		{
			owner_bit += 1;
			special_bit += 4;
		}
		else if (permissions_arg[2] == 'S')
			special_bit += 4;
		else if (permissions_arg[2] != '-')
			return invalid_permission_mode;
			
		// group bit
		if (permissions_arg[3] == 'r')
			group_bit += 4;
		else if (permissions_arg[3] != '-')
			return invalid_permission_mode;
		if (permissions_arg[4] == 'w')
			group_bit += 2;
		else if (permissions_arg[4] != '-')
			return invalid_permission_mode;
		if (permissions_arg[5] == 'x')
			group_bit += 1;
		else if (permissions_arg[5] == 's')
		{
			group_bit += 1;
			special_bit += 2;
		}
		else if (permissions_arg[5] == 'S')
			special_bit += 2;
		else if (permissions_arg[5] != '-')
			return invalid_permission_mode;
			
		// others bit
		if (permissions_arg[6] == 'r')
			others_bit += 4;
		else if (permissions_arg[6] != '-')
			return invalid_permission_mode;
		if (permissions_arg[7] == 'w')
			others_bit += 2;
		else if (permissions_arg[7] != '-')
			return invalid_permission_mode;
		if (permissions_arg[8] == 'x')
			others_bit += 1;
		else if (permissions_arg[8] == 't')
		{
			others_bit += 1;
		    special_bit += 1;
		}
		else if (permissions_arg[8] == 'T')
			special_bit += 1;
		else if (permissions_arg[8] != '-')
			return invalid_permission_mode;
	}
	
	mode_t mode = (special_bit << 9) | (owner_bit << 6) | (group_bit << 3) | others_bit;
	return mode;
}

void collect_paths_recursively(std::vector<std::string>& paths)
{
	struct stat st;
    std::vector<std::string> stack;
    
    for (const auto& path : paths) // filtering out file paths
    {
		if (stat(path.c_str(), &st) == 0)
		{
			if (S_ISDIR(st.st_mode)) 
				stack.emplace_back(path);
		}
	}
    
    std::string current_dir;
    std::string item_name, path;
    
    while (!stack.empty())
    {
		current_dir = stack.back();
		stack.pop_back();
		
		DIR* dir = opendir(current_dir.c_str());
        if (!dir)
        {
            std::cerr << "Failed to open directory:\n"
                      << current_dir << "\n"
                      << strerror(errno) << "\n" << "\x1E";
                      // "\x1E" functions here and below as a separator between errors 
                      // for a GUI app reading the stream
            continue;
        }
        
        struct dirent* item;
        while ((item = readdir(dir)) != nullptr)
        {
			item_name = item->d_name;

            // skipping "." and ".."
            if (item_name == "." || item_name == "..")
                continue;
            
            path = current_dir + "/" + item_name;
            
            if (lstat(path.c_str(), &st) != 0)
            {
                std::cerr << "Failed to access:\n"
                          << path << "\n"
                          << strerror(errno) << "\n" << "\x1E";
                continue;
            }
            
            if (S_ISLNK(st.st_mode)) // if it's a symlink
				continue; // ignore symlinks
			else if (S_ISDIR(st.st_mode)) // if it's a directory
			{
				stack.emplace_back(path);
                paths.emplace_back(path);
			}
			else if (S_ISREG(st.st_mode)) // if it's a file
			{
				paths.emplace_back(path);
			}
			else // if it's anything else
			{
				std::cerr << "Failed to access the path as a directory or a file:\n"
                << path << "\n" << "\x1E";
			}
		}
		closedir(dir);
	}
}

int cerr_when_missing_args_if_(bool not_enough_args)
{
	if (not_enough_args)
	{
		std::cerr << "Missing arguments.\n" 
					 "Use -h or --help to display help.\n";
		return 1;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	if (cerr_when_missing_args_if_(argc < 2))
		return 1;
	
	std::string first_arg = argv[1];
	if (argc == 2)
	{
		if (first_arg == "--help" || first_arg == "-h")
		{
			std::cout << "Usage:\n" 
						 "cogp [-r | --recursive] <owner name> <group name> <permissions> <path1> [<path2> ... ]\n"
						 "\n"
						 "Examples:\n"
						 "	cogp user group rwxr--r-- <path1> [<path2> ... ] \n"
						 "	cogp user group 744 <path1> [<path2> ... ] \n"
						 "	cogp -r user group 0600 <path1> [<path2> ... ] \n"
						 "	cogp --recursive user group 644 <path1> [<path2> ... ] \n"
						 "\n"
						 "At least one path is required.\n"
						 "\n"
						 "Use \"-r\" or \"--recursive\" as the first argument to apply recursively.\n"
						 "Symlinks below top-level paths are ignored.\n"
						 "If none of the top-level paths is a directory, the flag has no effect.\n"
						 "\n"
						 "Permissions can be expressed either as:\n"
						 "	- Symbolic string: \"rwxrwxrwx\", special bits \"s\" \"S\" \"t\" \"T\" are allowed\n"
						 "	- Numeric (octal) mode: 000–777 \n"
						 "	- Numeric (octal) mode: 0000–7777 \n"
						 "\n"
						 "Use \"/\" to leave owner, group, or permissions unchanged.\n"
						 "\n"
						 "Examples:\n"
						 "	1: cogp user / / <path1> [<path2> ... ] \n"
						 "		(changes only owner) \n"
						 "	2: cogp user / r--r----- <path1> [<path2> ... ] \n"
						 "		(changes only owner and permissions) \n"
						 "	3: cogp / / 700 <path1> [<path2> ... ] \n"
						 "		(changes only permissions) \n"
						 "	4: cogp -r user / / <path1> [<path2> ... ] \n"
						 "		(changes only owner and applies recursively to all subpaths) \n"
						 "\n"
						 "Use -h or --help to display this message.\n"
						 "Use -V or --version to display version.\n";
			return 0;
		}
		else if (first_arg == "--version" || first_arg == "-V")
		{
			std::cout << "cogp: version 1.1\n";
			return 0;
		}
	}
	
	bool recursive = false;
	std::string owner, group, permissions;
	std::vector<std::string> paths;
	
	if (first_arg == "-r" || first_arg == "--recursive")
	{
		if (cerr_when_missing_args_if_(argc < 6))
			return 1;
		
		recursive = true;
		owner = argv[2];
		group = argv[3];
		permissions = argv[4];
		paths.assign(argv + 5, argv + argc);
		collect_paths_recursively(paths);
	}
	else
	{
		if (cerr_when_missing_args_if_(argc < 5))
			return 1;
		
		owner = argv[1];
		group = argv[2];
		permissions = argv[3];
		paths.assign(argv + 4, argv + argc);
    }
    
    uid_t owner_uid = dont_change_owner_uid;   // default values - don't change
	gid_t group_gid = dont_change_group_gid;
    bool change_owner = false; 
    bool change_group = false;
    mode_t permission_mode = get_permission_mode(permissions);
    
    if (permission_mode == invalid_permission_mode)
    {
		std::cerr << "Invalid permissions argument.\n"
					 "Permissions can be expressed either as:\n"
						 "	- Symbolic string: \"rwxrwxrwx\", special bits \"s\" \"S\" \"t\" \"T\" are allowed\n"
						 "	- Numeric (octal) mode: 000–777 \n"
						 "	- Numeric (octal) mode: 0000–7777 \n";
		return 1;
	}
    
    if (owner != "/") // if the owner is to be changed
    {
		change_owner = true;
		owner_uid = get_uid(owner);
		if (owner_uid == invalid_owner_uid)
		{
			std::cerr << "Invalid owner name: " << owner << "\n";
			return 1;
		}
	}
	if (group != "/") // if the group is to be changed
	{
		change_group = true;
		group_gid = get_gid(group);
		if (group_gid == invalid_group_gid)
		{
			std::cerr << "Invalid group name: " << group << "\n";
			return 1;
		}
	} 
	
	if (permission_mode != dont_change_permissions)
	{
		if (change_owner || change_group)
		{
			// Iterate in reverse to apply changes to children before parents.
			// This prevents permission changes on a parent directory from 
			// locking access to its contents when running the program without root.
			for (int i = paths.size()-1; i >= 0; i--)
			{
				const char* path = paths[i].c_str();
				if (chown(path, owner_uid, group_gid) != 0) // if chown failed
				{
					std::cerr << "Failed to change owner or group for:\n" 
					          << path << "\n" << strerror(errno) << "\n" << "\x1E"; 
				}
				if (chmod(path, permission_mode) != 0) // if chmod failed
				{
					std::cerr << "Failed to change permissions for:\n" 
					          << path << "\n" << strerror(errno) << "\n" << "\x1E";
				}
			}
		}
		else // if only permissions are to be changed
		{
			for (int i = paths.size()-1; i >= 0; i--)
			{
				const char* path = paths[i].c_str();
				if (chmod(path, permission_mode) != 0)
				{
					std::cerr << "Failed to change permissions for:\n" 
					          << path << "\n" << strerror(errno) << "\n" << "\x1E";
				}
			}
		}
	}
	else // if in dont_change_permissions mode
	{
		if (change_owner || change_group)
		{
			for (int i = paths.size()-1; i >= 0; i--)
			{
				const char* path = paths[i].c_str();
				if (chown(path, owner_uid, group_gid) != 0)
				{
					std::cerr << "Failed to change owner or group for:\n" 
					          << path << "\n" << strerror(errno) << "\n" << "\x1E";
				}
			}
		}
	}
    
	return 0;
}
