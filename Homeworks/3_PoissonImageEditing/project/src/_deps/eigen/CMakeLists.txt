Ubpa_DownloadZip(
	https://cdn.jsdelivr.net/gh/Ubpa/UData/common/eigen/eigen_3_3_7.zip
	eigen_3_3_7.zip
	SHA256 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
)

Ubpa_AddTarget(
  TEST
  MODE INTERFACE
  ADD_CURRENT_TO NONE
  INC
    "src/_deps/eigen"
)
