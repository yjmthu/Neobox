import os, shutil

def copy_plugins(src: str, des: str):
    plugins = os.listdir(src)
    if not os.path.exists(des):
        os.makedirs(des)
    for plugin in plugins:
        print(f"正在复制：{plugin}", end='  ')
        from_path = os.path.join(src, plugin)
        to_dir = os.path.join(des, plugin.split('.')[0])
        if not os.path.exists(to_dir):
            os.mkdir(to_dir)
        to_path = os.path.join(to_dir, plugin)
        shutil.copyfile(from_path, to_path)
        print(f"成功")

def get_data_path():
    return os.path.join(os.path.expanduser('~'), ".config/Neobox/plugins")

if __name__ == '__main__':
    from_dir = './install/plugins'
    to_dir = get_data_path()
    copy_plugins(from_dir, to_dir)
