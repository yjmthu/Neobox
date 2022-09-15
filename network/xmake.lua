add_requires("libcurl")

target("network")
  set_kind("static")
  add_files("src/*.cpp")
  add_packages("libcurl")
  if is_os("windows") then
    add_syslinks("Wininet")
  end
target_end()

