import sys
import os
import list_files
import shlex
import subprocess
import glob
reload (list_files)
#print sys.argv
n=len(sys.argv)
typ = sys.argv[n-1]  # last argument cube3D annulus3D ...
files=list_files.use_glob_to_get_files_of_type (typ)
#isovalue 
if len(sys.argv)==1:
  print 'must atleast enter an isovalue and a type such as cube3D or annulus3D'
  print 'python run_iso_cgrad.py 4.2 annulus3D'
  sys.exit()
#create the central gradient files.  
  

isoval=sys.argv[n-2]

#  ADD TO THIS LIST FOR THE VARIOUS POSITIONS

position_list = ['gradNS', 'gradEC'] 
#print position_list

for single_file in files:
  cmd = sys.argv[1:n-2]
  #calculate cgrad file name
  cgradfname_list=single_file.split('.')
  cgradfname_list.insert(2,'cgrad')
  cgradfname=cgradfname_list[0]+\
  '.'+cgradfname_list[1]+'.'+cgradfname_list[2]+\
  '.'+cgradfname_list[3]
  '''
  print cgradfname
  cgrad_command=[]
  cgrad_command.append('./cgradient')
  cgrad_command.append('data/'+single_file)
  cgrad_command.append(cgradfname)
  p2=subprocess.Popen(cgrad_command, stdout=subprocess.PIPE) # Success!
  '''
  # for each position in position list 
  for pos in position_list:
    updated_cmd = cmd[:]
    updated_cmd.append('-position')
    updated_cmd.append(pos)
    updated_cmd.append('-trimesh')
    updated_cmd.append('-o')
    outfile=pos+"."+single_file+".off"
    updated_cmd.append(outfile)
    
    updated_cmd.append('-gradient')
    updated_cmd.append(cgradfname)
    updated_cmd.append(isoval) # update isovalue
    #add path to data
    file_path = 'data/'+single_file
    updated_cmd.append(file_path)
    updated_cmd.insert(0,'./isodual3D')
    #print 'command ',updated_cmd
    p=subprocess.Popen(updated_cmd, stdout=subprocess.PIPE) # Success!
    

   



