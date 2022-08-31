Ubpa_GetTargetName(eigen "${PROJECT_SOURCE_DIR}/src/_deps/eigen")

set(libs "${eigen};Qt6::Widgets")


Ubpa_AddTarget(
  TEST
  QT
  MODE EXE
  LIB
    ${libs}
	opencv_world
)