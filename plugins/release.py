import os, shutil, platform

sys_name = platform.system()

def copy_plugins(src: str, des: str):
    plugins = os.listdir(src)
    if not os.path.exists(des):
        os.makedirs(des)
    for plugin in plugins:
        print(f"正在复制：{plugin}", end='  ')
        from_path = os.path.join(src, plugin)
        des_name = plugin.split('.')[0]
        if sys_name == 'Linux':
            des_name = des_name[3:]
        to_dir = os.path.join(des, des_name)
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
