﻿#include <systemapi.h>
#include <download.h>
#include <pluginmgr.h>
#include <httplib.h>

using namespace std::literals;

fs::path FileNameFilter(std::u8string& path) {
  std::u8string_view pattern(u8":*?\"<>|");
  std::u8string result;
  result.reserve(path.size());
  for (int count=0; auto c : path) {
    if (pattern.find(c) == pattern.npos) {
      result.push_back(c);
    } else if (c == u8':') {
      // 当路径为相对路径时，可能会有Bug
      if (++count == 1) {
        result.push_back(c);
      }
    }
  }
  path = std::move(result);
  fs::path ret = path;
  return ret.make_preferred();
}

std::map<std::filesystem::path, const DownloadJob*> DownloadJob::m_Pool;

DownloadJob::DownloadJob(std::filesystem::path path, std::u8string url, Callback cb)
  : m_HttpJob(new HttpLib(url, true))
  , m_ImageFile(new std::ofstream(path, std::ios::out | std::ios::binary))
  , m_Callback(std::move(cb))
  , m_Path(std::move(path))
{
  m_HttpJob->SetRedirect(1);

  HttpLib::Callback callback = {
    .m_WriteCallback = [this](auto data, auto size) {
      m_ImageFile->write(reinterpret_cast<const char*>(data), size);
    },
    .m_FinishCallback = [this, url](auto msg, auto res) {
      if (msg.empty() && res->status == 200) {
        m_ImageFile->close();
        m_Callback();
      } else {
        mgr->ShowMsgbox(L"出错"s,
          std::format(L"网络异常！\n"
          "文件名：{}\n网址：{}\n错误信息：{}\n状态码：{}",
          m_Path.wstring(), Utf82WideString(url),
          msg, res->status));
      }

      m_Mutex.lock();
      auto iter = m_Pool.find(m_Path);
      if (iter != m_Pool.end()) {
        delete iter->second;
      }
      m_Pool.erase(iter);
      m_Mutex.unlock();
    },
  };

  m_HttpJob->GetAsync(std::move(callback));
}

DownloadJob::~DownloadJob()
{
  delete m_HttpJob;
  if (m_ImageFile->is_open()) {
    m_ImageFile->close();
    if (fs::exists(m_Path))
      fs::remove(m_Path);
  }
  delete m_ImageFile;
}


void DownloadJob::DownloadImage(const ImageInfoEx imageInfo,
  Callback callback)
{
  if (imageInfo->ErrorCode != ImageInfo::NoErr) {
    mgr->ShowMsgbox(L"出错", Utf82WideString(imageInfo->ErrorMsg));
    return ;
  }

  // Check image dir and file.
  const auto& filePath = FileNameFilter(imageInfo->ImagePath);
  const auto& dir = filePath.parent_path();

  if (!fs::exists(dir)) {
    try {
      fs::create_directories(dir);
    } catch (fs::filesystem_error error) {
      mgr->ShowMsgbox(L"出错", std::format(L"创建文件夹失败！\n{}", Ansi2WideString(error.what())));
      return;
    }
  }
  if (fs::exists(filePath)) {
    if (!fs::file_size(filePath))
      fs::remove(filePath);
    else
      return callback();
  }
  if (imageInfo->ImageUrl.empty()) {
    return;
  }

  if (!HttpLib::IsOnline()) {
    mgr->ShowMsgbox(L"出错", L"网络异常");
    return;
  }

  std::lock_guard<std::mutex> locker(m_Mutex);
  auto& job = m_Pool[filePath];
  if (job) {
    mgr->ShowMsgbox(L"提示", L"任务已经存在！");
    return;
  }

  job = new DownloadJob(filePath, imageInfo->ImageUrl, std::move(callback));
}

bool DownloadJob::IsImageFile(const std::u8string& filesName) {
  // BMP, PNG, GIF, JPG
  auto wideString { Utf82WideString(filesName) };
  return std::regex_match(wideString, std::wregex(m_ImgNamePattern, std::wregex::icase));
}

const DownloadJob::String DownloadJob::m_ImgNamePattern {
  L".*\\.(jpg|bmp|gif|jpeg|png)$"
};
