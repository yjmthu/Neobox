for _, filepath in ipairs(os.files("src/*.cpp")) do
  -- print(path.basename(filepath))
  local filename = path.basename(filepath)
  local targetname = "test_"..filename:split('_')[1]
  target(targetname) 
    set_kind("binary")
    add_files(filepath)
    add_deps("yjson", "core", "network", "translate")
  target_end()
end
