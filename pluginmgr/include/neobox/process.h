#ifndef NEO_PROCESS_H
#define NEO_PROCESS_H

#include <vector>
#include <string>
#include <filesystem>
#include <any>

#include <neobox/coroutine.h>


class NeoProcess: public AsyncAwaiterObject<int>
{
  typedef AsyncAwaiterObject<int> Base;
  typedef std::filesystem::path Path;
protected:
  int* GetResult() override { return &m_ExitCode; }
  void DoSuspend(std::coroutine_handle<> handle) override;
public:
  explicit NeoProcess(const std::u8string& command);
  explicit NeoProcess(const Path& app, const std::u8string& args);
  ~NeoProcess();
  void SetAppPath(const Path& app);
  void SetWorkDir(const std::u8string& dir);
  void SetArgs(const std::u8string& args);
  void SetEnvs(std::vector<std::u8string> envs);
  AsyncInt Run();
  void Stop(bool force = false);
  auto GetStdOut() const { return m_StdOut; }
  auto GetStdErr() const { return m_StdErr; }
private:
  std::filesystem::path m_AppPath;
  std::u8string m_Args;
  std::optional<std::vector<std::u8string>> m_Envs;
  std::filesystem::path m_WorkDir;
  std::string m_StdOut;
  std::string m_StdErr;
  bool m_IsRunning = false;
  int m_ExitCode = 0;
  std::any m_Handle;
private:
  bool StartProcess();
  void CleanUp();
  void ReadOutput();
  void ReadError();
  bool ParseCommand(const std::u8string& command);
  static void ParseArgs(const std::u8string& line, std::vector<std::string>& args);
};

#endif // NEO_PROCESS_H