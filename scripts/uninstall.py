import sys, os
from utils import *

def delete_files(paths):
	for path in paths:
		if os.path.isfile(path):
			try:
				os.remove(path)
				print(f"Removed {path} {ok}")
			except Exception as e:
				print(f"Could not remove: {path} {fail}:\n{e}")
		else:
			print(f"No {os.path.basename(path)} found in {os.path.dirname(path)} {fail}")

if __name__ == "__main__":
	try:
		home_dir = get_home_dir()
		cogp_path = home_dir + "/.local/bin/cogp"
		delete_files([cogp_path])
		
		print(green("Uninstallation completed.\n"))
		input("Press ENTER to exit.")
		sys.exit(0)
		
	except Exception as e:
		print(error)
		print(e)
		input("Press ENTER to exit.")
		sys.exit(1)
