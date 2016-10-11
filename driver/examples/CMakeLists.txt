set(TestApps
  FuerteBench
)

foreach(app ${TestApps})
  add_subdirectory(${app})
endforeach()
