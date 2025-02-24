PUSHD %~dp0
go run .\cmd\run_testlist\main.go --test-list=%~dp0testlists\vk-master.txt %*
POPD
