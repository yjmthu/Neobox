add_requires("openssl", "libcurl")

target("translate")
  set_kind("static")
  add_files("src/*.cpp")
  add_packages("libcurl", "openssl")
  add_deps("yjson")
target_end()

