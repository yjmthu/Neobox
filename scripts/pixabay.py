from urllib import request, parse
import json, sys, os, winreg, re, random

headers = {
    'User-Agent': 'Mozilla/5.0 (X11; CrOS i686 4319.74.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.57 Safari/537.36'
}

def get_pic_path(file_name) -> str:
    pattern = re.compile(r"%([^%]+?)%", re.S)
    keys = winreg.OpenKey(winreg.HKEY_CURRENT_USER,r'Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders')
    user_pic = winreg.QueryValueEx(keys,'My Pictures')[0]
    winreg.CloseKey(keys)
    env_prev = re.findall(pattern, user_pic)
    if len(env_prev):
        for i in env_prev:
            user_pic = user_pic.replace("%{:s}%".format(i), os.environ[i])
    user_pic += "\\桌面壁纸\\Pixabay"
    if not os.path.exists(user_pic):
        os.mkdir(user_pic)
    return user_pic + "\\" + file_name

def get_json_path() -> str:
    folder = os.environ['LOCALAPPDATA'] + "\\SpeedBox\\ScriptsData"
    if not os.path.exists(folder):
        os.makedirs(folder)
    return folder + "\\pixabay.json"

def download(img_url, img_path) -> None:
    # print(img_url)
    if not os.path.exists(img_path):
        img_data = request.urlopen(request.Request(img_url, headers=headers, method='GET'), timeout=60)
        with open(img_path, 'wb') as f:
            f.write(img_data.read())
    print(img_path)

if len(sys.argv) > 1:
    if sys.argv[1] != '1':
        api = "https://pixabay.com/api/?"

        parameters = {
            'key': "19193873-53255d7da0c4e524f0a2c722f",
            'q': "flower",
            'editors_choice': "true",
            'lang': "en",
            'orientation': "horizontal",
            'category': "animals+backgrounds", #backgrounds、 fashion、 nature、 science、 education、 feelings、 health、 people、 religion、 places、 animals、 industry、 computer、 food、 sports、 transportation、 travel、 buildings、 business、 music
            'min_width': 1000,
            'min_height': 1000,
            'image_type': "photo",
            'per_page': 100,
            'order': "popular",
            'safesearch': "false"
        }

        js = {
            "index": 0,
            "len": 0,
            "data": [{
            'imageHeight': i['imageHeight'],
            'imageWidth': i['imageWidth'],
            'imageURL': i['largeImageURL'],
            'user': i["user"],
            'imageName': i['pageURL'].split("/")[-2] + ".jpg"
            } for i in json.loads(request.urlopen(request.Request(api + parse.urlencode(parameters), method='GET'), timeout=60).read())['hits']]
        }
        js['len'] = len(js['data'])
        with open(get_json_path(), 'w') as f:
            json.dump(js, f)
        download(js["data"][0]["imageURL"], get_pic_path(js["data"][0]['imageName']))
    else:
        if os.path.exists(get_json_path()):
            js = None
            with open(get_json_path(), 'r') as f:
                js = json.load(f)
            i = random.randrange(0, js['len'])
            js['index'] = i = i if i != js['index'] else random.randrange(js['len'])
            with open(get_json_path(), 'w') as f:
                json.dump(js, f)
            download(js["data"][i]["imageURL"], get_pic_path(js["data"][i]['imageName']))
else:
    print(get_json_path())
