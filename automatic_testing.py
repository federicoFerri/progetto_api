import os
import subprocess
import time
import argparse
import requests
import tarfile

def download_file_from_google_drive(id, destination):
    URL = "https://docs.google.com/uc?export=download"

    session = requests.Session()

    response = session.get(URL, params = { 'id' : id }, stream = True)
    token = get_confirm_token(response)

    if token:
        params = { 'id' : id, 'confirm' : token }
        response = session.get(URL, params = params, stream = True)

    save_response_content(response, destination)

def get_confirm_token(response):
    for key, value in response.cookies.items():
        if key.startswith('download_warning'):
            return value

    return None

def save_response_content(response, destination):
    CHUNK_SIZE = 32768

    with open(destination, "wb") as f:
        for chunk in response.iter_content(CHUNK_SIZE):
            if chunk: # filter out keep-alive new chunks
                f.write(chunk)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('eseguibile', action='store')
    args = parser.parse_args()
    data_folder = 'test_pubblici_api'
    if not os.path.exists('test_pubblici_api'):
        print('test_pubblici_api not found, downloading...')
        file_id = '1m4Ieihwc2VOxToJFel5l8jopMOsgyygc'
        file_name = 'test_pubblici_api.tar.xz'
        download_file_from_google_drive(file_id, file_name)
        with tarfile.open(file_name) as tar:
            tar.extractall(data_folder)
        os.remove(file_name)
        print('done!')
    print('your solution / correct solution / is your solution correct')
    for file in os.listdir(os.path.join(data_folder, 'problems')):
        print (file[:-9])
        p = subprocess.Popen(args.eseguibile, stdout=subprocess.PIPE, stdin=subprocess.PIPE)
        with open(os.path.join(data_folder, 'problems', file)) as f:
            s = time.time()
            out, err = p.communicate(f.read().encode())
            print ('time {}'.format(time.time() - s))
        try:
            with open(os.path.join(data_folder, 'solutions', file.replace('prob', 'sol'))) as f:
                for i, line in enumerate(f.readlines()):
                    s = line.strip()
                    print(out.decode().split('\n')[i], s, out.decode().split('\n')[i] == s)
        except:
            print ('error')