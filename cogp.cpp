#include <iostream>
#include <pwd.h>		
#include <grp.h>        
#include <sys/stat.h>  
#include <unistd.h>     
#include <cstring>
#include <cerrno>

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
	int permissions_arg_size = permissions_arg.size();
	if (permissions_arg == "/")
		return dont_change_permissions;
	else if (permissions_arg_size != 3 && permissions_arg_size != 9)
		return invalid_permission_mode;
	
	int owner_bit = 0;
	int group_bit = 0;
	int others_bit = 0; 
	
	if (permissions_arg_size == 3)
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
	else // (permissions_arg_size == 9)
	{
		for (char c : permissions_arg) 
		{
			if (c != 'r' && c != 'w' && c != 'x' && c != '-')
				return invalid_permission_mode;
		}
	
		// owner bit
		if (permissions_arg[0] == 'r')
			owner_bit += 4;
		if (permissions_arg[1] == 'w')
			owner_bit += 2;
		if (permissions_arg[2] == 'x')
			owner_bit += 1;
			
		// group bit
		if (permissions_arg[3] == 'r')
			group_bit += 4;
		if (permissions_arg[4] == 'w')
			group_bit += 2;
		if (permissions_arg[5] == 'x')
			group_bit += 1;
			
		// others bit
		if (permissions_arg[6] == 'r')
			others_bit += 4;
		if (permissions_arg[7] == 'w')
			others_bit += 2;
		if (permissions_arg[8] == 'x')
			others_bit += 1;
	}
	
	mode_t mode = (owner_bit << 6) | (group_bit << 3) | others_bit;
	return mode;
}

int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		std::string arg = argv[1];
		if (arg == "--help" || arg == "-h")
		{
			std::cout << "Usage: cogp <owner name> <group name> <permissions> <path1> <path2> ... \n"
						 "Example:\n"
						 "	cogp user group rwxr--r-- /path1 /path2 /path3 \n"
						 "	cogp user group 744 /path1 /path2 /path3 \n"
						 "\n"
						 "At least one path is required.\n"
						 "\n"
						 "Permissions can be expressed either as:\n"
						 "	- Symbolic string: \"rwxrwxrwx\"\n"
						 "	- Numeric (octal) mode: 000–777 \n"
						 "\n"
						 "Use \"/\" to leave owner, group, or permissions unchanged.\n"
						 "\n"
						 "Examples:\n"
						 "	1: cogp user / / /some/path \n"
						 "		(changes only owner) \n"
						 "	2: cogp user / r--r----- /some/path \n"
						 "		(changes only owner and permissions) \n"
						 "	3: cogp / / 700 /some/path \n"
						 "		(changes only permissions) \n"
						 "\n"
						 "Use -h or --help to display this message.\n"
						 "Use -V or --version to display version.\n";
			return 0;
		}
		else if (arg == "--version" || arg == "-V")
		{
			std::cout << "cogp: version 1.0\n";
			return 0;
		}
	}
	
	if (argc < 5)
	{
		std::cerr << "Missing arguments.\n" 
					 "Use -h or --help to display help.\n";
		return 1;
	}
		
    std::string owner = argv[1];
    std::string group = argv[2];
    std::string permissions = argv[3];
    uid_t owner_uid = dont_change_owner_uid;   // default values - don't change
	gid_t group_gid = dont_change_group_gid;
    bool change_owner = false; 
    bool change_group = false;
    mode_t permission_mode = get_permission_mode(permissions);
    
    if (permission_mode == invalid_permission_mode)
    {
		std::cerr << "Invalid permissions argument.\n"
					 "Permissions can be expressed either as:\n"
						 "	- Symbolic string: \"rwxrwxrwx\"\n"
						 "	- Numeric (octal) mode: 000–777 \n";
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
			for (int i = 4; i < argc; i++)
			{
				char* path = argv[i];
				if (chown(path, owner_uid, group_gid) != 0) // if chown failed
				{
					std::cerr << "Failed to change owner or group for:\n" 
					          << path << "\n" << strerror(errno) << "\n" << "\x1E"; 
					          // "\x1E" functions here as a separator between errors for a GUI app reading the stream
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
			for (int i = 4; i < argc; i++)
			{
				char* path = argv[i];
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
			for (int i = 4; i < argc; i++)
			{
				char* path = argv[i];
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
