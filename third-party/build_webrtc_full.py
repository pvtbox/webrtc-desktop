import os
import sys
import re
import subprocess
import shutil

webrtc_out = sys.argv[1] #'src/out/Release'
objext = sys.argv[2] #'.o' / '.obj'
libext = sys.argv[3] #'.a' / '.lib'
out_dir = sys.argv[4]
out = sys.argv[5]
ninja_file_path = sys.argv[6]
ninja_file = sys.argv[7]
ninja_target = sys.argv[8]
ar = sys.argv[9]

once = True 
dependencies = []
tmp_targets = []

examples_path = ninja_file_path 
ninja_file = webrtc_out + '/' + examples_path + ninja_file
build_target = examples_path + ninja_target if sys.platform == 'darwin' else './' + ninja_target

if sys.platform == 'darwin':
    ar_flags = '-qc'
else:
    ar_flags = '-Prcs'

for line in open(ninja_file, 'r'):
    if not line.startswith('build ' + build_target): 
        continue
    for f in line.split(' '):
        if f.startswith(examples_path + ninja_target):
            continue
        if f.endswith(objext):
            file_path = webrtc_out + '/' + f
            tmp_targets.append(file_path)
            if len(tmp_targets) < 40:
                continue
        elif f.endswith(libext):
            file_path = webrtc_out + '/' + f
            shutil.copy(file_path, out_dir)
            continue
        else:
            continue
        if os.name == 'nt':
            if once:
                once = False
                buildline = 'lib /nologo /ltcg /out:{} {}'.format(out_dir+out, ' '.join(tmp_targets))
                os.system(buildline)
            else:
                buildline = 'lib /nologo /ltcg {} {}'.format(out_dir+out, ' '.join(tmp_targets))
                os.system(buildline)
        else:
            buildline = '{} {} {} {}'.format(ar, ar_flags, out_dir+out, ' '.join(tmp_targets))
            os.system(buildline)
        tmp_targets = []
    if len(tmp_targets):
        if os.name == 'nt':
            if once:
                once = False
                buildline = 'lib /nologo /ltcg /out:{} {}'.format(out_dir+out, ' '.join(tmp_targets))
                os.system(buildline)
            else:
                buildline = 'lib /nologo /ltcg {} {}'.format(out_dir+out, ' '.join(tmp_targets))
                os.system(buildline)
        else:
            buildline = '{} {} {} {}'.format(ar, ar_flags, out_dir + out, ' '.join(tmp_targets))
            os.system(buildline)
    break
