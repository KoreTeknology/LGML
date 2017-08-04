import sys
import os
import time
from shUtils import *


gitShaPath = os.path.dirname(os.path.abspath(__file__));
gitShaPath = os.path.join(gitShaPath,"../../Source/GitSha.h");
gitShaPath = os.path.abspath(gitShaPath);

def writeSha():
	sha = getGitSha()
	date =  time.strftime("%d/%m/%y (%H:%M)")

	with open(gitShaPath,'w') as f:
		f.write("// autogenerated file (GitSha.py), do not edit unless knowing what you're doing\n")
		f.write('#pragma once\n')
		f.write('#define GIT_SHA  "'+sha+'"\n')
		f.write('#define GIT_SHA_SHORT  "'+sha[0:7]+'"\n')
		f.write('#define COMMIT_DATE  "'+date+'"\n')

def getFileSha():
	with open(gitShaPath,'r') as f:
		l = f.readline();
		while (l):
			if(l.startswith('#define GIT_SHA ')):
				return l.split(' ')[-1].strip().replace("\"","")
			l = f.readline();
	return "No Sha in file"

def getGitSha():
	return sh("git rev-parse --verify HEAD",printIt=False)[:-1].strip()
	
def checkNewSha():
	fileSha = getFileSha()
	gitSha = getGitSha()
	return fileSha!=gitSha


if __name__=='__main__':
	print(("writen sha : "+getFileSha()))
	print(("git sha : "+getGitSha()))
	print(("checking git sha's :"+str(checkNewSha())))
	if (checkNewSha()):
		writeSha()
	# sh("git add "+gitShaPath)
	# exit(1)

