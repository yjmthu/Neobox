from ctypes import windll
import zipfile, os, re, sys

'''
基本格式：zipfile.ZipFile(filename[,mode[,compression[,allowZip64]]])
mode：可选 r,w,a 代表不同的打开文件的方式；r 只读；w 重写；a 添加
compression：指出这个 zipfile 用什么压缩方法，默认是 ZIP_STORED，另一种选择是 ZIP_DEFLATED；
allowZip64：bool型变量，当设置为True时可以创建大于 2G 的 zip 文件，默认值 True；
'''
pattern = re.compile(r"^Speed-Box_v\d{1,2}\.\d{1,2}\.\d{1,2}\.zip$")

zip_dir = os.path.join(os.environ["LOCALAPPDATA"], "SpeedBox")
file_list = os.listdir(zip_dir)
zip_path = ''

for i in file_list:
    if re.match(pattern, i):
        zip_path = os.path.join(zip_dir, i)
        break
if not len(zip_path):
    exit(1)
zip_file = zipfile.ZipFile(zip_path)
zip_list = zip_file.namelist() # 得到压缩包里所有文件

exe_folder = os.path.dirname(os.path.abspath(sys.argv[0]))

for f in zip_list:
    zip_file.extract(f, exe_folder) # 循环解压文件到指定目录
 
zip_file.close() # 关闭文件，必须有，释放内存

os.remove(zip_path)

exe_file = os.path.join(exe_folder, "SpeedBox.exe")
print(exe_file)

windll.shell32.ShellExecuteW(0, 'open', exe_file, '', '', 1)
