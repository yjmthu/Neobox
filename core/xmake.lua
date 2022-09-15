target("core")
  set_kind("static")
  add_files("src/*.cpp")
  if is_os("windows") then
    add_syslinks("Iphlpapi")
  end
target_end()
