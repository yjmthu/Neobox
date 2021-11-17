from urllib import request
import os, winreg, re, time, random

headers = {
    'User-Agent': 'Mozilla/5.0 (X11; CrOS i686 4319.74.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.57 Safari/537.36'
}

apis = [
    'https://api.ixiaowai.cn/api/api.php',          # 二次元动漫
    'https://api.ixiaowai.cn/mcapi/mcapi.php',      # mc酱动漫
    'https://api.ixiaowai.cn/gqapi/gqapi.php',      # 高清壁纸
    'https://img.paulzzh.com/touhou/random',        # sm.ms 图片源
    'https://api.paugram.com/wallpaper/?source=sm', # 新浪图片源
    'https://v1.yurikoto.com/wallpaper'             # Yurikoto
    'https://www.dmoe.cc/random.php?233335556958'   # 樱花
]

def get_pic_path(file_name) -> str:
    pattern = re.compile(r"%([^%]+?)%", re.S)
    keys = winreg.OpenKey(winreg.HKEY_CURRENT_USER,r'Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders')
    user_pic = winreg.QueryValueEx(keys,'My Pictures')[0]
    winreg.CloseKey(keys)
    env_prev = re.findall(pattern, user_pic)
    if len(env_prev):
        for i in env_prev:
            user_pic = user_pic.replace("%{:s}%".format(i), os.environ[i])
    user_pic += "\\桌面壁纸\\XiaoWai"
    if not os.path.exists(user_pic):
        os.mkdir(user_pic)
    return user_pic + "\\" + file_name


def download(img_url, img_path) -> None:
    # print(img_url)
    if not os.path.exists(img_path):
        img_data = request.urlopen(request.Request(img_url, headers=headers, method='GET'), timeout=60)
        with open(img_path, 'wb') as f:
            f.write(img_data.read())
    print(img_path)

download(random.choice(apis), get_pic_path(time.strftime('%Y-%m-%d-%H-%M-%S.jpg',time.localtime(time.time()))))
