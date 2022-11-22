add_requires("libcurl")

target("translate")
  set_kind("static")
  add_files("src/*.cpp")
  add_packages("libcurl")
  add_deps("yjson")
target_end()

