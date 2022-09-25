set_languages("c++20")

if is_plat("windows") then
  add_defines("UNICODE", "_UNICODE")
  add_cxflags("/utf-8")
end

if is_mode("debug") then
  add_defines("DEBUG")
  set_symbols("debug")
  set_optimize("none")
elseif is_mode("release", "profile") then
  if is_mode("release") then
    set_symbols("hidden")
    set_strip("all")
  end
end

add_rules("mode.release", "mode.debug")

add_includedirs(os.dirs("3rdlib/*/include"),
  os.dirs("./*/include"))

includes("3rdlib/YJson", "core", "network",
  "wallpaper", "translate", "widgets", "tests")
