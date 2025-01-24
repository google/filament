This folder was last updated as follows:

    cd third_party
    curl -L -O https://github.com/abseil/abseil-cpp/archive/refs/heads/master.zip
    unzip master.zip
    mv abseil-cpp-master abseil_new
    rsync -r abseil_new/ abseil/ --delete --exclude tnt
    rm -rf master.zip abseil_new
    git add abseil ; git status
