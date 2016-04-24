set(TestApps
  BasicTests
  ColTests
  DocTests
  AqlTests
)

foreach(app ${TestApps})
  add_subdirectory(${app})
endforeach()
