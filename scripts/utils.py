import tempfile, os

this_dir = os.path.dirname(os.path.abspath(__file__))
src_dir = os.path.dirname(this_dir) + "/src"

def red(text):
	return f"\033[31m{text}\033[0m"
	
def green(text):
	return f"\033[32m{text}\033[0m"
	
def blue(text):
	return f"\033[34m{text}\033[0m"
	
ok = green("[ OK ]")
fail = red("[ FAIL ]")
error = red("*** ERROR ***")

def is_accessible(directory):
	try:
		with tempfile.TemporaryFile(dir=directory):
			pass
	except:
		return False
		
	return True

def get_home_dir():
	return os.path.expanduser('~')
