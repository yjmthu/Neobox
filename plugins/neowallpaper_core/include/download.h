#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <wallbase.h>

#include <map>

class DownloadJob {
  typedef std::function<void()> Callback;
public:
#ifdef _WIN32
  typedef std::wstring String;
#else
  typedef std::string String;
#endif
  static void DownloadImage(const ImageInfoEx imageInfo,
    Callback callback);
  static bool IsImageFile(const std::u8string & fileName);

  static std::map<std::filesystem::path, const DownloadJob*> m_Pool;
  inline static std::mutex m_Mutex;
  static const String m_ImgNamePattern;
private:
  DownloadJob(std::filesystem::path path, std::u8string url, Callback callback);
  ~DownloadJob();
  class HttpLib* m_HttpJob;
  std::ofstream* m_ImageFile;
  const Callback m_Callback;
  std::filesystem::path m_Path;
};

#endif // DOWNLOAD_H