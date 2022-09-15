set_languages("cxxlatest")

if is_plat("windows") then
  add_defines("UNICODE", "_UNICODE")
  add_cxflags("/utf-8")
end

add_rules("mode.release", "mode.debug")

add_includedirs(os.dirs("3rdlib/*/include"),
  os.dirs("./*/include"))

includes("3rdlib/YJson", "core", "network",
  "wallpaper", "translate", "widgets", "tests")

