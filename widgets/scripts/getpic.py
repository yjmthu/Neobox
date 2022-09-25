import requests, os, sys, time, ctypes
import ctypes.wintypes as wintypes

CSIDL_COMMON_PICTURES = 0x0027

def get_picfolder() -> str:
    buffer = ctypes.create_unicode_buffer(wintypes.MAX_PATH)
    ctypes.windll.shell32.SHGetFolderPathW(None, CSIDL_COMMON_PICTURES, None, 0, buffer)
    return buffer.value

if __name__ == '__main__':
    # https://api.ixiaowai.cn/api/api.php
    data = requests.get("https://api.ixiaowai.cn/api/api.php")
    folder = os.path.join(get_picfolder(), "桌面壁纸", "脚本获取")
    if not os.path.exists(folder):
        os.makedirs(folder)
    path = os.path.join(folder, time.strftime("%Y-%m-%d %H%M%S.jpg", time.localtime()))
    with open(path, "wb") as f:
        f.write(data.content)
        print(path)

