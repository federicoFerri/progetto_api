import os
import subprocess
import time

if __name__ == '__main__':
    for file in os.listdir('problems'):
        print (file)
        p = subprocess.Popen('./progetto_api', stdout=subprocess.PIPE, stdin=subprocess.PIPE)
        with open(os.path.join('problems',file)) as f:
            s = time.time()
            out, err = p.communicate(f.read().encode())
            print ('time {}'.format(time.time() - s))
        try:
            with open(os.path.join('solutions', file.replace('prob', 'sol'))) as f:
                for i, line in enumerate(f.readlines()):
                    s = line.strip()
                    print(out.decode().split('\n')[i], s, out.decode().split('\n')[i] == s)
        except:
            print ('error')