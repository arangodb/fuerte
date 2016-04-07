set(TestApps BasicTests ColTests DocTests)

foreach(app ${TestApps})

add_subdirectory(${app})

endforeach()
