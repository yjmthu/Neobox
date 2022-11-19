add_requires("libcurl")

target("wallpaper")
  set_kind("static")
  add_files("src/*.cpp")
  add_deps("network", "yjson", "core")
target_end()

